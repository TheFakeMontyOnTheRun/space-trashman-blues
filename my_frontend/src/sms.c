#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sms.h>
#include <graphics.h>

#include "Core.h"
#include "Derelict.h"

extern struct ObjectNode* focusedItem;
extern struct ObjectNode* roomItem;
extern int accessGrantedToSafe;

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

int currentlyInGraphics = FALSE;
void backToGraphics();

unsigned char pal1[] = {0x00, 0x20, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
                        0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};

unsigned char pal2[] = {0x00, 0x03, 0x08, 0x28, 0x02, 0x22, 0x0A, 0x2A,
                        0x15, 0x35, 0x1D, 0x3D, 0x17, 0x37, 0x1F, 0x3F};


uint8_t font[] = {
          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 // space
        , 0x10,0x38,0x38,0x10,0x10,0x00,0x10,0x00
        , 0x6c,0x6c,0x48,0x00,0x00,0x00,0x00,0x00
        , 0x00,0x28,0x7c,0x28,0x28,0x7c,0x28,0x00
        , 0x20,0x38,0x40,0x30,0x08,0x70,0x10,0x00
        , 0x64,0x64,0x08,0x10,0x20,0x4c,0x4c,0x00
        , 0x20,0x50,0x50,0x20,0x54,0x48,0x34,0x00
        , 0x30,0x30,0x20,0x00,0x00,0x00,0x00,0x00
        , 0x10,0x20,0x20,0x20,0x20,0x20,0x10,0x00
        , 0x20,0x10,0x10,0x10,0x10,0x10,0x20,0x00
        , 0x00,0x28,0x38,0x7c,0x38,0x28,0x00,0x00
        , 0x00,0x10,0x10,0x7c,0x10,0x10,0x00,0x00
        , 0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x20
        , 0x00,0x00,0x00,0x7c,0x00,0x00,0x00,0x00
        , 0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00
        , 0x00,0x04,0x08,0x10,0x20,0x40,0x00,0x00 // /space - 15




        , 0x38,0x44,0x4c,0x54,0x64,0x44,0x38,0x00 // 0
        , 0x10,0x30,0x10,0x10,0x10,0x10,0x38,0x00
        , 0x38,0x44,0x04,0x18,0x20,0x40,0x7c,0x00
        , 0x38,0x44,0x04,0x38,0x04,0x44,0x38,0x00
        , 0x08,0x18,0x28,0x48,0x7c,0x08,0x08,0x00
        , 0x7c,0x40,0x40,0x78,0x04,0x44,0x38,0x00
        , 0x18,0x20,0x40,0x78,0x44,0x44,0x38,0x00
        , 0x7c,0x04,0x08,0x10,0x20,0x20,0x20,0x00
        , 0x38,0x44,0x44,0x38,0x44,0x44,0x38,0x00
        , 0x38,0x44,0x44,0x3c,0x04,0x08,0x30,0x00
        , 0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00
        , 0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x20
        , 0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x00
        , 0x00,0x00,0x7c,0x00,0x00,0x7c,0x00,0x00
        , 0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x00
        , 0x38,0x44,0x04,0x18,0x10,0x00,0x10,0x00
        , 0x38,0x44,0x5c,0x54,0x5c,0x40,0x38,0x00 // /0

        , 0x38,0x44,0x44,0x44,0x7c,0x44,0x44,0x00 // a
        , 0x78,0x44,0x44,0x78,0x44,0x44,0x78,0x00
        , 0x38,0x44,0x40,0x40,0x40,0x44,0x38,0x00
        , 0x78,0x44,0x44,0x44,0x44,0x44,0x78,0x00
        , 0x7c,0x40,0x40,0x78,0x40,0x40,0x7c,0x00
        , 0x7c,0x40,0x40,0x78,0x40,0x40,0x40,0x00
        , 0x38,0x44,0x40,0x5c,0x44,0x44,0x3c,0x00
        , 0x44,0x44,0x44,0x7c,0x44,0x44,0x44,0x00
        , 0x38,0x10,0x10,0x10,0x10,0x10,0x38,0x00
        , 0x04,0x04,0x04,0x04,0x44,0x44,0x38,0x00
        , 0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x00
        , 0x40,0x40,0x40,0x40,0x40,0x40,0x7c,0x00
        , 0x44,0x6c,0x54,0x44,0x44,0x44,0x44,0x00
        , 0x44,0x64,0x54,0x4c,0x44,0x44,0x44,0x00
        , 0x38,0x44,0x44,0x44,0x44,0x44,0x38,0x00
        , 0x78,0x44,0x44,0x78,0x40,0x40,0x40,0x00
        , 0x38,0x44,0x44,0x44,0x54,0x48,0x34,0x00
        , 0x78,0x44,0x44,0x78,0x48,0x44,0x44,0x00
        , 0x38,0x44,0x40,0x38,0x04,0x44,0x38,0x00
        , 0x7c,0x10,0x10,0x10,0x10,0x10,0x10,0x00
        , 0x44,0x44,0x44,0x44,0x44,0x44,0x38,0x00
        , 0x44,0x44,0x44,0x44,0x44,0x28,0x10,0x00
        , 0x44,0x44,0x54,0x54,0x54,0x54,0x28,0x00
        , 0x44,0x44,0x28,0x10,0x28,0x44,0x44,0x00
        , 0x44,0x44,0x44,0x28,0x10,0x10,0x10,0x00
        , 0x78,0x08,0x10,0x20,0x40,0x40,0x78,0x00
        , 0x38,0x20,0x20,0x20,0x20,0x20,0x38,0x00
        , 0x00,0x40,0x20,0x10,0x08,0x04,0x00,0x00
        , 0x38,0x08,0x08,0x08,0x08,0x08,0x38,0x00
        , 0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00
        , 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc
        , 0x30,0x30,0x10,0x00,0x00,0x00,0x00,0x00
        , 0x00,0x00,0x38,0x04,0x3c,0x44,0x3c,0x00
        , 0x40,0x40,0x78,0x44,0x44,0x44,0x78,0x00
        , 0x00,0x00,0x38,0x44,0x40,0x44,0x38,0x00
        , 0x04,0x04,0x3c,0x44,0x44,0x44,0x3c,0x00
        , 0x00,0x00,0x38,0x44,0x78,0x40,0x38,0x00
        , 0x18,0x20,0x20,0x78,0x20,0x20,0x20,0x00
        , 0x00,0x00,0x3c,0x44,0x44,0x3c,0x04,0x38
        , 0x40,0x40,0x70,0x48,0x48,0x48,0x48,0x00
        , 0x10,0x00,0x10,0x10,0x10,0x10,0x18,0x00
        , 0x08,0x00,0x18,0x08,0x08,0x08,0x48,0x30
        , 0x40,0x40,0x48,0x50,0x60,0x50,0x48,0x00
        , 0x10,0x10,0x10,0x10,0x10,0x10,0x18,0x00
        , 0x00,0x00,0x68,0x54,0x54,0x44,0x44,0x00
        , 0x00,0x00,0x70,0x48,0x48,0x48,0x48,0x00
        , 0x00,0x00,0x38,0x44,0x44,0x44,0x38,0x00
        , 0x00,0x00,0x78,0x44,0x44,0x44,0x78,0x40
        , 0x00,0x00,0x3c,0x44,0x44,0x44,0x3c,0x04
        , 0x00,0x00,0x58,0x24,0x20,0x20,0x70,0x00
        , 0x00,0x00,0x38,0x40,0x38,0x04,0x38,0x00
        , 0x00,0x20,0x78,0x20,0x20,0x28,0x10,0x00
        , 0x00,0x00,0x48,0x48,0x48,0x58,0x28,0x00
        , 0x00,0x00,0x44,0x44,0x44,0x28,0x10,0x00
        , 0x00,0x00,0x44,0x44,0x54,0x7c,0x28,0x00
        , 0x00,0x00,0x48,0x48,0x30,0x48,0x48,0x00
        , 0x00,0x00,0x48,0x48,0x48,0x38,0x10,0x60
        , 0x00,0x00,0x78,0x08,0x30,0x40,0x78,0x00
        , 0x18,0x20,0x20,0x60,0x20,0x20,0x18,0x00
        , 0x10,0x10,0x10,0x00,0x10,0x10,0x10,0x00
        , 0x30,0x08,0x08,0x0c,0x08,0x08,0x30,0x00
        , 0x28,0x50,0x00,0x00,0x00,0x00,0x00,0x00
        , 0x10,0x38,0x6c,0x44,0x44,0x7c,0x00,0x00
};

