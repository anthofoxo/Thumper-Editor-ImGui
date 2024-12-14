#include "te_audio.hpp"

namespace aurora {
    void AudioEngine::init() {
        ma_context_init(nullptr, 0, nullptr, &mContext);

        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format = ma_format_f32;
        deviceConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) -> void {
            ma_engine_read_pcm_frames(reinterpret_cast<ma_engine*>(pDevice->pUserData), pOutput, frameCount, nullptr);
            };
        deviceConfig.pUserData = &mEngine;

        ma_device_init(&mContext, &deviceConfig, &mDevice);
        ma_device_start(&mDevice);

        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.pDevice = &mDevice;

        ma_engine_init(&engineConfig, &mEngine);
    }

    void AudioEngine::uninit() {
        ma_engine_uninit(&mEngine);
        ma_device_uninit(&mDevice);
        ma_context_uninit(&mContext);
    }
}