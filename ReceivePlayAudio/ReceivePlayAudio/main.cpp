#include "errorHandling.h"
#include "listers.h"
#include "receivePlayAudio.h"
#include <iostream>
#include <Mmdeviceapi.h>


int main()
{
    std::cout << "Hello World!\n";

    //make this thread single-threaded apartment
    HRESULT hr = CoInitialize(NULL);
    if (hr != S_OK && hr != S_FALSE) {
        std::cout << "Something went wrong when starting CoInitialize.\n";
        ErrorDescription(hr);
        return 1;
    }
    int status = listAllMicrophonesAndSpeakers();
    if (status == 1) {
        return 1;
    }

    //get speaker endpoint
    IMMDevice* speaker = getDefaultEndpointDevice(eRender);
    if (speaker == NULL) {
        std::cout << "\nSomething went wrong when getting the default speaker.\n";
        return 1;
    }

    status = receivePlayAudio(speaker);
    return status;
}
