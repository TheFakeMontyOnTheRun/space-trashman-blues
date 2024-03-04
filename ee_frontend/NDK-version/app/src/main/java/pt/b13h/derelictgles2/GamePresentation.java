package pt.b13h.derelictgles2;

import android.app.Presentation;
import android.content.Context;
import android.os.Bundle;
import android.view.Display;
import android.view.View;
import android.widget.ImageView;

public class GamePresentation extends Presentation {

    private final View canvas;

    public GamePresentation(Context context, Display display, View gameView) {
        super(context, display);

        this.canvas = gameView;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(canvas);
    }
}
