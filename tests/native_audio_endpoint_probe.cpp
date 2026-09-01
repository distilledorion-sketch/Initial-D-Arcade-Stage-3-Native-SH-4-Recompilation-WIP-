#include "native_windows_audio_endpoint.h"

#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    bool allowNone = false;
    for (int index = 1; index < argc; ++index) {
        if (std::strcmp(argv[index], "--allow-none") == 0)
            allowNone = true;
    }

    WAVEFORMATEX format = idas3::nativeAudioWaveFormat();
    HWAVEOUT device = nullptr;
    const MMRESULT opened = waveOutOpen(
        &device, WAVE_MAPPER, &format, 0u, 0u, CALLBACK_NULL);
    std::printf(
        "[AUDIO-ENDPOINT-PROBE] open_result=%u rate=%lu channels=%u "
        "bits=%u block_align=%u\n",
        static_cast<unsigned>(opened),
        static_cast<unsigned long>(format.nSamplesPerSec),
        static_cast<unsigned>(format.nChannels),
        static_cast<unsigned>(format.wBitsPerSample),
        static_cast<unsigned>(format.nBlockAlign));
    if (opened != MMSYSERR_NOERROR || !device) {
        std::fprintf(stderr,
            "[AUDIO-ENDPOINT-PROBE] result=%s reason=waveout-open\n",
            allowNone ? "SKIP" : "FAIL");
        return allowNone ? 0 : 2;
    }

    const idas3::NativeWaveOutEndpointInfo info =
        idas3::nativeWaveOutEndpointInfo(device);
    std::printf(
        "[AUDIO-ENDPOINT-PROBE] id_result=%u id=%u caps_result=%u "
        "name='%s' volume_result=%u volume=%u,%u preferred_result=%u "
        "preferred_id=%u preferred_name='%s'\n",
        static_cast<unsigned>(info.idResult),
        static_cast<unsigned>(info.deviceId),
        static_cast<unsigned>(info.capabilitiesResult),
        info.name.c_str(),
        static_cast<unsigned>(info.volumeResult),
        info.leftVolume, info.rightVolume,
        static_cast<unsigned>(info.preferredResult),
        static_cast<unsigned>(info.preferredDeviceId),
        info.preferredName.c_str());
    const MMRESULT closed = waveOutClose(device);
    if (info.idResult != MMSYSERR_NOERROR ||
        info.capabilitiesResult != MMSYSERR_NOERROR ||
        closed != MMSYSERR_NOERROR) {
        std::fprintf(stderr,
            "[AUDIO-ENDPOINT-PROBE] result=FAIL reason=endpoint-query-or-close "
            "close_result=%u\n",
            static_cast<unsigned>(closed));
        return 3;
    }
    std::printf(
        "[AUDIO-ENDPOINT-PROBE] result=PASS product-shared-format=1 "
        "sound_written=0\n");
    return 0;
}
