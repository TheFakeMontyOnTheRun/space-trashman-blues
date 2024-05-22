#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Label.h>
#include <X11/Xlib.h>
#include <X11/Shell.h>

#define MAX_LINES 100

typedef struct {
    int x1, y1, x2, y2;
} Line;

Line lines[MAX_LINES];
int line_count = 0;
Widget draw_area;

uint8_t map[32][32];

void PopupDialog(Widget widget, XtPointer client_data, XtPointer call_data);
void SaveAsDialog(Widget widget, XtPointer client_data, XtPointer call_data);
void SaveFile(Widget widget, XtPointer client_data, XtPointer call_data);
void ClosePopup(Widget widget, XtPointer client_data, XtPointer call_data);
void ExposeCallback(Widget widget, XtPointer client_data, XEvent *event, Boolean *dispatch);
void ClickCallback(Widget widget, XtPointer client_data, XEvent *event, Boolean *dispatch);

int main(int argc, char **argv) {
    XtAppContext app_context;
    Widget topLevel, form, button, save_button, viewport;
    Display *display;
    int screen;
    Window window;

    for (int y = 0; y < 32; ++y ) {
        for (int x = 0; x < 32; ++x) {
            map[y][x] = 0;
        }
    }

    topLevel = XtVaAppInitialize(&app_context, "XawExample", NULL, 0, &argc, argv, NULL, NULL);
    
    form = XtVaCreateManagedWidget("form", formWidgetClass, topLevel, NULL);
    
    button = XtVaCreateManagedWidget("button", commandWidgetClass, form,
                                     XtNlabel, "Show Popup",
                                     XtNfromVert, NULL,
                                     XtNwidth, 100,
                                     XtNheight, 30,
                                     NULL);
    
    save_button = XtVaCreateManagedWidget("save_button", commandWidgetClass, form,
                                          XtNlabel, "Save As",
                                          XtNfromVert, button,
                                          XtNwidth, 100,
                                          XtNheight, 30,
                                          NULL);
    
    viewport = XtVaCreateManagedWidget("viewport", viewportWidgetClass, form,
                                       XtNfromVert, save_button,
                                       XtNwidth, 200,
                                       XtNheight, 200,
                                       NULL);
    
    draw_area = XtVaCreateManagedWidget("", labelWidgetClass, viewport,
                                        XtNwidth, 200,
                                        XtNheight, 200,
                                        NULL);
    
    XtAddCallback(button, XtNcallback, PopupDialog, (XtPointer)topLevel);
    XtAddCallback(save_button, XtNcallback, SaveAsDialog, (XtPointer)topLevel);
    XtAddEventHandler(draw_area, ExposureMask, False, ExposeCallback, NULL);
    XtAddEventHandler(draw_area, ButtonPressMask, False, ClickCallback, NULL);
    
    XtRealizeWidget(topLevel);
    
    display = XtDisplay(topLevel);
    window = XtWindow(draw_area);
    screen = DefaultScreen(display);
    
    XMapWindow(display, window);
    
    XtAppMainLoop(app_context);
    
    return 0;
}

void PopupDialog(Widget widget, XtPointer client_data, XtPointer call_data) {
    Widget popup, dialog, popup_button;
    XtAppContext app_context = XtWidgetToApplicationContext(widget);

    popup = XtVaCreatePopupShell("popup", transientShellWidgetClass, (Widget)client_data,
                                 XtNwidth, 200,
                                 XtNheight, 100,
                                 XtNlabel, "Popup",
                                 NULL);

    dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass, popup,
                                     XtNlabel, "Button clicked!",
                                     NULL);
    
    popup_button = XtVaCreateManagedWidget("popup_button", commandWidgetClass, dialog,
                                           XtNlabel, "OK",
                                           NULL);

    XtAddCallback(popup_button, XtNcallback, ClosePopup, (XtPointer)popup);

    XtPopup(popup, XtGrabNone);
}

