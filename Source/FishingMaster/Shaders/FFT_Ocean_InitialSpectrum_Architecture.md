# FFT 海面模拟 - 初始频谱 (H0) 生成管线数据架构说明书
## FFT Ocean Simulation - Initial Spectrum (H0) Generation Architecture

本技术文档旨在详细阐述基于 FFT（快速傅里叶变换）的多级级联海面模拟系统中，初始频谱（$H_0$）生成的整体数据流向、CPU 与 GPU 的职责划分、运行时计算链条以及显存内存布局。

---

## 一、 架构核心设计原则 (Core Design Principles)

本系统采用 **数据驱动（Data-Driven）** 与 **关注点分离（Separation of Concerns）** 的现代图形学管线架构：
* **CPU 职责：** 负责处理与具体像素空间位置无关、所有线程共享的全局物理常数。
* **GPU 职责：** 负责处理高频的空间映射、逐像素的能量分布求解以及并行复数拼装。

通过将统一部署表达式从 GPU 侧提速（Hoisting）到 CPU 侧，在极大地解放 GPU 核心算力的同时，利用物理连续性彻底消除了美术动态调参时级联边界可能发生的能量断层风险。

---

## 二、 CPU 端数据流水线与参数推导 (CPU-Side Data Pipeline)

美术或策划人员在编辑器（材质实例/数据资产）中仅接触和调节符合现实世界物理量纲的参数。在进入渲染图表（RDG）前，CPU 执行 1973 年 Hasselmann 经验公式，计算出核心控制常量并塞入常量缓冲区（Constant Buffer）。

### 1. 美术输入参数规范 (UI / Data Assets)

| 参数名 (Parameter) | 数据类型 | 物理单位 | 架构说明 (Description) |
| :--- | :--- | :--- | :--- |
| `WindSpeed` | `float` | 米/秒 (m/s) | 10米高空处的物理风速，控制海面能量上限。 |
| `WindDirection` | `FVector2f` | 无 (Normalized) | 主风向向量，在 CPU 端进行归一化。 |
| `Fetch` | `float` | 公里 (km) | 风吹过的物理距离。**注意：CPU 计算前需乘以 1000 转换为米**。 |
| `JONSWAP_Gamma` | `float` | 无 | 谱峰提升系数，默认 3.3。值越大波峰越尖锐。 |
| `WindDependency` | `float` | 无 | 高频截断阻尼因子，默认 0.07，消除非物理高频尖刺。 |
| `Resolution` | `uint32` | 像素 (Pixels) | FFT 频域网格分辨率，如 256 或 512。 |
| `PatchLengths` | `FVector4f` | 米 (meters) | 最多支持4级级联的物理尺寸，如 `(1000.0, 250.0, 62.5, 15.6)`。 |
| `Amplitudes` | `FVector4f` | 无 | 各级级联的振幅高度缩放权重，用于美术表现微调。 |

### 2. CPU 端经验公式推导

CPU 利用以下公式推导全局控制常数：

1. **无量纲风区 (Dimensionless Fetch $\tilde{x}$):**
   $$\tilde{x} = \frac{g \cdot (Fetch \times 1000)}{WindSpeed^2}$$
2. **Phillips 能量尺度常数 ($\alpha$):**
   $$\alpha = 0.076 \cdot \tilde{x}^{-0.22}$$
3. **谱峰角频率 ($\omega_p$):**
   $$\omega_p = 22.0 \cdot \tilde{x}^{-0.33} \cdot \frac{g}{WindSpeed}$$

*(注：其中 $g$ 为重力加速度常数 $9.81\text{ m/s}^2$)*

---

## 三、 GPU 端运行时计算管线 (GPU Compute Shader Pipeline)

GPU 调度 Compute Shader（`InitialSpectrumCS`），分配线程组：`Dispatch(Resolution / 16, Resolution / 16, NumCascades)`。其中 Z 轴代表当前的级联层级（`CascadeIndex`）。

逐线程的核心并行计算链条如下：

### Step 1: 频域坐标映射 (k-Space Mapping)
利用全局线程 ID 减去 `Resolution / 2`，将低频（0频率中心）移动到频域纹理的几何正中心，构建二维波矢空间 $\mathbf{k} = (k_x, k_y)$：
$$k_x = (ThreadId.x - \frac{N}{2}) \cdot \frac{2\pi}{L}, \quad k_y = (ThreadId.y - \frac{N}{2}) \cdot \frac{2\pi}{L}$$

