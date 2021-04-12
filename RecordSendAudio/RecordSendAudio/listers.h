#ifndef LISTERS_H
#define LISTERS_H

#include "errorHandling.h"
#include <iostream>
#include <Mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
int listAllMicrophonesAndSpeakers();
HRESULT listAllEndpointsOfType(EDataFlow dataFlow);
HRESULT printIMMDeviceName(IMMDevice* device);

#endif