package pt.b13h.derelictgles2

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.Log
import pt.b13h.derelictgles2.DerelictJNI
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay
import javax.microedition.khronos.opengles.GL10

internal class GL2JNIView(context: Context?) : GLSurfaceView(context), GLSurfaceView.Renderer {
    override fun onDrawFrame(gl: GL10) {
        DerelictJNI.drawFrame()
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        DerelictJNI.init(width, height)
    }

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
        // Do nothing.
    }

    init {
        setEGLContextClientVersion(2)
        setRenderer(this)
    }
}