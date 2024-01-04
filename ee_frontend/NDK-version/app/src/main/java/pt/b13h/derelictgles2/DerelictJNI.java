package pt.b13h.derelictgles2;

import android.content.res.AssetManager;

public class DerelictJNI {
    static {
        System.loadLibrary("native-lib");
    }

    public static native void init(int width, int height);

    public static native void initAssets(AssetManager assetManager);

    public static native void sendCommand(char cmd);

    public static native int getSoundToPlay();

    public static native int isOnMainMenu();

    public static native void drawFrame();

    public static native void onDestroy();
}