void clrscr() {
}

void clearTextScreen() {
    clg();
}


uint8_t buffer[64 * 64];

void setup_mode2() {
    if (!currentlyInGraphics) {
        memset(&buffer[0], 0, 64 * 64);
        clg();
    }
    currentlyInGraphics = TRUE;
}

void init() {
    clear_vram();
    load_tiles(standard_font, 0, 255, 1);
    load_palette(pal1, 0, 16);
    load_palette(pal2, 16, 16);
    set_vdp_reg(VDP_REG_FLAGS1, VDP_REG_FLAGS1_BIT7 | VDP_REG_FLAGS1_SCREEN);
    clg();
}

char *menuItems[] = {
        "Use/Toggle current item",
        "Use current item with...",
        "Use/Pick object on the room...",
        "Drop object in hand",
        "Next item in inventory",
        "Next room item in focus",
        "Toogle item desc/room desc",
        "Back to room",
};

void setup_text_mode() {
    currentlyInGraphics = FALSE;
}

void graphicsFlush();

void renderScene();

int cooldown = 0;
int cursorPosition = 0;


void show_text(int _x, int y, char *text) {

    int len = strlen(text);
    char *ptr = text;
    int c = 0;
    int chary = 0;
    int x = _x;

    for (; c < len; ++c ) {

        char cha = *ptr;

        ++x;

        if (cha == '\n') {
            ++y;
            x = _x;
        }

        if (cha != ' ') {

            for (chary = 0; chary < 8; ++chary) {
                uint8_t ch = font[((cha - 32) * 8) + chary];
                int baseX = (x * 8);
                int baseY = (y * 8) + chary;

                if (ch & 1) {
                    plot(baseX + 7, baseY);
                }

                if (ch & 2) {
                    plot(baseX + 6, baseY);
                }

                if (ch & 4) {
                    plot(baseX + 5, baseY);
                }

                if (ch & 8) {
                    plot(baseX + 4, baseY);
                }

                if (ch & 16) {
                    plot(baseX + 3, baseY);
                }

                if (ch & 32) {
                    plot(baseX + 2, baseY);
                }

                if (ch & 64) {
                    plot(baseX + 1, baseY);
                }

                if (ch & 128) {
                    plot(baseX, baseY);
                }
            }
        }
        ++ptr;
    }
}

