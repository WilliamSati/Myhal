#include "recordSendAudio.h"
#include <chrono>

#ifndef REFTIMES_PER_SEC
#define REFTIMES_PER_SEC 10000000 // number of 100 nanoseconds per second
#define DEFAULT_PORT 27015
#endif



HRESULT recordSendAudio(IMMDevice* microphone, PCSTR address) {
    WAVEFORMATEX* pwfx = NULL;

    //initialize microphone audioClient
    IAudioClient* microphoneAudioClient = initializeIAudioClient(microphone, &pwfx);
    if (microphoneAudioClient == NULL) {
        return E_FAIL;
    }
    HRESULT hr = microphoneAudioClient->GetMixFormat(&pwfx);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when getting the IAudioClient format.\n";
        ErrorDescription(hr);
        return E_FAIL;
    }

    //get the microphone capture client
    IAudioCaptureClient* microphoneCaptureClient = NULL;
    hr = microphoneAudioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&microphoneCaptureClient);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when getting the capture client from the iAudioClient.\n";
        ErrorDescription(hr);
        return hr;
    }

    SOCKET socket = createClientSocket();
    if (socket == INVALID_SOCKET) {
        std::cout << "\nCouldn't create or connect the socket.\n";
        return E_FAIL;
    }

    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    //setup address structure
    memset((char*)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(DEFAULT_PORT);
    int inetResult = inet_pton(AF_INET, address, &si_other.sin_addr.S_un.S_addr);
    if (inetResult !=  1) {
        std::cout << "\nCouldn't convert ip address.\n";
        return E_FAIL;
    }

    // Start recording.
    hr = microphoneAudioClient->Start();
    if (hr != S_OK) {
        std::cout << "\nCould not start recording.\n";
        ErrorDescription(hr);
        return hr;
    }

    UINT32 microphoneBufferFrameCount;
    microphoneAudioClient->GetBufferSize(&microphoneBufferFrameCount);
    // Calculate the actual duration of the allocated buffer in 0.1us units
    REFERENCE_TIME microphoneHnsActualDuration = (double)REFTIMES_PER_SEC *
        microphoneBufferFrameCount / pwfx->nSamplesPerSec;

    std::cout << "\nmicrophoneBufferFrameCount: " << microphoneBufferFrameCount << " microphoneHnsActualDuration: " << microphoneHnsActualDuration << std::endl;


    //for microphone
    bool bDone = FALSE;
    UINT32 packetLength;
    BYTE* microphoneData;
    UINT32 numFramesAvailableM;
    DWORD microphoneBufferFlags;


    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> timeElapsed = now - start;
    std::chrono::duration<double, std::milli> maxTime(15000);//5 seconds

    double avPacketLength;
    UINT32 numFramesWritten = NULL;
    int iResult;

    while (timeElapsed < maxTime) {

        hr = microphoneCaptureClient->GetNextPacketSize(&packetLength);
        if (hr != S_OK) {
            std::cout << "\nsomething went wrong getting next packet size.\n";
            ErrorDescription(hr);
            return hr;
        }


        while (packetLength != 0 ) {
            
            // Retrieves a pointer to the next available packet of data in the capture endpoint buffer.
            hr = microphoneCaptureClient->GetBuffer(
                &microphoneData,
                &numFramesAvailableM,
                &microphoneBufferFlags, NULL, NULL);
            if (hr != S_OK) {
                std::cout << "\nsomething went wrong getting pointer to enpoint buffer.\n";
                ErrorDescription(hr);
                return hr;
            }

            if (microphoneBufferFlags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                microphoneData = NULL;  // Tell CopyData to write silence.
            }

            // Send an initial buffer
            iResult = sendto(socket, (const char*) microphoneData, (int)strlen((const char*)microphoneData), 0, (struct sockaddr*)&si_other, slen);
            if (iResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(socket);
                WSACleanup();
                return E_FAIL;
            }

            numFramesWritten = iResult / pwfx->nBlockAlign;

            hr = microphoneCaptureClient->ReleaseBuffer(numFramesWritten);
            if (hr != S_OK) {
                std::cout << "\nsomething went wrong releasing the microphone buffer.\n";
                ErrorDescription(hr);
                return hr;
            }

            hr = microphoneCaptureClient->GetNextPacketSize(&packetLength);
            if (hr != S_OK) {
                std::cout << "\nsomething went wrong getting next packet size.\n";
                ErrorDescription(hr);
                return hr;
            }

        }

        now = std::chrono::high_resolution_clock::now();
        timeElapsed = now - start;
    }

    // shutdown the connection for sending since no more data will be sent
    // the client can still use the ConnectSocket for receiving data
    iResult = shutdown(socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return E_FAIL;
    }

    // cleanup
    closesocket(socket);
    WSACleanup();

    return hr;
}