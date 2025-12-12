//
// Created by liup on 2022/9/29.
//
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_update_1data_1test(JNIEnv *env, jobject thiz, jstring data) {
    // TODO: implement update_data_test()

    return true;
}

