package pt.b13h.derelictgles2

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

internal class GL2JNIView : GLSurfaceView, GLSurfaceView.Renderer {
    override fun onDrawFrame(gl: GL10) {
        DerelictJNI.drawFrame()
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        DerelictJNI.init(width, height)
    }

    override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
    }

    override fun onResume() {
        super.onResume()
    }

    override fun onPause() {
        DerelictJNI.onDestroy()
        super.onPause()
    }

    constructor(context: Context) : super(context) {
        setEGLContextClientVersion(2)
        setRenderer(this)
    }

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        setEGLContextClientVersion(2)
        setRenderer(this)
    }
}