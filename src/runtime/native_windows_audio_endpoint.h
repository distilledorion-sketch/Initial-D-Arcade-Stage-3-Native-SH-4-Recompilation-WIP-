#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>

#include <string>

namespace idas3 {

inline WAVEFORMATEX nativeAudioWaveFormat() {
    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 44100u;
    format.wBitsPerSample = 16u;
    format.nBlockAlign = static_cast<WORD>(
        format.nChannels * format.wBitsPerSample / 8u);
    format.nAvgBytesPerSec =
        format.nSamplesPerSec * format.nBlockAlign;
    return format;
}

struct NativeWaveOutEndpointInfo {
    MMRESULT idResult = MMSYSERR_ERROR;
    UINT deviceId = WAVE_MAPPER;
    MMRESULT capabilitiesResult = MMSYSERR_ERROR;
    std::string name = "UNKNOWN";
    MMRESULT volumeResult = MMSYSERR_ERROR;
    unsigned leftVolume = 0u;
    unsigned rightVolume = 0u;
    MMRESULT preferredResult = MMSYSERR_ERROR;
    UINT preferredDeviceId = WAVE_MAPPER;
    std::string preferredName = "UNKNOWN";
};

inline NativeWaveOutEndpointInfo nativeWaveOutEndpointInfo(HWAVEOUT device) {
    NativeWaveOutEndpointInfo info{};
    info.idResult = waveOutGetID(device, &info.deviceId);
    WAVEOUTCAPSA capabilities{};
    info.capabilitiesResult = info.idResult == MMSYSERR_NOERROR
        ? waveOutGetDevCapsA(info.deviceId, &capabilities,
            sizeof(capabilities))
        : info.idResult;
    if (info.capabilitiesResult == MMSYSERR_NOERROR)
        info.name = capabilities.szPname;
    DWORD endpointVolume = 0u;
    info.volumeResult = waveOutGetVolume(device, &endpointVolume);
    info.leftVolume = endpointVolume & 0xFFFFu;
    info.rightVolume = (endpointVolume >> 16u) & 0xFFFFu;
    DWORD preferredFlags = 0u;
    info.preferredResult = waveOutMessage(
        reinterpret_cast<HWAVEOUT>(static_cast<UINT_PTR>(WAVE_MAPPER)),
        DRVM_MAPPER_PREFERRED_GET,
        reinterpret_cast<DWORD_PTR>(&info.preferredDeviceId),
        reinterpret_cast<DWORD_PTR>(&preferredFlags));
    if (info.preferredResult == MMSYSERR_NOERROR) {
        WAVEOUTCAPSA preferredCapabilities{};
        if (waveOutGetDevCapsA(info.preferredDeviceId,
                &preferredCapabilities, sizeof(preferredCapabilities)) ==
                MMSYSERR_NOERROR)
            info.preferredName = preferredCapabilities.szPname;
    }
    return info;
}

}  // namespace idas3

#endif  // _WIN32
