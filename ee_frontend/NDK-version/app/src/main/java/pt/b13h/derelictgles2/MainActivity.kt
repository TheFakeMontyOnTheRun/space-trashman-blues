package pt.b13h.derelictgles2

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.ImageButton

class MainActivity : AppCompatActivity() {

    private var mView: GL2JNIView? = null
    private var running = false

    override fun onCreate(icicle: Bundle?) {
        super.onCreate(icicle)

        DerelictJNI.initAssets(assets)
        setContentView(R.layout.activity_main)
        mView = findViewById(R.id.imageView)

        findViewById<ImageButton>(R.id.btnUp).setOnClickListener {
            DerelictJNI.sendCommand('w')
        }

        findViewById<ImageButton>(R.id.btnDown).setOnClickListener {
            DerelictJNI.sendCommand('s')
        }

        findViewById<ImageButton>(R.id.btnStrafeLeft).setOnClickListener {
            DerelictJNI.sendCommand('n')
        }

        findViewById<ImageButton>(R.id.btnStrafeRight).setOnClickListener {
            DerelictJNI.sendCommand('m')
        }

        findViewById<ImageButton>(R.id.btnLeft).setOnClickListener {
            DerelictJNI.sendCommand('a')
        }

        findViewById<ImageButton>(R.id.btnRight).setOnClickListener {
            DerelictJNI.sendCommand('d')
        }

        findViewById<Button>(R.id.btnUse).setOnClickListener {
            DerelictJNI.sendCommand('x')
        }


        findViewById<Button>(R.id.btnUseWith).setOnClickListener {
            DerelictJNI.sendCommand('z')
        }


        findViewById<Button>(R.id.btnNextItem).setOnClickListener {
            DerelictJNI.sendCommand('v')
        }


        findViewById<Button>(R.id.btnItemInfo).setOnClickListener {
            DerelictJNI.sendCommand('c')
        }
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