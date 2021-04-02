/*
* Created by Daniel Monteiro on 2019-08-02.
*/

#ifndef THE_MISTRAL_REPORT_UI_H
#define THE_MISTRAL_REPORT_UI_H

void drawWindow(const int x, const int y, const int dx, const int dy, const char *title);

void drawTextWindow(const int x, const int y, const int dx, const int dy, const char *__restrict__ title,
                    const char *__restrict__ content);

void drawImageWindow(const int x, const int y, const int dx, const int dy, const char *__restrict__ title,
                     const struct Bitmap *__restrict__ content);

#endif /*THE_MISTRAL_REPORT_UI_H*/
