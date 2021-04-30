#include "endpointInitializers.h"

#ifndef REFTIMES_PER_SEC
#define REFTIMES_PER_SEC 10000000 // number of 100 nanoseconds per second
#endif



//create an IAudioClient interface to allow application to interact with enpoint device.
IAudioClient* initializeIAudioClient(IMMDevice* endpointDevice, WAVEFORMATEX** pwfx) {
    IAudioClient* pAudioClient = NULL;

    HRESULT hr = endpointDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL,
        NULL, (void**)&pAudioClient);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when getting the IAudioClient.\n";
        ErrorDescription(hr);
        return NULL;
    }


    WAVEFORMATEX* ppClosestMatch;
    //get the format to be used to initialize the endpoint
    if (*pwfx == NULL) {
        //get the audio format the endpoint is storing the audio in
        hr = pAudioClient->GetMixFormat(pwfx);
        if (hr != S_OK) {
            std::cout << "\nSomething went wrong when getting the IAudioClient format.\n";
            ErrorDescription(hr);
            return NULL;
        }
    }
    //if required to initialize in a specific format, 
    else {
        hr = pAudioClient->IsFormatSupported(
            AUDCLNT_SHAREMODE_SHARED,
            *pwfx,
            &ppClosestMatch
        );
        if (hr != S_OK) {
            std::cout << "\nFormat not supported.\n";
            ErrorDescription(hr);
            return NULL;
        }
    }

    //get the period of the endpoint buffer
    REFERENCE_TIME phnsDefaultDevicePeriod = 1;
    REFERENCE_TIME phnsMinimumDevicePeriod = 1;
    hr = pAudioClient->GetDevicePeriod(
        &phnsDefaultDevicePeriod,
        &phnsMinimumDevicePeriod
    );
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when getting default microphone period.\n";
        ErrorDescription(hr);
        return NULL;
    }
    std::cout << "\ndefault device period (in 100ns units): " << phnsDefaultDevicePeriod << std::endl;
    std::cout << "minimum device period (in 100ns units): " << phnsMinimumDevicePeriod << std::endl;


    //initialize the audio client with 1 second buffer.
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
        REFTIMES_PER_SEC,
        0,
        *pwfx,
        NULL);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when initializing the AudioClient.\n";
        ErrorDescription(hr);
        return NULL;
    }
    return pAudioClient;
}

IMMDevice* getDefaultEndpointDevice(EDataFlow dataFlow) {
    //create a IMMDeviceEnumerator object
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* endpoint = NULL;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator);
    if (hr != S_OK) {
        std::cout << "Something went wrong when creating the MMDeviceEnumerator.\n";
        ErrorDescription(hr);
        return NULL;
    }

    //get an IMMDeviceCollection object with a set of audio endpoint devices that capture audio (microphones).
    hr = pEnumerator->GetDefaultAudioEndpoint(
        dataFlow, eCommunications,
        &endpoint);
    if (hr != S_OK) {
        std::cout << "Something went wrong when geting Audio Endpoints. Response code: ";
        ErrorDescription(hr);
        return NULL;
    }

    pEnumerator->Release();

    return endpoint;
}