void backToGraphics() {
    clrscr();
    setup_mode2();
}

void showMessage(const char *message) {
    int keepGoing = 1;
    clearTextScreen();
    setup_text_mode();

    show_text(1, 1, (char *) message);
    show_text(1, 3, "Press any button to continue");

    while (keepGoing) {
        if (read_joypad1() & JOY_FIREA) {
            keepGoing = 0;
        }
    }

    clg();
}

void titleScreen() {
    int keepGoing = 1;

    setup_text_mode();

    show_text(1, 1, "Space Mare Imperium - Derelict");
    show_text(1, 2, "by Daniel \"MontyOnTheRun\"Monteiro");
    show_text(1, 3, "Press any button to start");

    while (keepGoing) {
        if (read_joypad1() & JOY_FIREA) {
            keepGoing = 0;
        }
    }

    clg();
}

void pauseMenu() {
    int keepGoing = 1;
    int refresh = 1;
    int itemDesc = 1;
    struct Room *room = getRoom(getPlayerRoom());

    setup_text_mode();

    while (keepGoing) {

        switch (getGameStatus()) {
            case kBadVictory:
                showMessage("Victory! Too bad you didn't survive\nto tell the story\n\n\n\n\n\n");
                while (1);
                break;

            case kBadGameOver:
                showMessage("You're dead! And so are millions of\n"
                            "other people on the path of\n"
                            "destruction faulty reactor\n\n\n\n\n\n");
                while (1);
                break;

            case kGoodVictory:
                showMessage("Victory! You managed to destroy the\nship and get out alive\n\n\n\n\n\n");
                while (1);
                break;

            case kGoodGameOver:
                showMessage("You failed! While you fled the ship\n"
                            "alive, you failed to prevent the \n"
                            "worstscenario and now EVERYBODY is\n"
                            "dead (and that includes you!)\n\n\n\n\n");
                while (1);
                break;

            default:
            case kNormalGameplay:
                break;
        }

        if (refresh) {
            int i = 0;
            refresh = 0;

            clearTextScreen();


            show_text(1, 1, "Object at room:");

            if (focusedItem != NULL) {
                struct Item *item = getItem(focusedItem->item);

                if (item->active) {
                    show_text(1, 2, "*");
                }

                show_text(2, 2, item->description);
            }


            if (roomItem != NULL) {
                struct Item *item = getItem(roomItem->item);

                if (item->active) {
                    show_text(1, 3, "*");
                }

                show_text(2, 3, item->description);

                if (itemDesc) {
                    show_text(1, 4, item->info);
                }
            }

            if (!itemDesc) {
                show_text(1, 2, " ");
                show_text(2, 2, room->description);
                show_text(1, 3, room->info);
            }

            for (i = 0; i < 8; ++i) {
                if (i == cursorPosition) {
                    show_text(1, 16 + i, ">");
                }
                show_text(2, 16 + i, menuItems[i]);
            }
            cooldown = 4000;
        }

        cooldown--;

        if (cooldown < 0) {
            unsigned int key = read_joypad1();
            cooldown = 0;


/*
char *menuItems[] = {
 0       "Use/Toggle current item",
 1       "Use current item with...",
 2       "Pick",
 3       "Drop",
 4       "Next item in inventory",
 5       "Next room item in focus",
 6       "Toogle item desc/room desc",
 7       "Back to room",
};
*/
            if (key & JOY_FIREA) {
                switch (cursorPosition) {
                    case 0:
                        useObjectNamed(getItem(focusedItem->item)->description);
                        break;


                    case 1:
                        interactWithItemInRoom();
                        break;

                    case 2:
                        pickItem();
                        break;

                    case 3:
                        dropItem();
                        break;
                    case 4:
                        nextItemInHand();
                        break;

                    case 5:
                        nextItemInRoom();
                        break;

                    case 6:
                        itemDesc = !itemDesc;
                        break;
                    case 7:
                        keepGoing = 0;
                        clg();
                        return;
                }
                refresh = 1;
            }

            if (key & JOY_FIREB) {
                focusedItem = focusedItem->next;
                if (!focusedItem) {
                    focusedItem = getPlayerItems();
                }
                refresh = 1;
            }


            if (key & JOY_UP) {
                cursorPosition--;
                refresh = 1;
                if (cursorPosition < 0) {
                    cursorPosition = 0;
                }
            }

            if (key & JOY_DOWN) {
                cursorPosition++;
                refresh = 1;
                if (cursorPosition >= 8) {
                    cursorPosition = 7;
                }
            }
        }
    }
    clg();
}


