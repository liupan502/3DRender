package com.zeekr.hmi3d;

public class JniCallbackTest {


    public void call_back() {
        int a = 0;
        a++;
    }

    public void call_back(String content) {
        int a = 0;
        a = content.length();

    }
}


//        if (mDataSource == null) {
//            LogUtils.logD(this, "data source initAndroidDataSource " + System.currentTimeMillis());
//            mDataSource = new AndroidDataSourceManager("AndroidDataSourceManager");
//
//            self3DNotifyListener = new AndroidNotifyListener() {
//                @Override
//                public void notifyDataChanged(String name, int type, String value) {
//                    super.notifyDataChanged(name, type, value);
//                    LogUtils.logD(TAG, "notifyDataChanged() name:" + name + " type:" + type + " value:" + value);
//                    if (name.equals(Constants.KANZI_INIT_FINISH) && value.equals("1")) {
//                        isSelf3DInitFinish = true;
//                        DataCenter.getInstance().getKanziToInitFinish().postValue(isSelf3DInitFinish);
//                    } else if (name.equals(Constants.DS_TO_APA)) {
//                        self3DToApaLiveData(new Self3DNotifyModel(name, value));
//                    }
//                }
//            };
//            mDataSource.addAndroidNotifyListener(self3DNotifyListener);
//        } else {
//            LogUtils.logE(this, "data source null " + System.currentTimeMillis());
//        }