### Step 2: 多级级联频域截断滤波 (Bandpass Cutoff Filtering)
依据当前级联的物理尺寸计算其管辖的波数边界 $k_{min}$ 与 $k_{max}$：
$$k_{min} = \frac{2\pi}{L_{current}}, \quad k_{max} = \frac{2\pi}{L_{current} / 4}$$
若当前像素的波数长度 $kLength = |\mathbf{k}|$ 超出区间，滤波器 `filter` 标记为 `0.0` 并立刻阻断后续昂贵的数学运算，防止多级级联在相同频率发生能量重复叠加导致的波浪爆炸。

### Step 3: 2D 波数空间能量求解 (2D Energy Evaluation)
基于深水色散关系反推当前像素的角频率 $\omega = \sqrt{g|\mathbf{k}|}$，代入 JONSWAP 公式计算一维频域能量 $S(\omega)$，接着通过**雅可比行列式（Jacobian）**完成频率域到空间波数域的元转换，并应用方向扩展函数 $D(\theta)$ 分配主风向能量：
$$P(\mathbf{k}) = \frac{S(\omega) \cdot \frac{g}{2\omega} \cdot D(\theta)}{|\mathbf{k}|}$$

### Step 4: 高性能哈希噪声发生与复数拼装 (PCG Random & Amplitude Synthesis)
系统彻底抛弃外部噪声图采样。直接使用当前像素的二维坐标与级联层级作为随机数种子运行 **PCG Hash**。结合 **Box-Muller 变换**现场生成两组独立的、服从标准正态分布的高斯复数噪声（$gauss_1$ 与 $gauss_2$）。
最终结合随机相位与能量开方合成初始正向与逆向共轭复数：
$$h_0(\mathbf{k}) = gauss_1 \cdot \sqrt{P(\mathbf{k}) \cdot 0.5}$$
$$h_0(-\mathbf{k}) = gauss_2 \cdot \sqrt{P(-\mathbf{k}) \cdot 0.5}$$

---

## 四、 显存持久化与内存布局 (VRAM Storage Architecture)

计算完毕后，数据被打包存入统一受虚幻引擎 RDG 管理的 `Texture2DArray` 瞬态资源中，其数据格式必须采用高精度浮点格式 **`PF_A32B32G32R32F` (`float4`)**。

### 1. 通道分配映射表 (Channel Mapping)

初始频谱贴图的单像素内四个通道分配如下：

| 纹理通道 (Channel) | 存储物理量 (Physical Quantity) | 数学定义 (Math Representation) |
| :---: | :--- | :--- |
| **R** | 正向波矢复数实部 (Forward Real) | $\text{Real}(h_0(\mathbf{k}))$ |
| **G** | 正向波矢复数虚部 (Forward Imaginary) | $\text{Imag}(h_0(\mathbf{k}))$ |
| **B** | 逆向波矢复数实部 (Backward Real) | $\text{Real}(h_0(-\mathbf{k}))$ |
| **A** | 逆向波矢复数虚部 (Backward Imaginary) | $\text{Imag}(h_0(-\mathbf{k}))$ |

### 2. 采用 `float4` 拼装的架构优势

本架构选择双向复数合并拼装至 `float4` 的核心动机，是为了服务于下一阶段的 **“时间演化着色器（Time Evolution CS）”**。

在游戏运行时的每一帧，时间演化 Shader 在计算当前频域高度 $h(\mathbf{k}, t)$ 时，物理公式要求线程必须**同时**获取 $h_0(\mathbf{k})$ 和 $h_0(-\mathbf{k})$ 的数值。
* **高缓存命中率（Cache Locality）：** 通过 `float4` 布局，下游的演化 Shader 线程只需执行**单次合并纹理采样（Coalesced Texture Fetch）**，就能从 GPU 的同一根缓存行（Cache Line）里一次性加载全部正逆数据。
* **规避性能瓶颈：** 完美避免了因为只使用 `float2` 存储而导致线程必须跨越巨大显存地址进行对角线镜像采样造成的 **GPU 缓存缺失（Cache Miss）**。用 2 倍的初始网格空间，换取了每一帧高频演化核心变换阶段极高的执行效率。