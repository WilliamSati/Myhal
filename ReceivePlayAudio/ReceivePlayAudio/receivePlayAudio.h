#ifndef RECEIVEPLAYAUDIO_H
#define RECEIVEPLAYAUDIO_H

#include <winsock2.h>

#include "errorHandling.h"
#include "listers.h"
#include "endpointInitializers.h"
#include "createServerSocket.h"

#include <iostream>
#include <Mmdeviceapi.h>
#include <audioclient.h>

HRESULT receivePlayAudio(IMMDevice* speaker);

#endif