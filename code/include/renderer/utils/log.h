//
// Created by liup on 2022/10/19.
//

#pragma once
#ifdef PLATFORM_ANDROID
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "z_renderer", __VA_ARGS__);

#else 
#include <stdio.h>
#define LOGD(...) printf("z_renderer %s\n", __VA_ARGS__); fflush(stdout);
#endif
