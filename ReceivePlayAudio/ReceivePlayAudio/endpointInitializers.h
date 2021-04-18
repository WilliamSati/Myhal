#ifndef ENDPOINTINITIALIZERS_H
#define ENDPOINTINITIALIZERS_H

#include "errorHandling.h"
#include <iostream>
#include <Mmdeviceapi.h>
#include <audioclient.h>

IAudioClient* initializeIAudioClient(IMMDevice* endpointDevice, WAVEFORMATEX** pwfx);
IMMDevice* getDefaultEndpointDevice(EDataFlow dataFlow);

#endif