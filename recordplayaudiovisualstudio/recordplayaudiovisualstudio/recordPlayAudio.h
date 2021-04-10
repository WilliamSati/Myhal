#ifndef RECORDPLAYAUDIO_H
#define RECORDPLAYAUDIO_H


#include "errorHandling.h"
#include "listers.h"
#include "endpointInitializers.h"

#include <iostream>
#include <Mmdeviceapi.h>
#include <audioclient.h>

HRESULT recordPlayAudio(IMMDevice* microphone, IMMDevice* speaker);

#endif