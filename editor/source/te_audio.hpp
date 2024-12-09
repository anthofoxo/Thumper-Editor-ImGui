#pragma once

#include <miniaudio.h>

struct AudioEngine final {
    void init();
    void uninit();

    ma_context mContext;
    ma_device mDevice;
    ma_engine mEngine;
};