uint8_t getKey() {
    unsigned int key = read_joypad1();

    if (key & JOY_UP) {
        return 'w';
    }

    if (key & JOY_LEFT) {
        if (key & JOY_FIREB) {
            return 'a';
        }
        return 'q';
    }


    if (key & JOY_RIGHT) {
        if (key & JOY_FIREB) {
            return 'd';
        }
        return 'e';
    }

    if (key & JOY_DOWN) {
        return 's';
    }

    if (key & JOY_FIREA) {
        pauseMenu();
        return 'p';
    }

    return '.';
}

void shutdownGraphics() {
}

void clearGraphics() {
}

void graphicsFlush() {
    uint8_t *ptr = &buffer[0];
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            uint8_t pixel = *ptr;
            if (pixel & 1) {
                if (~(pixel & 2)) {
                    plot(x, y);
                }
            } else {
                if (pixel & 2) {
                    unplot(x, y);
                }
            }

            pixel = pixel << 1;
            *ptr = pixel;
            ptr++;
        }
    }
}

void hLine(uint8_t x0, uint8_t x1, uint8_t y0) {
    uint8_t *ptr;
    uint8_t _x0 = x0;
    uint8_t _x1 = x1;

    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }

    ptr = &buffer[(y0 * 64) + _x0];
    for (uint8_t x = _x0; x <= _x1; ++x) {
        {
            if (y0 >= 64 || x >= 64) return;

            ptr++;
            *ptr |= 1;

        }
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {

    uint8_t *ptr;
    uint8_t _y0 = y0;
    uint8_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    ptr = &buffer[(_y0 * 64) + x0];

    for (uint8_t y = _y0; y <= _y1; ++y) {
        if (y >= 64 || x0 >= 64) return;

        *ptr |= 1;
        ptr += 64;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {

    if (y >= 64 || x >= 64) return;

    buffer[(y * 64) + x] |= 1;
}

void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    int offset;
    uint8_t *ptr;
    if (x0 == x1) {

        uint8_t _y0 = y0;
        uint8_t _y1 = y1;

        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }


        offset = (_y0 * 64) + x0;
        ptr = &buffer[offset];

        for (uint8_t y = _y0; y <= _y1; ++y) {


            {
                ptr += 64;
                *ptr |= 1;

            }
        }
        return;
    }

    if (y0 == y1) {
        uint8_t _x0 = x0;
        uint8_t _x1 = x1;

        if (x0 > x1) {
            _x0 = x1;
            _x1 = x0;
        }


        offset = (y0 * 64) + _x0;
        ptr = &buffer[offset];
        for (uint8_t x = _x0; x <= _x1; ++x) {
            {
                ptr++;
                *ptr |= 1;

            }
        }
        return;
    }


    {
        //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

        uint8_t dx = abs(x1 - x0);
        int8_t sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;  /* error value e_xy */
        int16_t e2;
        int offset = (y0 * 64) + x0;

        if (sy > 0) {
            while (1) {

                buffer[offset] |= 1;

                /* loop */
                if (x0 == x1 && y0 == y1) return;
                e2 = err << 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                    offset += sx;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0++;
                    offset += 64;
                }
            }
        } else {
            while (1) {

                buffer[offset] |= 1;

                /* loop */
                if (x0 == x1 && y0 == y1) return;
                e2 = err << 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                    offset += sx;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0--;
                    offset -= 64;
                }
            }
        }

    }
}

void printSituation() {}