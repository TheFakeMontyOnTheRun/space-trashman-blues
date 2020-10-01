package pt.b13h.themistralreport

import android.app.Presentation
import android.content.Context
import android.content.res.Configuration
import android.graphics.Bitmap
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.MediaRouter
import android.media.SoundPool
import android.os.Build
import android.os.Bundle
import android.view.Display
import android.view.KeyEvent
import android.view.View
import android.view.ViewManager
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity(), View.OnClickListener {

    private var soundPool : SoundPool? = null
    private var presentation: Presentation? = null
    private var sounds = IntArray(8)
    private var pixels = ByteArray(320 * 240 * 4)
    val bitmap: Bitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888)
    private var running = false

    private fun initAudio() {
        soundPool = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            SoundPool.Builder().setAudioAttributes(AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build()).build()
        } else {
            SoundPool(5, AudioManager.STREAM_MUSIC, 0 )
        }

        sounds[0] = soundPool!!.load(this, R.raw.menu_move, 1)
        sounds[1] = soundPool!!.load(this, R.raw.menu_select, 1)
        sounds[2] = soundPool!!.load(this, R.raw.gotclue, 1)
        sounds[3] = soundPool!!.load(this, R.raw.detected2, 1)
        sounds[4] = soundPool!!.load(this, R.raw.detected, 1)
        sounds[5] = soundPool!!.load(this, R.raw.fire, 1)
        sounds[6] = soundPool!!.load(this, R.raw.enemyfire, 1)
        sounds[7] = soundPool!!.load(this, R.raw.hurt, 1)
    }

    private fun enterStickyImmersiveMode() {
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        if (Build.VERSION.SDK_INT >= 29) {
            enterStickyImmersiveMode()
        } else {
            window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN
        }


        setContentView(R.layout.activity_main)

        if (savedInstanceState == null ) {
            MistralJNI.initAssets(resources.assets)
        }


        if  ( (application as MistralApplication).mayEnableSound() ) {
            initAudio()
        } else {
            soundPool = null
        }


        imageView.setImageBitmap(bitmap)
    }

    override fun onDestroy() {

        soundPool?.release()
        super.onDestroy()
    }

    override fun onPostResume() {
        super.onPostResume()

        running = true

        useBestRouteForGameplayPresentation()

        Thread( Runnable {
            while (running) {
                Thread.sleep(1000)

                runOnUiThread {
                    if  ( !(application as MistralApplication).hasPhysicalController() ) {

                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            llActions.visibility = View.VISIBLE
                            llDirections.visibility = View.VISIBLE
                        } else {
                            llScreenControllers.visibility = View.VISIBLE
                        }

                        btnUp.setOnClickListener(this)
                        btnDown.setOnClickListener(this)
                        btnFire.setOnClickListener(this)
                        btnPick.setOnClickListener(this)
                        btnAim.setOnClickListener(this)
                        btnLeft.setOnClickListener(this)
                        btnRight.setOnClickListener(this)
                        btnStrafeLeft.setOnClickListener(this)
                        btnStrafeRight.setOnClickListener(this)
                    } else {
                        if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            llActions.visibility = View.GONE
                            llDirections.visibility = View.GONE
                        } else {
                            llScreenControllers.visibility = View.GONE
                        }
                    }
                }

                val route = findSecundaryDisplayRouter()
                if ((this@MainActivity).presentation != null && (route == null || route.presentationDisplay == null)) {
                    presentation = null
                    runOnUiThread { (this@MainActivity).recreate() }
                }


            }
        }).start()

        Thread(Runnable {
            while (running) {
                Thread.sleep(75)
                runOnUiThread { redraw() }
            }
        }
        ).start()

        if (soundPool != null) {
            Thread(Runnable {

                while (running) {
                    Thread.sleep(10)
                    when (val sound = MistralJNI.getSoundToPlay()) {
                        0, 1, 2, 3, 4, 5, 6, 7, 8 -> soundPool!!.play(sounds[sound], 1f, 1f, 0, 0, 1f)
                    }

                }
            }
            ).start()
        }
    }

    override fun onBackPressed() {
        if (MistralJNI.isOnMainMenu() != 0 ) {
            super.onBackPressed()
        } else {
            MistralJNI.sendCommand('q')
        }
    }
    override fun onPause() {
        super.onPause()
        running = false
        presentation?.hide()
        presentation?.dismiss()
        presentation?.cancel()
    }

    private fun redraw() {
        MistralJNI.getPixelsFromNative(pixels)
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(pixels))
        imageView.invalidate()
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
            KeyEvent.KEYCODE_BUTTON_B, KeyEvent.KEYCODE_X-> 'x'
            KeyEvent.KEYCODE_BUTTON_C, KeyEvent.KEYCODE_BUTTON_Y, KeyEvent.KEYCODE_C-> 'c'
            KeyEvent.KEYCODE_BUTTON_START, KeyEvent.KEYCODE_BUTTON_X, KeyEvent.KEYCODE_ENTER -> 'q'
            else -> return super.onKeyUp(keyCode, event )
        }
        MistralJNI.sendCommand(toSend)
        return true
    }


    override fun onClick(v: View) {
        var toSend = '.'
        when (v.id) {
            R.id.btnUp -> toSend = 'w'
            R.id.btnDown -> toSend = 's'
            R.id.btnLeft -> toSend = 'a'
            R.id.btnRight -> toSend = 'd'
            R.id.btnFire -> toSend = 'z'
            R.id.btnAim -> toSend = 'x'
            R.id.btnPick -> toSend = 'c'
            R.id.btnStrafeLeft-> toSend = 'n'
            R.id.btnStrafeRight-> toSend = 'm'
        }
        MistralJNI.sendCommand(toSend)
    }


    private fun findSecundaryDisplayRouter(): MediaRouter.RouteInfo? {
        val mMediaRouter =
            getSystemService(Context.MEDIA_ROUTER_SERVICE) as MediaRouter
        return mMediaRouter.getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_VIDEO)
    }

    private fun useBestRouteForGameplayPresentation() {
        val mRouteInfo = findSecundaryDisplayRouter()
        if (mRouteInfo != null) {
            val presentationDisplay = mRouteInfo.presentationDisplay
            if (presentationDisplay != null) {
                useSecundaryDisplayForGameplayPresentation(presentationDisplay)
                Thread.sleep(1000)
            }
        }
    }


    private fun useSecundaryDisplayForGameplayPresentation(presentationDisplay: Display) {

        (imageView.parent as ViewManager).removeView(imageView)
        presentation =
            GamePresentation(this, presentationDisplay, imageView)

        presentation?.show()
    }
}