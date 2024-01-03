package pt.b13h.derelictgles2

import android.content.Context
import android.view.GestureDetector
import android.view.GestureDetector.SimpleOnGestureListener
import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import kotlin.math.abs

/**
 * Detects left and right swipes across a view.
 * http://stackoverflow.com/questions/4139288/android-how-to-handle-right-to-left-swipe-gestures
 */
open class OnSwipeTouchListener(context: Context) : OnTouchListener {
	private val gestureDetector: GestureDetector
	open fun onSwipeLeft() {}
	open fun onSwipeRight() {}
	open fun onSwipeUp() {}
	open fun onSwipeDown() {}
	open fun onDoubleTap() {}
	open fun onLongPress() {}

	companion object {
		private const val SWIPE_DISTANCE_THRESHOLD = 100
		private const val SWIPE_VELOCITY_THRESHOLD = 100
	}

	init {
		gestureDetector = GestureDetector(context, GestureListener())
	}

	override fun onTouch(v: View, event: MotionEvent): Boolean {
		return gestureDetector.onTouchEvent(event)
	}

	private inner class GestureListener : SimpleOnGestureListener() {
		override fun onDown(e: MotionEvent): Boolean {
			return true
		}

		override fun onFling(
			e1: MotionEvent?,
			e2: MotionEvent,
			velocityX: Float,
			velocityY: Float
		): Boolean {
			val distanceX = e2.x - e1!!.x
			val distanceY = e2.y - e1.y
			if (abs(distanceX) > abs(distanceY) && abs(distanceX) > Companion.SWIPE_DISTANCE_THRESHOLD && abs(
					velocityX
				) > Companion.SWIPE_VELOCITY_THRESHOLD
			) {
				if (distanceX > 0) {
					onSwipeRight()
				} else {
					onSwipeLeft()
				}
				return true
			}
			if (abs(distanceX) < abs(distanceY) && abs(distanceY) > Companion.SWIPE_DISTANCE_THRESHOLD && abs(
					velocityY
				) > Companion.SWIPE_VELOCITY_THRESHOLD
			) {
				if (distanceY > 0) {
					onSwipeDown()
				} else {
					onSwipeUp()
				}
				return true
			}
			return false
		}

		override fun onDoubleTap(e: MotionEvent): Boolean {
			this@OnSwipeTouchListener.onDoubleTap()
			return true
		}

		override fun onLongPress(e: MotionEvent) {
			super.onLongPress(e)
			this@OnSwipeTouchListener.onLongPress()
		}
	}
}