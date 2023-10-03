/*
* Created by Daniel Monteiro on 2019-08-02.
*/

#ifndef UI_H
#define UI_H

void drawWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title);

void drawTextWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                    const char *content);

void drawImageWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                     struct Bitmap *content);

int drawAppearingWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                        long remainingTime);

void updateMap(void);

#endif /*UI_H*/
