package pt.b13h.themistralreport;

import android.app.Presentation;
import android.content.Context;
import android.os.Bundle;
import android.view.Display;
import android.widget.ImageView;

public class GamePresentation extends Presentation {

    private final ImageView canvas;

    public GamePresentation(Context context, Display display, ImageView gameView) {
        super(context, display);

        this.canvas = gameView;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(canvas);
    }
}
