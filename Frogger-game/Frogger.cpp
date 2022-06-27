#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
using namespace std;

extern "C" {
#include "SDL2-2.0.10/include/SDL.h"
#include "SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH    516
#define SCREEN_HEIGHT   550
#define TILE_SIZE 37
#define MAP_HEIGH 504
#define PORUSZAJACE_PO_X 10
#define MAX_TIME 50


void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while (*text) {
        c = *text & 255;
        px = (c % 16) * 8;
        py = (c / 16) * 8;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += 8;
        text++;

    };
};

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
    SDL_Rect dest;
    dest.x = x - sprite->w;
    dest.y = y - sprite->h;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32*)p = color;
};

void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
    for (int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;

    };
};

void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
    Uint32 outlineColor, Uint32 fillColor) {
    int i;
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for (i = y + 1; i < y + k - 1; i++)
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void KolizjaF(int** kolizja, int dlugosc_x, int dlugosc_y, int x, int y, int wartosc) {
    if (x + dlugosc_x > SCREEN_WIDTH) {
        dlugosc_x = SCREEN_WIDTH - x;

    }
    else if (x <= 1) {
        dlugosc_x += x - 1;
        x = 2;

    }

    for (int i = 0; i < dlugosc_x; i++) {
        for (int j = 0; j < dlugosc_y; j++) {
            kolizja[x + i][y + j] = wartosc;

        }

    }
    if (wartosc % 2 == 1) {
        for (int i = 0; i < dlugosc_y; i++) {
            kolizja[x - 1][y + i] = wartosc - 1;
            kolizja[x + dlugosc_x][y + i] = wartosc - 1;

        }

    }

}

void DrawCar(SDL_Surface* screen, SDL_Surface* sprite, int x, int y, int** kolizja, int wartosc_kolizja) {
    if (x > SCREEN_WIDTH * 2) {
        x -= SCREEN_WIDTH * 2;

    }
    DrawSurface(screen, sprite, x, y);
    if (x < SCREEN_WIDTH + sprite->w)
        KolizjaF(kolizja, sprite->w, sprite->h, x - sprite->w, y - sprite->h, wartosc_kolizja);
}

void DrawTurtle(SDL_Surface* screen, SDL_Surface* zolw, SDL_Surface* zolw1, SDL_Surface* zolw2, SDL_Surface* zolw3, double x, double y, int** kolizja, int wartosc_kolizja, double fpsTimer, int i) {
    if (i == 0 || i == 2) {
        DrawCar(screen, zolw, x, y, kolizja, wartosc_kolizja);

    }
    else if (fpsTimer < 0.25)
        DrawCar(screen, zolw1, x, y, kolizja, 3);
    else if (fpsTimer < 0.5 || fpsTimer > 0.75)
        DrawCar(screen, zolw2, x, y, kolizja, 3);
    else
        DrawCar(screen, zolw3, x, y, kolizja, 3);
}

void FrogReset(double* worldTime, double* frogger_xy, int* frogger_tryb) {
    *frogger_tryb = 0;
    *worldTime = 0;
    frogger_xy[2] = frogger_xy[0] = frogger_xy[4];
    frogger_xy[7] = frogger_xy[3] = frogger_xy[1] = frogger_xy[5];
}

void Death(int* ktory_frogger, int* lives, double* worldTime, double* frogger_xy, int* frogger_tryb) {
    *ktory_frogger = 0;
    *lives -= 1;
    FrogReset(worldTime, frogger_xy, frogger_tryb);
}

void Win(int** kolizja, int i, int* win, double* worldTime, double* frogger_xy, int* frogger_tryb) {
    int a = TILE_SIZE / 2;
    for (int j = 0; j < TILE_SIZE; j++) {
        for (int k = 0; k < MAP_HEIGH - TILE_SIZE * 12; k++) {
            kolizja[j + a + i * 3 * TILE_SIZE][k] = 2;

        }

    }
    win[i] = 1;
    FrogReset(worldTime, frogger_xy, frogger_tryb);
}

void WinReset(int** kolizja) {
    int a = TILE_SIZE / 2;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < TILE_SIZE; j++) {
            for (int k = 0; k < MAP_HEIGH - TILE_SIZE * 12; k++) {
                kolizja[j + a + i * 3 * TILE_SIZE][k] = 3;

            }

        }

    }
}


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    int t1, t2, globalTime_int, quit, pause, frames, rc, exit, lives, win[5] = {
    }, wins, score, wartosc_kolizja, frogger_tryb;
    double delta, worldTime, globalTime, fpsTimer, fps, distance, etiSpeed, czasogolny, car_x[PORUSZAJACE_PO_X] = {
    }, car_v[PORUSZAJACE_PO_X] = { -1, 1, -3, 1.5, -2, -1.5, 0.8, 2, -1.5, 1
    }, worldSpeed, frogger_xy[8], frogger_v, frogger_zgubiony_x[4] = { 0, TILE_SIZE, 0, TILE_SIZE
    };
    SDL_Event event;
    SDL_Surface* screen, * charset;
    SDL_Surface* eti, * background, * car1, * car2, * car2_animacja[2], * car3, * car4, * car5, * bala1, * bala2, * bala3, * mucha;
    SDL_Surface* zolw, * zolw1, * zolw2, * zolw3, * zolw4, * zolw5;
    SDL_Surface* frogger, * frogger_kierunek[8], * frogger_zgubiony, * frogger_zgubiony_kierunek[8];
    SDL_Texture* scrtex;
    SDL_Window* window;
    SDL_Renderer* renderer;

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;

    }

    rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
        &window, &renderer);
    if (rc != 0) {
        SDL_Quit();
        printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
        return 1;

    };

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);



    SDL_ShowCursor(SDL_DISABLE);


    charset = SDL_LoadBMP("cs8x8.bmp");
    if (charset == NULL) {
        printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };
    SDL_SetColorKey(charset, true, 0x000000);

    frogger_kierunek[0] = SDL_LoadBMP("Graphics/frog_up1.bmp");
    if (frogger_kierunek[0] == NULL) {
        printf("SDL_LoadBMP(frog_up1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[1] = SDL_LoadBMP("Graphics/frog_up2.bmp");
    if (frogger_kierunek[1] == NULL) {
        printf("SDL_LoadBMP(frog_up2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[2] = SDL_LoadBMP("Graphics/frog_down1.bmp");
    if (frogger_kierunek[2] == NULL) {
        printf("SDL_LoadBMP(frog_down1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[3] = SDL_LoadBMP("Graphics/frog_down2.bmp");
    if (frogger_kierunek[3] == NULL) {
        printf("SDL_LoadBMP(frog_down2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[4] = SDL_LoadBMP("Graphics/frog_left1.bmp");
    if (frogger_kierunek[4] == NULL) {
        printf("SDL_LoadBMP(frog_left1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[5] = SDL_LoadBMP("Graphics/frog_left2.bmp");
    if (frogger_kierunek[5] == NULL) {
        printf("SDL_LoadBMP(frog_left2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[6] = SDL_LoadBMP("Graphics/frog_right1.bmp");
    if (frogger_kierunek[6] == NULL) {
        printf("SDL_LoadBMP(frog_right1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_kierunek[7] = SDL_LoadBMP("Graphics/frog_right2.bmp");
    if (frogger_kierunek[7] == NULL) {
        printf("SDL_LoadBMP(frog_right2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[0] = SDL_LoadBMP("Graphics/frog2_up1.bmp");
    if (frogger_zgubiony_kierunek[0] == NULL) {
        printf("SDL_LoadBMP(frog2_up1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[1] = SDL_LoadBMP("Graphics/frog2_up2.bmp");
    if (frogger_zgubiony_kierunek[1] == NULL) {
        printf("SDL_LoadBMP(frog2_up2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[2] = SDL_LoadBMP("Graphics/frog2_down1.bmp");
    if (frogger_zgubiony_kierunek[2] == NULL) {
        printf("SDL_LoadBMP(frog2_down1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[3] = SDL_LoadBMP("Graphics/frog2_down2.bmp");
    if (frogger_zgubiony_kierunek[3] == NULL) {
        printf("SDL_LoadBMP(frog2_down2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[4] = SDL_LoadBMP("Graphics/frog2_left1.bmp");
    if (frogger_zgubiony_kierunek[4] == NULL) {
        printf("SDL_LoadBMP(frog2_left1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[5] = SDL_LoadBMP("Graphics/frog2_left2.bmp");
    if (frogger_zgubiony_kierunek[5] == NULL) {
        printf("SDL_LoadBMP(frog2_left2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[6] = SDL_LoadBMP("Graphics/frog2_right1.bmp");
    if (frogger_zgubiony_kierunek[6] == NULL) {
        printf("SDL_LoadBMP(frog2_right1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    frogger_zgubiony_kierunek[7] = SDL_LoadBMP("Graphics/frog2_right2.bmp");
    if (frogger_zgubiony_kierunek[7] == NULL) {
        printf("SDL_LoadBMP(frog2_right2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    mucha = SDL_LoadBMP("Graphics/mucha.bmp");
    if (mucha == NULL) {
        printf("SDL_LoadBMP(mucha.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    car1 = SDL_LoadBMP("Graphics/car1.bmp");
    if (car1 == NULL) {
        printf("SDL_LoadBMP(car1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    car2_animacja[0] = SDL_LoadBMP("Graphics/car2_1.bmp");
    if (car2_animacja[0] == NULL) {
        printf("SDL_LoadBMP(car2_1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    car2_animacja[1] = SDL_LoadBMP("Graphics/car2_2.bmp");
    if (car2_animacja[0] == NULL) {
        printf("SDL_LoadBMP(car2_1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    car3 = SDL_LoadBMP("Graphics/car3.bmp");
    if (car3 == NULL) {
        printf("SDL_LoadBMP(car3.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    car4 = SDL_LoadBMP("Graphics/car4.bmp");
    if (car4 == NULL) {
        printf("SDL_LoadBMP(car4.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    car5 = SDL_LoadBMP("Graphics/truck.bmp");
    if (car5 == NULL) {
        printf("SDL_LoadBMP(truck.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    bala1 = SDL_LoadBMP("Graphics/bala1.bmp");
    if (bala1 == NULL) {
        printf("SDL_LoadBMP(bala1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    bala2 = SDL_LoadBMP("Graphics/bala2.bmp");
    if (bala2 == NULL) {
        printf("SDL_LoadBMP(bala2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    bala3 = SDL_LoadBMP("Graphics/bala3.bmp");
    if (bala3 == NULL) {
        printf("SDL_LoadBMP(bala3.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    zolw1 = SDL_LoadBMP("Graphics/zolw1.bmp");
    if (zolw1 == NULL) {
        printf("SDL_LoadBMP(zolw1.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    zolw2 = SDL_LoadBMP("Graphics/zolw2.bmp");
    if (zolw2 == NULL) {
        printf("SDL_LoadBMP(zolw2.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    zolw3 = SDL_LoadBMP("Graphics/zolw3.bmp");
    if (zolw3 == NULL) {
        printf("SDL_LoadBMP(zolw3.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    zolw4 = SDL_LoadBMP("Graphics/zolw4.bmp");
    if (zolw4 == NULL) {
        printf("SDL_LoadBMP(zolw4.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    zolw5 = SDL_LoadBMP("Graphics/zolw5.bmp");
    if (zolw5 == NULL) {
        printf("SDL_LoadBMP(zolw5.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    background = SDL_LoadBMP("Graphics/frogger_background.bmp");
    if (background == NULL) {
        printf("SDL_LoadBMP(frogger_background.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;

    };

    int a = background->w + 2;
    int b = background->h;
    int** kolizja = new int* [a];
    for (int i = 0; i < a; i++)
        kolizja[i] = new int[b];

    for (int i = 0; i < a; i++) {
        for (int j = 0; j < b - TILE_SIZE * 7; j++)
            kolizja[i][j] = 2;
        for (int j = b - TILE_SIZE * 7; j < b; j++)
            kolizja[i][j] = 0;

    }
    for (int i = 0; i < b; i++) {
        kolizja[0][i] = 1;
        kolizja[a - 1][i] = 1;

    }
    WinReset(kolizja);


    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
    int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    frames = 0;
    fpsTimer = 0;
    fps = 0;
    quit = 0;
    pause = 0;
    worldTime = 0;
    globalTime = 0;
    globalTime_int = 0;
    distance = 0;
    etiSpeed = 1;
    worldSpeed = 50;
    exit = 0;
    frogger = frogger_kierunek[0];
    frogger_v = 4;
    frogger_xy[4] = frogger_xy[2] = frogger_xy[0] = SCREEN_WIDTH / 2 + frogger->w / 2;
    frogger_xy[7] = frogger_xy[5] = frogger_xy[3] = frogger_xy[1] = MAP_HEIGH - (TILE_SIZE - frogger->h) / 2;
    lives = 3;
    wins = 0;
    score = 0;
    frogger_tryb = 0;

    int mucha_kolizja = 0;

    czasogolny = SDL_GetTicks();
    int losowa = 0, losowa2;

    while (!exit) {
        t1 = SDL_GetTicks();



        SDL_StartTextInput();


        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) exit = 1;
                else if (event.key.keysym.sym == SDLK_p && pause == 1) pause = 0;
                else if (event.key.keysym.sym == SDLK_n && pause == 2) pause = 0;
                else if (event.key.keysym.sym == SDLK_y && pause == 2) exit = 1;
                else if (event.key.keysym.sym == SDLK_y && lives == 0) {
                    lives = 3;
                    score = 0;
                    FrogReset(&worldTime, frogger_xy, &frogger_tryb);
                    wins = 0;
                    WinReset(kolizja);

                }
                else if (event.key.keysym.sym == SDLK_n && lives == 0) exit = 1;
                break;
            case SDL_QUIT:
                exit = 1;
                break;

            };

        };


        while (!quit && lives && !pause) {
            t2 = SDL_GetTicks();

            delta = (t2 - t1) * 0.001;
            t1 = t2;

            worldTime += delta;
            globalTime += delta;
            globalTime_int = globalTime;
            for (int i = 0; i < PORUSZAJACE_PO_X; i++) {
                car_x[i] += delta * worldSpeed * car_v[i];
                if (car_x[i] > 2 * SCREEN_WIDTH) {
                    car_x[i] = 0;

                }
                else if (car_x[i] < 0)
                    car_x[i] = 2 * SCREEN_WIDTH;

            }

            if (frogger_xy[2] > SCREEN_WIDTH || frogger_xy[2] < TILE_SIZE / 2)
                frogger_xy[2] = frogger_xy[0];
            if (frogger_xy[3] > MAP_HEIGH || frogger_xy[3] < 3 * TILE_SIZE / 2)
                frogger_xy[3] = frogger_xy[1];

            fpsTimer += delta;
            if (fpsTimer > 1) {
                fps = frames;
                frames = 0;
                fpsTimer -= 1;

            };

            if (globalTime_int % 5 < 3) {
                wartosc_kolizja = 3;
                if (fpsTimer < 0.25)
                    zolw = zolw1;
                else if (fpsTimer < 0.5 || fpsTimer > 0.75)
                    zolw = zolw2;
                else
                    zolw = zolw3;

            }
            else {
                wartosc_kolizja = 2;
                if (fpsTimer < 0.33 || fpsTimer > 0.66)
                    zolw = zolw4;
                else
                    zolw = zolw5;

            }


            if (fpsTimer < 0.25 || (fpsTimer > 0.5 && fpsTimer < 0.75))
                car2 = car2_animacja[0];
            else
                car2 = car2_animacja[1];



            SDL_FillRect(screen, NULL, czarny);

            DrawSurface(screen, background, SCREEN_WIDTH, 504);


            DrawCar(screen, car1, car_x[0], MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 2, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 6, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 8, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 12, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 14, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 18, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 20, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            DrawCar(screen, car1, car_x[0] + TILE_SIZE * 24, MAP_HEIGH - TILE_SIZE - (TILE_SIZE - car1->h) / 2, kolizja, 1);
            for (int i = 0; i < 25; i += 4) {
                DrawCar(screen, car2, car_x[1] + TILE_SIZE * i, MAP_HEIGH - TILE_SIZE * 2 - (TILE_SIZE - car2->h) / 2, kolizja, 1);

            }
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++) {
                    DrawCar(screen, car3, car_x[2] + TILE_SIZE * 3 * j + TILE_SIZE * 14 * i, MAP_HEIGH - TILE_SIZE * 3 - (TILE_SIZE - car3->h) / 2, kolizja, 1);

                }

            }
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    DrawCar(screen, car4, car_x[3] + TILE_SIZE * 2 * j + TILE_SIZE * 13 * i, MAP_HEIGH - TILE_SIZE * 4 - (TILE_SIZE - car4->h) / 2, kolizja, 1);

                }

            }
            for (int i = 0; i < 3; i++) {
                DrawCar(screen, car5, car_x[4] + TILE_SIZE * 9 * i, MAP_HEIGH - TILE_SIZE * 5 - (TILE_SIZE - car5->h) / 2, kolizja, 1);

            }

            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 3; j++) {
                    DrawTurtle(screen, zolw, zolw1, zolw2, zolw3, car_x[5] + j * TILE_SIZE + i * 5 * TILE_SIZE, MAP_HEIGH - TILE_SIZE * 7 - (TILE_SIZE - zolw->h) / 2, kolizja, wartosc_kolizja, fpsTimer, i);

                }

            }

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 3; j++)
                    DrawCar(screen, bala3, car_x[6] + j * 3.5 * TILE_SIZE + i * 13 * TILE_SIZE, MAP_HEIGH - TILE_SIZE * 8 - (TILE_SIZE - bala3->h) / 2, kolizja, 3);

            }
            if (frogger_tryb == 0) {
                if (frogger_zgubiony_x[0] - frogger_zgubiony_x[1] > 0.5 || frogger_zgubiony_x[0] - frogger_zgubiony_x[1] < -0.5) {
                    if (frogger_zgubiony_x[0] < frogger_zgubiony_x[1]) {
                        frogger_zgubiony_x[0] += delta * worldSpeed;
                        frogger_zgubiony = frogger_zgubiony_kierunek[5];

                    }
                    else {
                        frogger_zgubiony_x[0] -= delta * worldSpeed;
                        frogger_zgubiony = frogger_zgubiony_kierunek[7];

                    }

                }
                else {
                    frogger_zgubiony_x[0] = frogger_zgubiony_x[1];
                    if (frogger_zgubiony_x[1] == frogger_zgubiony_x[2])
                        frogger_zgubiony_x[1] = frogger_zgubiony_x[3];
                    else
                        frogger_zgubiony_x[1] = frogger_zgubiony_x[2];

                }

                DrawCar(screen, frogger_zgubiony, car_x[6] - frogger_zgubiony_x[0], MAP_HEIGH - TILE_SIZE * 8 - (TILE_SIZE - bala3->h) / 2, kolizja, 6);

            }


            for (int i = 0; i < 3; i++)
                DrawCar(screen, bala1, car_x[7] + i * 8 * TILE_SIZE, MAP_HEIGH - TILE_SIZE * 9 - (TILE_SIZE - bala1->h) / 2, kolizja, 3);

            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 2; j++) {
                    DrawTurtle(screen, zolw, zolw1, zolw2, zolw3, car_x[8] + j * TILE_SIZE + i * 5 * TILE_SIZE + 1.5 * TILE_SIZE, MAP_HEIGH - TILE_SIZE * 10 - (TILE_SIZE - zolw->h) / 2, kolizja, wartosc_kolizja, fpsTimer, i);

                }

            }

            for (int i = 0; i < 5; i++)
                DrawCar(screen, bala2, car_x[9] + i * 4 * TILE_SIZE, MAP_HEIGH - TILE_SIZE * 11 - (TILE_SIZE - bala2->h) / 2, kolizja, 3);

            int* ktory_mucha = &losowa2;
            if (globalTime_int % 30 < 10) {
                int b = TILE_SIZE / 2;
                if (mucha_kolizja == 0) {
                    for (int i = 0, j = 1; i < j; i++) {
                        *ktory_mucha = rand() % 5;
                        if (win[*ktory_mucha] == 1)
                            j++;

                    }

                    for (int j = 0; j < TILE_SIZE; j++) {
                        for (int k = 0; k < MAP_HEIGH - TILE_SIZE * 12; k++) {
                            kolizja[j + b + *ktory_mucha * 3 * TILE_SIZE][k] = 4;

                        }

                    }
                    mucha_kolizja = 1;

                }
                if (kolizja[b + *ktory_mucha * 3 * TILE_SIZE + 10][10] == 4) {
                    DrawSurface(screen, mucha, TILE_SIZE * 1.5 - (TILE_SIZE - mucha->w) / 2 + *ktory_mucha * 3 * TILE_SIZE, TILE_SIZE * 1.5);

                }

            }
            else if (mucha_kolizja == 1) {
                int b = TILE_SIZE / 2;
                if (kolizja[b + *ktory_mucha * 3 * TILE_SIZE + 10][10] == 4) {
                    for (int j = 0; j < TILE_SIZE; j++) {
                        for (int k = 0; k < MAP_HEIGH - TILE_SIZE * 12; k++) {
                            kolizja[j + b + *ktory_mucha * 3 * TILE_SIZE][k] = 3;

                        }

                    }

                }
                mucha_kolizja = 0;

            }


            for (int i = 0; i < 5; i++)
                if (win[i] == 1)
                    DrawSurface(screen, frogger_kierunek[2], TILE_SIZE * 1.5 - (TILE_SIZE - frogger_kierunek[2]->w) / 2 + i * 3 * TILE_SIZE, TILE_SIZE * 1.5);

            int* ktory_frogger = &losowa;

            if ((MAX_TIME - worldTime) > 0) {
                sprintf(text, "Time");
                DrawString(screen, SCREEN_WIDTH - 40, SCREEN_HEIGHT - (TILE_SIZE + 3) / 2 - 8, text, charset);
                if (worldTime < 40) {
                    DrawRectangle(screen, SCREEN_WIDTH - 3 * (MAX_TIME - worldTime) - 45, SCREEN_HEIGHT - TILE_SIZE - 3, 3 * (MAX_TIME - worldTime), TILE_SIZE, zielony, zielony);

                }
                else {
                    DrawRectangle(screen, SCREEN_WIDTH - 3 * (MAX_TIME - worldTime) - 45, SCREEN_HEIGHT - TILE_SIZE - 3, 3 * (MAX_TIME - worldTime), TILE_SIZE, czerwony, czerwony);

                }

            }
            else {
                Death(ktory_frogger, &lives, &worldTime, frogger_xy, &frogger_tryb);

            }

            sprintf(text, "Lives: %d", lives);
            DrawString(screen, 10, SCREEN_HEIGHT - (TILE_SIZE + 3) / 2 - 13, text, charset);

            sprintf(text, "Score: %d", score);
            DrawString(screen, 10, SCREEN_HEIGHT - (TILE_SIZE + 3) / 2, text, charset);


            int f_x = round(frogger_xy[0]);
            int f_y = round(frogger_xy[1]);

            if (kolizja[f_x][f_y] == 1 || kolizja[f_x - frogger->w][f_y] == 1 || kolizja[f_x][f_y - frogger->h / 2] == 1 || kolizja[f_x - frogger->w][f_y - frogger->h / 2] == 1 || kolizja[f_x][f_y - frogger->h] == 1 || kolizja[f_x - frogger->w][f_y - frogger->h] == 1) {
                Death(ktory_frogger, &lives, &worldTime, frogger_xy, &frogger_tryb);

            }
            else {
                if (f_y - frogger->h > MAP_HEIGH - TILE_SIZE * 7) {
                    DrawSurface(screen, frogger, frogger_xy[0], frogger_xy[1]);

                }
                else if (f_y - frogger->h > MAP_HEIGH - TILE_SIZE * 12) {
                    if (kolizja[f_x][f_y] != 2 || kolizja[f_x - frogger->w][f_y] != 2 || kolizja[f_x][f_y - frogger->h / 2] != 2 || kolizja[f_x - frogger->w][f_y - frogger->h / 2] != 2 || kolizja[f_x][f_y - frogger->h] != 2 || kolizja[f_x - frogger->w][f_y - frogger->h] != 2) {
                        if (kolizja[f_x][f_y] == 6 || kolizja[f_x - frogger->w][f_y] == 6 || kolizja[f_x][f_y - frogger->h / 2] == 6 || kolizja[f_x - frogger->w][f_y - frogger->h / 2] == 6 || kolizja[f_x][f_y - frogger->h] == 6 || kolizja[f_x - frogger->w][f_y - frogger->h] == 6)
                            frogger_tryb = 1;

                        if (frogger_xy[3] == frogger_xy[1]) {
                            int gdzie = PORUSZAJACE_PO_X + 1 - f_y / TILE_SIZE;
                            frogger_xy[0] += delta * worldSpeed * car_v[gdzie];
                            frogger_xy[2] += delta * worldSpeed * car_v[gdzie];

                        }
                        DrawSurface(screen, frogger, frogger_xy[0], frogger_xy[1]);

                    }
                    else {
                        Death(ktory_frogger, &lives, &worldTime, frogger_xy, &frogger_tryb);

                    }

                }
                else {
                    int y = frogger_xy[3];
                    if (kolizja[f_x - frogger->w / 2][y] == 3 || kolizja[f_x - frogger->w / 2][y] == 4 || kolizja[f_x][f_y] == 3 || kolizja[f_x - frogger->w][f_y] == 3) {
                        DrawSurface(screen, frogger, frogger_xy[0], frogger_xy[1]);
                        if (frogger_xy[1] == frogger_xy[3] && frogger_xy[0] == frogger_xy[2]) {
                            wins += 1;
                            score += floor(MAX_TIME - worldTime) * 10 + 50;
                            for (int i = 0; i < 5; i++) {
                                if (frogger_xy[0] > (TILE_SIZE / 2 + i * 3 * TILE_SIZE) && frogger_xy[0] - frogger->w < (TILE_SIZE * 1.5 + i * 3 * TILE_SIZE)) {
                                    if (kolizja[f_x - frogger->w / 2][y] == 4 && frogger_tryb == 1) {
                                        score += 400;

                                    }
                                    else if (kolizja[f_x - frogger->w / 2][y] == 4 || frogger_tryb == 1) {
                                        score += 200;

                                    }
                                    Win(kolizja, i, win, &worldTime, frogger_xy, &frogger_tryb);
                                    break;

                                }

                            }

                            if (wins % 5 == 0) {
                                WinReset(kolizja);
                                for (int i = 0; i < 5; i++)
                                    win[i] = 0;

                            }

                        }

                    }
                    else {
                        Death(ktory_frogger, &lives, &worldTime, frogger_xy, &frogger_tryb);

                    }

                }

            }


            if (frogger_xy[0] != frogger_xy[2]) {
                if (frogger_xy[0] - frogger_xy[2] > 0.5 || frogger_xy[0] - frogger_xy[2] < -0.5) {
                    if (frogger_xy[2] > frogger_xy[0]) {
                        frogger_xy[0] += delta * worldSpeed * frogger_v;
                        *ktory_frogger = 7;

                    }
                    else {
                        frogger_xy[0] -= delta * worldSpeed * frogger_v;
                        *ktory_frogger = 5;

                    }

                }
                else {
                    frogger_xy[0] = frogger_xy[2];
                    if (*ktory_frogger % 2 == 1)
                        *ktory_frogger -= 1;

                }

            }
            if (frogger_xy[1] != frogger_xy[3]) {
                if (frogger_xy[1] - frogger_xy[3] > 0.5 || frogger_xy[1] - frogger_xy[3] < -0.5) {
                    if (frogger_xy[3] > frogger_xy[1]) {
                        frogger_xy[1] += delta * worldSpeed * frogger_v;
                        *ktory_frogger = 3;

                    }
                    else {
                        frogger_xy[1] -= delta * worldSpeed * frogger_v;
                        *ktory_frogger = 1;

                    }

                }
                else {
                    frogger_xy[1] = frogger_xy[3];
                    if (*ktory_frogger % 2 == 1)
                        *ktory_frogger -= 1;

                }

            }

            if (frogger_tryb == 0)
                frogger = frogger_kierunek[*ktory_frogger];
            else
                frogger = frogger_zgubiony_kierunek[*ktory_frogger];


            if (frogger_xy[1] == frogger_xy[3] && frogger_xy[3] < frogger_xy[7]) {
                score += 10;
                frogger_xy[7] = frogger_xy[3];

            }



            SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
            SDL_RenderCopy(renderer, scrtex, NULL, NULL);
            SDL_RenderPresent(renderer);


            while (SDL_PollEvent(&event)) {
                if (frogger_xy[2] == frogger_xy[0] && frogger_xy[3] == frogger_xy[1])
                    switch (event.type) {
                    case SDL_KEYDOWN:
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            quit = 1;
                            exit = 1;

                        }
                        else if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) frogger_xy[3] -= TILE_SIZE;
                        else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) frogger_xy[3] += TILE_SIZE;
                        else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) frogger_xy[2] += TILE_SIZE;
                        else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) frogger_xy[2] -= TILE_SIZE;
                        else if (event.key.keysym.sym == SDLK_p) pause = 1;
                        else if (event.key.keysym.sym == SDLK_q) pause = 2;
                        break;
                    case SDL_KEYUP:
                        break;
                    case SDL_QUIT:
                        quit = 1;
                        exit = 1;
                        break;

                    };

            };
            frames++;

        }

        if (pause) {
            DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 46, czerwony, niebieski);
            if (pause == 1) {
                sprintf(text, "PAUSED");
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 23, text, charset);

            }
            else {
                sprintf(text, "OUIT GAME?");
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 15, text, charset);
                sprintf(text, "Y / N");
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 31, text, charset);

            }

        }
        if (!lives) {
            DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 61, czerwony, niebieski);
            sprintf(text, "GAME OVER");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 15, text, charset);
            sprintf(text, "Do you wish to continue");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 31, text, charset);
            sprintf(text, "Y / N");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 47, text, charset);

        }

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

    };

    SDL_StopTextInput();

    SDL_FreeSurface(charset);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    delete[] kolizja;

    SDL_Quit();
    return 0;
};