#include "listers.h"

int listAllMicrophonesAndSpeakers() {
    std::cout << "\nListing all microphones\n";
    HRESULT hr = listAllEndpointsOfType(eCapture);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when listing all microphones.\n";
        return 1;
    }

    std::cout << "\nListing all speakers\n";
    hr = listAllEndpointsOfType(eRender);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when listing all speakers.\n";
        return 1;
    }

    return 0;//success
}

HRESULT listAllEndpointsOfType(EDataFlow dataFlow) {
    //create a IMMDeviceEnumerator object
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pCollection = NULL;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL,
        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator);
    if (hr != S_OK) {
        std::cout << "Something went wrong when creating the MMDeviceEnumerator.";
        ErrorDescription(hr);
        return hr;
    }

    //get an IMMDeviceCollection object with a set of audio endpoint devices that capture audio (microphones).
    hr = pEnumerator->EnumAudioEndpoints(
        dataFlow, DEVICE_STATE_ACTIVE,
        &pCollection);
    if (hr != S_OK) {
        std::cout << "Something went wrong when geting Audio Endpoints. Response code: ";
        ErrorDescription(hr);
        return hr;
    }

    // for every enpoint device, print it's name
    UINT count = NULL;
    hr = pCollection->GetCount(&count);
    if (hr != S_OK) {
        std::cout << "Something went wrong when geting Audio Endpoints. Response code: ";
        ErrorDescription(hr);
        return hr;
    }
    IMMDevice* device;
    IPropertyStore* deviceProperties;
    PROPVARIANT property;
    PropVariantInit(&property);
    LPWSTR pwszID = NULL;
    for (UINT i = 0; i < count; i++) {
        //get an endpoint from the collection
        hr = pCollection->Item(i, &device);
        if (hr != S_OK) {
            std::cout << "Something went wrong when geting an endpoint from the collection. Response: ";
            ErrorDescription(hr);
            return hr;
        }

        // Get the endpoint ID string.
        hr = device->GetId(&pwszID);
        if (hr != S_OK) {
            std::cout << "Something went wrong when geting the id of an endpoint. Response: ";
            ErrorDescription(hr);
            return hr;
        }

        //open the properties of this endpoint
        hr = device->OpenPropertyStore(STGM_READ, &deviceProperties);
        if (hr != S_OK) {
            std::cout << "Something went wrong when geting the properties of an endpoint. Response: ";
            ErrorDescription(hr);
            return hr;
        }

        //get the name of the endpoint
        hr = deviceProperties->GetValue(PKEY_Device_FriendlyName, &property);
        if (hr != S_OK) {
            std::cout << "Something went wrong when geting the name from the endpoint's properties. Response: ";
            ErrorDescription(hr);
            return hr;
        }

        // Print endpoint friendly name and endpoint ID.
        printf("Endpoint %d: \"%S\" (%S)\n",
            i, property.pwszVal, pwszID);

        CoTaskMemFree(pwszID); 
        pwszID = NULL;

        PropVariantClear(&property); //property variable itself is local so don't have to free that
        deviceProperties->Release();
        device->Release();
    }
    pCollection->Release();
    pEnumerator->Release();
    return S_OK;

}


HRESULT printIMMDeviceName(IMMDevice* device) {

    LPWSTR pwszID = NULL;
    IPropertyStore* deviceProperties;
    PROPVARIANT property;
    PropVariantInit(&property);

    // Get the endpoint ID string.
    HRESULT hr = device->GetId(&pwszID);
    if (hr != S_OK) {
        std::cout << "Something went wrong when geting the id of an endpoint. Response: ";
        ErrorDescription(hr);
        return hr;
    }

    //open the properties of this endpoint
    hr = device->OpenPropertyStore(STGM_READ, &deviceProperties);
    if (hr != S_OK) {
        std::cout << "Something went wrong when geting the properties of an endpoint. Response: ";
        ErrorDescription(hr);
        return hr;
    }

    //get the name of the endpoint
    hr = deviceProperties->GetValue(PKEY_Device_FriendlyName, &property);
    if (hr != S_OK) {
        std::cout << "Something went wrong when geting the name from the endpoint's properties. Response: ";
        ErrorDescription(hr);
        return hr;
    }

    // Print endpoint friendly name and endpoint ID.
    printf("Endpoint: \"%S\" (%S)\n",
        property.pwszVal, pwszID);

    CoTaskMemFree(pwszID); 
    PropVariantClear(&property); //property variable itself is local so don't have to free that
    deviceProperties->Release();
    return S_OK;
}