void SaveAsDialog(Widget widget, XtPointer client_data, XtPointer call_data) {
    Widget popup, dialog, save_button;
    XtAppContext app_context = XtWidgetToApplicationContext(widget);

    popup = XtVaCreatePopupShell("save_popup", transientShellWidgetClass, (Widget)client_data,
                                 XtNwidth, 300,
                                 XtNheight, 150,
                                 XtNlabel, "Save As",
                                 NULL);

    dialog = XtVaCreateManagedWidget("save_dialog", dialogWidgetClass, popup,
                                     XtNlabel, "Enter filename:",
                                     XtNvalue, "",
                                     NULL);
    
    save_button = XtVaCreateManagedWidget("save_button", commandWidgetClass, dialog,
                                          XtNlabel, "Save",
                                          NULL);

    XtAddCallback(save_button, XtNcallback, SaveFile, (XtPointer)dialog);
    XtAddCallback(save_button, XtNcallback, ClosePopup, (XtPointer)popup);

    XtPopup(popup, XtGrabNone);
}

void SaveFile(Widget widget, XtPointer client_data, XtPointer call_data) {
    Widget dialog = (Widget)client_data;
    char *filename = XawDialogGetValueString(dialog);

    if (filename != NULL && *filename != '\0') {
        FILE *file = fopen(filename, "w");
        if (file != NULL) {
            for (int i = 0; i < line_count; i++) {
                fprintf(file, "%d %d %d %d\n", lines[i].x1, lines[i].y1, lines[i].x2, lines[i].y2);
            }
            fclose(file);
        }
    }
}

void ClosePopup(Widget widget, XtPointer client_data, XtPointer call_data) {
    Widget popup = (Widget)client_data;
    XtDestroyWidget(popup);
}



void ExposeCallback(Widget widget, XtPointer client_data, XEvent *event, Boolean *dispatch) {
    if (event->type == Expose) {
        Display *display = XtDisplay(widget);
        Window window = XtWindow(widget);
        XWindowAttributes attr;
        XGetWindowAttributes(display, window, &attr);

        GC gc;
        XGCValues values;

        gc = XCreateGC(display, window, 0, &values);
        XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));

        for (int y = 1; y <= 32; ++y) {
            XDrawLine(display, window, gc, 0, y * (attr.height / 32), attr.width, y * (attr.height / 32));
        }

        for (int x = 1; x <= 32; ++x) {
            XDrawLine(display, window, gc, x * (attr.width / 32), 0, x * (attr.width / 32), attr.height);
        }

        for (int i = 0; i < line_count; i++) {
            XDrawLine(display, window, gc, lines[i].x1, lines[i].y1, lines[i].x2, lines[i].y2);
        }

        for (int y = 0; y < 32; ++y ) {
            for (int x = 0; x < 32; ++x ) {
                if (map[y][x]) {
                    values.foreground = rand() % (1 << 24);
                    XChangeGC(display, gc, GCForeground, &values);
                    XFillRectangle(display, window, gc,
                                   (x + 0) * (attr.width / 32), (y + 0) * (attr.height / 32),
                                   (attr.width / 32), (attr.height / 32) );

                }
            }
        }

        XFreeGC(display, gc);
    }
}

void ClickCallback(Widget widget, XtPointer client_data, XEvent *event, Boolean *dispatch) {
    if (event->type == ButtonPress) {
        Display *display = XtDisplay(draw_area);
        XButtonEvent *button_event = (XButtonEvent *)event;
        Window window = XtWindow(draw_area);
        XWindowAttributes attr;
        XGetWindowAttributes(display, window, &attr);

        map[(32 * button_event->y) / attr.height][ (32 * button_event->x ) / attr.width] = 1;

        if (line_count < MAX_LINES) {
            lines[line_count].x1 = 10;
            lines[line_count].y1 = 10;
            lines[line_count].x2 = button_event->x;
            lines[line_count].y2 = button_event->y;
            line_count++;

            XClearArea(XtDisplay(widget), XtWindow(widget), 0, 0, 0, 0, True);
        }
    }
}
