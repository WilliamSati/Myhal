#ifndef RECORDPLAYAUDIO_H
#define RECORDPLAYAUDIO_H


#include "errorHandling.h"
#include "listers.h"
#include "endpointInitializers.h"
#include "createSocket.h"

#include <iostream>
#include <Mmdeviceapi.h>
#include <audioclient.h>

HRESULT recordSendAudio(IMMDevice* microphone, PCSTR address);

#endif