package pt.b13h.derelictgles2

import android.app.Presentation
import android.content.Context
import android.content.res.Configuration
import android.media.MediaRouter
import android.os.Build
import android.os.Bundle
import android.view.Display
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.View
import android.view.ViewConfiguration
import android.view.ViewManager
import android.widget.Button
import android.widget.ImageButton
import android.widget.LinearLayout
import androidx.appcompat.app.AppCompatActivity
import pt.b13h.spacetrashmanblues.DerelictApplication

class GameActivity : AppCompatActivity() {

    private var presentation: Presentation? = null
    private var mView: GL2JNIView? = null
    private var running = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT >= 29) {
            enterStickyImmersiveMode()
        } else {
            window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN
        }

        if (savedInstanceState == null) {
            DerelictJNI.initAssets(resources.assets)
        }

        setContentView(R.layout.activity_main)

        mView = findViewById(R.id.imageView)
        mView!!.setOnTouchListener(
            object : OnSwipeTouchListener(this@GameActivity) {
                override fun onTouch(v: View, event: MotionEvent): Boolean {
                    synchronized(mView!!) {
                        val downTime = event.eventTime - event.downTime
                        val pressTimeout = ViewConfiguration.getLongPressTimeout().toLong()
                        if (event.action == MotionEvent.ACTION_UP && downTime < pressTimeout) {
                            DerelictJNI.sendCommand('z')
                        }
                    }
                    return super.onTouch(v, event)
                }

                override fun onSwipeLeft() {
                    super.onSwipeLeft()
                    DerelictJNI.sendCommand('d')
                }

                override fun onSwipeRight() {
                    super.onSwipeRight()
                    DerelictJNI.sendCommand('a')
                }

                override fun onSwipeUp() {
                    super.onSwipeUp()
                    DerelictJNI.sendCommand('s')
                }

                override fun onDoubleTap() {
                    super.onDoubleTap()
                    DerelictJNI.sendCommand('x')
                }

                override fun onLongPress() {
                    super.onLongPress()
                    DerelictJNI.sendCommand('q')
                }

                override fun onSwipeDown() {
                    super.onSwipeDown()
                    DerelictJNI.sendCommand('w')
                }
            })
    }

    private fun enterStickyImmersiveMode() {
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
    }

    override fun onPostResume() {
        super.onPostResume()

        running = true

        useBestRouteForGameplayPresentation()

        Thread {
            while (running) {
                runOnUiThread {
                    if (!(application as DerelictApplication).hasPhysicalController()) {

                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            findViewById<LinearLayout>(R.id.llActions).visibility = View.VISIBLE
                            findViewById<LinearLayout>(R.id.llDirections).visibility = View.VISIBLE
                        } else {
                            findViewById<LinearLayout>(R.id.llScreenControllers).visibility =
                                View.VISIBLE
                        }

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
                            DerelictJNI.sendCommand('z')
                        }


                        findViewById<Button>(R.id.btnUseWith).setOnClickListener {
                            DerelictJNI.sendCommand('x')
                        }


                        findViewById<Button>(R.id.btnNextItem).setOnClickListener {
                            DerelictJNI.sendCommand('c')
                        }
                    } else {
                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            findViewById<LinearLayout>(R.id.llActions).visibility = View.GONE
                            findViewById<LinearLayout>(R.id.llDirections).visibility = View.GONE
                        } else {
                            findViewById<LinearLayout>(R.id.llScreenControllers).visibility =
                                View.GONE
                        }
                    }
                }

                val route = findSecondaryDisplayRouter()
                if ((this@GameActivity).presentation != null && (route == null || route.presentationDisplay == null)) {
                    presentation = null
                    runOnUiThread { (this@GameActivity).recreate() }
                }

                Thread.sleep(1000)
            }
        }.start()
    }

    override fun onPause() {
        running = false
        mView!!.onPause()
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        mView!!.onResume()
    }

    override fun onDestroy() {
        DerelictJNI.onDestroy()
        super.onDestroy()
    }


    override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {

        var toSend = '.'
        toSend = when (keyCode) {
            KeyEvent.KEYCODE_DPAD_UP, KeyEvent.KEYCODE_W -> 'w'
            KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_S -> 's'
            KeyEvent.KEYCODE_DPAD_LEFT, KeyEvent.KEYCODE_Q -> 'a'
            KeyEvent.KEYCODE_DPAD_RIGHT, KeyEvent.KEYCODE_E -> 'd'

            KeyEvent.KEYCODE_BUTTON_L1, KeyEvent.KEYCODE_A -> 'n'
            KeyEvent.KEYCODE_BUTTON_R1, KeyEvent.KEYCODE_D -> 'm'

            KeyEvent.KEYCODE_BUTTON_A, KeyEvent.KEYCODE_Z -> 'z'
            KeyEvent.KEYCODE_BUTTON_B, KeyEvent.KEYCODE_X -> 'x'
            KeyEvent.KEYCODE_BUTTON_C, KeyEvent.KEYCODE_BUTTON_Y, KeyEvent.KEYCODE_C -> 'c'
            KeyEvent.KEYCODE_BUTTON_START, KeyEvent.KEYCODE_BUTTON_X, KeyEvent.KEYCODE_ENTER -> 'q'
            else -> return super.onKeyUp(keyCode, event)
        }
        DerelictJNI.sendCommand(toSend)
        return true
    }

    private fun findSecondaryDisplayRouter(): MediaRouter.RouteInfo? {
        val mMediaRouter =
            getSystemService(Context.MEDIA_ROUTER_SERVICE) as MediaRouter
        return mMediaRouter.getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_VIDEO)
    }

    private fun useBestRouteForGameplayPresentation() {
        val mRouteInfo = findSecondaryDisplayRouter()
        if (mRouteInfo != null) {
            val presentationDisplay = mRouteInfo.presentationDisplay
            if (presentationDisplay != null) {
                useSecondaryDisplayForGameplayPresentation(presentationDisplay)
                Thread.sleep(1000)
            }
        }
    }


    private fun useSecondaryDisplayForGameplayPresentation(presentationDisplay: Display) {

        (mView!!.parent as ViewManager).removeView(mView)
        presentation =
            GamePresentation(this, presentationDisplay, mView!!)

        presentation?.show()
    }
}