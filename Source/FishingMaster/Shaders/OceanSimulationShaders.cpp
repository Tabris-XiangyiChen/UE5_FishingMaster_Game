#include "OceanSimulationShaders.h"

IMPLEMENT_GLOBAL_SHADER(FOceanSimulationCS, "/FishingMasterShaders/OceanSimulation.usf", "MainCS", SF_Compute);

IMPLEMENT_GLOBAL_SHADER(FOceanInitialSpectrumCS, "/FishingMasterShaders/OceanInitialSpectrum.usf", "InitialSpectrumCS", SF_Compute); 

IMPLEMENT_GLOBAL_SHADER(FOceanTimeSpectrumCS, "/FishingMasterShaders/OceanTimeSpectrum.usf", "TimeSpectrumCS", SF_Compute);

IMPLEMENT_GLOBAL_SHADER(FOceanButterflyTextureCS, "/FishingMasterShaders/OceanButterflyTexture.usf", "ButterflyCS", SF_Compute);

IMPLEMENT_GLOBAL_SHADER(FOceanIFFTCS, "/FishingMasterShaders/OceanIFFT.usf", "IFFTCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FOceanAssembleCS, "/FishingMasterShaders/OceanAssemble.usf", "AssembleCS", SF_Compute);