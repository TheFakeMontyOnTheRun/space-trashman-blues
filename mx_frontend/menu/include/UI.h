/*
* Created by Daniel Monteiro on 2019-08-02.
*/

#ifndef THE_MISTRAL_REPORT_UI_H
#define THE_MISTRAL_REPORT_UI_H

void drawWindow( const int x, const int y, const unsigned int dx, const unsigned int dy, const char* title);
void drawTextWindow( const int x, const int y, const unsigned int dx, const unsigned int dy, const char* __restrict__ title, const char* __restrict__ content);
void drawImageWindow( const int x, const int y, const unsigned int dx, const unsigned int dy, const char* __restrict__ title, const struct Bitmap* __restrict__ content);
int drawAppearingWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char * title, long remainingTime);

void redrawHUD(void);
void updateMap(void);

#endif /*THE_MISTRAL_REPORT_UI_H*/