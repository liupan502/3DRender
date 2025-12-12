package com.zeekr.hmi3d;

import android.app.Activity;
import android.content.res.AssetManager;
import android.view.Surface;
import android.view.SurfaceView;

public class ApaAdaptor {
    {
        System.loadLibrary("hmi3d");
    }

    public native boolean update_data_test(String data);

    public native boolean init(AssetManager asset_manager, Surface surface);

    public native void destory();

    public native void on_motion_acted(int type, float x, float y);

    public native void update_parking_info(String parking_info);

    public native void update_geo_info(String geo_info);

    public native void update_self_car_info(String self_car_info);

    public native void frame();

    // public native void update_status(String key, )
}
