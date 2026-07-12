//
// Created by liup on 2022/10/19.
//

#pragma once
#ifdef PLATFORM_ANDROID
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "z_renderer", __VA_ARGS__);
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  "z_renderer", __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "z_renderer", __VA_ARGS__);

#else 
#include <stdio.h>
#define LOGD(fmt, ...) printf("z_renderer " fmt "\n", ##__VA_ARGS__); fflush(stdout);
#define LOGW(fmt, ...) printf("z_renderer " fmt "\n", ##__VA_ARGS__); fflush(stdout);
#define LOGE(fmt, ...) printf("z_renderer " fmt "\n", ##__VA_ARGS__); fflush(stdout);
#endif
