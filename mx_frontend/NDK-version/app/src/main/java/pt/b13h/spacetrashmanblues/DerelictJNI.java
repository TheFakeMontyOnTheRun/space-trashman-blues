package pt.b13h.spacetrashmanblues;

import android.content.res.AssetManager;

public class DerelictJNI {
    static {
        System.loadLibrary("native-lib");
    }

    public static native void initAssets(AssetManager assetManager);
    public static native void getPixelsFromNative(byte[] javaSide);
    public static native void sendCommand(char cmd);
    public static native int getSoundToPlay();
    public static native int isOnMainMenu();

}
