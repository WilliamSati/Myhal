#include "receivePlayAudio.h"
#include <chrono>


#ifndef REFTIMES_PER_SEC
#define REFTIMES_PER_SEC 10000000 // number of 100 nanoseconds per second
#define DEFAULT_BUFLEN 512

#endif



HRESULT receivePlayAudio(IMMDevice* speaker) {
    WAVEFORMATEX* pwfx = NULL;


    //initialize speaker audio client with microphone's mix format.
    IAudioClient* speakerAudioClient = initializeIAudioClient(speaker, pwfx);
    if (speakerAudioClient == NULL) {
        return E_FAIL;
    }

    //get the speaker render client
    IAudioRenderClient* speakerRenderClient = NULL;
    HRESULT hr = speakerAudioClient->GetService(
        __uuidof(IAudioRenderClient),
        (void**)&speakerRenderClient);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when getting the capture client from the iAudioClient.\n";
        ErrorDescription(hr);
        return hr;
    }

    //create a server socket
    SOCKET listenSocket = createServerSocket();
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "\nCouldn't create the listening socket\n";
        return E_FAIL;
    }

    //wait for client to request a connection
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return E_FAIL;
    }

    SOCKET ClientSocket = INVALID_SOCKET;

    // Accept a client socket
    ClientSocket = accept(listenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return E_FAIL;
    }


    // Start playing.
    hr = speakerAudioClient->Start();
    if (hr != S_OK) {
        std::cout << "\nCould not start playing.\n";
        ErrorDescription(hr);
        return hr;
    }


    UINT32 speakerBufferFrameCount;
    speakerAudioClient->GetBufferSize(&speakerBufferFrameCount);
    // Calculate the actual duration of the allocated buffer.
    REFERENCE_TIME speakerHnsActualDuration = (double)REFTIMES_PER_SEC *
        speakerBufferFrameCount / pwfx->nSamplesPerSec;

    std::cout << "speakerBufferFrameCount: " << speakerBufferFrameCount << " speakerHnsActualDuration: " << speakerHnsActualDuration << std::endl;

    //for spearker:
    UINT32 numFramesPaddingS;
    UINT32 numFramesAvailableS;
    BYTE* speakerBufferStart;
    DWORD speakerBufferFlags;

    int bytesReceived;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> timeElapsed = now - start;
    std::chrono::duration<double, std::milli> maxTime(15000);//5 seconds

    while (timeElapsed < maxTime) {

        // See how much buffer space is available.
        hr = speakerAudioClient->GetCurrentPadding(&numFramesPaddingS);
        if (hr != S_OK) {
            std::cout << "\nsomething went wrong getting padding of speaker buffer.\n";
            ErrorDescription(hr);
            return hr;
        }
        numFramesAvailableS = speakerBufferFrameCount - numFramesPaddingS;

        // Retrieves a pointer to the next available space in the rendering endpoint buffer into which the caller can write a data packet.
        hr = speakerRenderClient->GetBuffer(numFramesAvailableS, &speakerBufferStart);
        if (hr != S_OK) {
            std::cout << "\nsomething went wrong getting the buffer to write the speaker data in.\n";
            ErrorDescription(hr);
            return hr;
        }

        int iResult = recv(ClientSocket, (char*)speakerBufferStart, numFramesAvailableS * pwfx->nBlockAlign, 0);
        if (iResult == SOCKET_ERROR) {
            std::cout << "\nSomething went wrong receiving data from client. error code: " << WSAGetLastError();
            return E_FAIL;
        }
        else {
            bytesReceived = iResult;
        }
        hr = speakerRenderClient->ReleaseBuffer(bytesReceived/(pwfx->nBlockAlign), 0);
        if (hr != S_OK) {
            std::cout << "\nsomething went wrong releasing the speaker buffer.\n";
            ErrorDescription(hr);
            return hr;
        }


        now = std::chrono::high_resolution_clock::now();
        timeElapsed = now - start;
    }

    return hr;
}