#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_pt_b13h_derelictgles2_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}