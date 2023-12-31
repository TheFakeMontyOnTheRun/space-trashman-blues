package pt.b13h.derelictgles2

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle

class MainActivity : AppCompatActivity() {

    private var mView: GL2JNIView? = null
    private var running = false

    override fun onCreate(icicle: Bundle?) {
        super.onCreate(icicle)

        DerelictJNI.initAssets(assets)
        mView = GL2JNIView(application)
        setContentView(mView)
    }

    override fun onPause() {
        super.onPause()
        running = false
        mView!!.onPause()
    }

    override fun onResume() {
        super.onResume()
        mView!!.onResume()
        Thread {
            running = true
            while (running) {
                try {
                    Thread.sleep(20)
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
            }
        }.start()
    }

    override fun onDestroy() {
        DerelictJNI.onDestroy()
        super.onDestroy()
    }
}