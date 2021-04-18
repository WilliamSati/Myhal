#include "recordPlayAudio.h"
#include <chrono>


#ifndef REFTIMES_PER_SEC
#define REFTIMES_PER_SEC 10000000 // number of 100 nanoseconds per second
#endif



HRESULT recordPlayAudio(IMMDevice* microphone, IMMDevice* speaker) {
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

    //initialize speaker audio client with microphone's mix format.
    IAudioClient* speakerAudioClient = initializeIAudioClient(speaker, &pwfx);
    if (speakerAudioClient == NULL) {
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
    //get the speaker render client
    IAudioRenderClient* speakerRenderClient = NULL;
    hr = speakerAudioClient->GetService(
        __uuidof(IAudioRenderClient),
        (void**)&speakerRenderClient);
    if (hr != S_OK) {
        std::cout << "\nSomething went wrong when getting the capture client from the iAudioClient.\n";
        ErrorDescription(hr);
        return hr;
    }

    // Start recording.
    hr = microphoneAudioClient->Start();
    if (hr != S_OK) {
        std::cout << "\nCould not start recording.\n";
        ErrorDescription(hr);
        return hr;
    }
    // Start playing.
    hr = speakerAudioClient->Start();
    if (hr != S_OK) {
        std::cout << "\nCould not start playing.\n";
        ErrorDescription(hr);
        return hr;
    }

    UINT32 microphoneBufferFrameCount;
    microphoneAudioClient->GetBufferSize(&microphoneBufferFrameCount);
    // Calculate the actual duration of the allocated buffer in 0.1us units
    REFERENCE_TIME microphoneHnsActualDuration = (double)REFTIMES_PER_SEC *
        microphoneBufferFrameCount / pwfx->nSamplesPerSec;

    UINT32 speakerBufferFrameCount;
    speakerAudioClient->GetBufferSize(&speakerBufferFrameCount);
    // Calculate the actual duration of the allocated buffer.
    REFERENCE_TIME speakerHnsActualDuration = (double)REFTIMES_PER_SEC *
        speakerBufferFrameCount / pwfx->nSamplesPerSec;

    std::cout << "\nmicrophoneBufferFrameCount: " << microphoneBufferFrameCount << " microphoneHnsActualDuration: " << microphoneHnsActualDuration << std::endl;
    std::cout << "speakerBufferFrameCount: " << speakerBufferFrameCount << " speakerHnsActualDuration: " << speakerHnsActualDuration << std::endl;



    //for microphone
    bool bDone = FALSE;
    UINT32 packetLength;
    BYTE* microphoneData;
    UINT32 numFramesAvailableM;
    DWORD microphoneBufferFlags;

    //for spearker:
    UINT32 numFramesPaddingS;
    UINT32 numFramesAvailableS;
    BYTE* speakerBufferStart;
    DWORD speakerBufferFlags;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> timeElapsed = now - start;
    std::chrono::duration<double, std::milli> maxTime(15000);//5 seconds

    double avPacketLength;

    while (timeElapsed < maxTime) {

        // Sleep for half the buffer duration.
        //Sleep(microphoneHnsActualDuration / REFTIMES_PER_MILLISEC / 2);

        hr = microphoneCaptureClient->GetNextPacketSize(&packetLength);
        if (hr != S_OK) {
            std::cout << "\nsomething went wrong getting next packet size.\n";
            ErrorDescription(hr);
            return hr;
        }


        while (packetLength !=0 ) {
            
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

            UINT32 numFramesWritten;
            //you should only read the entire data packet or nothing
            if ((numFramesAvailableM <= numFramesAvailableS) && (microphoneData != NULL)) {
                memcpy(speakerBufferStart, microphoneData, numFramesAvailableM*pwfx->nBlockAlign);
                numFramesWritten = numFramesAvailableM;
            }
            else {
                numFramesWritten = 0;
            }

            hr = microphoneCaptureClient->ReleaseBuffer(numFramesWritten);
            if (hr != S_OK) {
                std::cout << "\nsomething went wrong releasing the microphone buffer.\n";
                ErrorDescription(hr);
                return hr;
            }

            if (microphoneData == NULL) {
                speakerBufferFlags = _AUDCLNT_BUFFERFLAGS::AUDCLNT_BUFFERFLAGS_SILENT;
            }
            else {
                speakerBufferFlags = 0;
            }
            hr = speakerRenderClient->ReleaseBuffer(numFramesWritten, speakerBufferFlags);
            if (hr != S_OK) {
                std::cout << "\nsomething went wrong releasing the speaker buffer.\n";
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

    return hr;
}