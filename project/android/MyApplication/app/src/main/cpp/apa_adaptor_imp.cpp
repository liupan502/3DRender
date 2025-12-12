

//
// Created by zhida.ji1 on 2022/9/29.
//

// static

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <utils/file_helper.h>
#include <apa/apa_application.h>

static  hmi_app::ApaApplication* apa_app = nullptr;

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_init(JNIEnv *env, jobject thiz, jobject asset_manager_java,
                                     jobject surface_java) {
    // TODO: implement init()

    AAssetManager* asset_manager = AAssetManager_fromJava(env, asset_manager_java);
    ANativeWindow* native_window = ANativeWindow_fromSurface(env, surface_java);
    if (!asset_manager || !native_window) {
        return false;
    }
    zr::utils::FileHelper::asset_mgr = asset_manager;
    if (apa_app) {
        delete apa_app;
    }
    apa_app = new hmi_app::ApaApplication("apa");
    apa_app->init(asset_manager, native_window);
    // apa_app->update("");
    return true;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_destory(JNIEnv *env, jobject thiz) {
    // TODO: implement destory()
    if (apa_app) {
        delete apa_app;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_on_1motion_1acted(JNIEnv *env, jobject thiz, jint type, jfloat x,
                                                  jfloat y) {
    // TODO: implement on_motion_acted()
    if (!apa_app) {
        return;
    }
    switch (type) {
        // for up
        case 0:
            apa_app->on_clicked(x, y);
            break;
        default:
            break;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_update_1parking_1info(JNIEnv *env, jobject thiz,
                                                      jstring parking_info) {
    // TODO: implement update_parking_info()

    if (!apa_app) {
        return;
    }

    const char* chars = env->GetStringUTFChars(parking_info, nullptr);
    apa_app->update(std::string(chars));
    env->ReleaseStringUTFChars(parking_info, chars);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_update_1geo_1info(JNIEnv *env, jobject thiz, jstring geo_info) {
    // TODO: implement update_geo_info()
    if (!apa_app) {
        return;
    }

    const char* chars = env->GetStringUTFChars(geo_info, nullptr);
    apa_app->update_geo(std::string(chars));
    env->ReleaseStringUTFChars(geo_info, chars);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_update_1self_1car_1info(JNIEnv *env, jobject thiz,
                                                        jstring self_car_info) {
    // TODO: implement update_self_car_info()
    if (!apa_app) {
        return;
    }

    const char* chars = env->GetStringUTFChars(self_car_info, nullptr);
    apa_app->update_self_car(std::string(chars));
    env->ReleaseStringUTFChars(self_car_info, chars);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zeekr_hmi3d_ApaAdaptor_frame(JNIEnv *env, jobject thiz) {
    // TODO: implement frame()
    apa_app->update("");
}