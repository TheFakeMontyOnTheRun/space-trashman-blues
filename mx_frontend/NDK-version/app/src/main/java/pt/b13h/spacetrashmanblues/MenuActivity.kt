package pt.b13h.spacetrashmanblues

import android.content.Intent
import android.media.MediaPlayer
import android.net.Uri
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.CompoundButton
import android.widget.Switch
import androidx.appcompat.app.AppCompatActivity
import pt.b13h.derelict.R


class MenuActivity : AppCompatActivity(), View.OnClickListener,
    CompoundButton.OnCheckedChangeListener {

    private lateinit var swEnableSound: Switch
    private var mp: MediaPlayer? = null

    override fun onPause() {
        if (mp != null) {
            mp!!.pause()
            mp!!.release()
        }
        mp = null
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        if (swEnableSound.isChecked) {
            mp = MediaPlayer.create(this, R.raw.derelicttheme)
            mp!!.isLooping = true
            mp!!.start()
        }
    }


    override fun onCheckedChanged(buttonView: CompoundButton, isChecked: Boolean) {
        if (isChecked) {
            mp = MediaPlayer.create(this, R.raw.derelicttheme)
            mp!!.isLooping = true
            mp!!.start()
        } else if (mp != null) {
            mp!!.stop()
            mp = null
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_menu)
        findViewById<View>(R.id.btnStartGame).setOnClickListener(this)
        findViewById<View>(R.id.btnHelp).setOnClickListener(this)
        swEnableSound = findViewById(R.id.swEnableSound)
        (findViewById<View>(R.id.btnStartGame) as Button).requestFocus()
        swEnableSound.isChecked = (application as DerelictApplication)
            .mayEnableSound()

        swEnableSound.setOnCheckedChangeListener(this)
    }

    override fun onClick(v: View) {
        var intent: Intent? = null
        when (v.id) {
            R.id.btnStartGame -> {
                intent = Intent(this, MainActivity::class.java)
                val bundle = Bundle()
                bundle.putBoolean("hasSound", swEnableSound.isChecked)
                intent.putExtras(bundle)
            }

            R.id.btnHelp -> {
                val uri = Uri.parse("https://montyontherun.neocities.org/derelict")
                intent = Intent(Intent.ACTION_VIEW, uri)
            }
        }
        this.startActivity(intent!!)
    }
}