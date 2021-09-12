/*
g++ -std=c++17 -pedantic-errors -O3 -pthread -lSDL2 fractal-explorer-cpu.cpp -o fractal-explorer-cpu

Deps: sdl2

current max. precision = 60 zoom iterations (2^60 magnification)



MIT License

Copyright (c) 2021 Ilya Ballet

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <complex>
#include <cmath>
#include <cstdio>
#include <thread>
#include <vector>
#include "SDL2/SDL.h"

#define FRACTAL 1

constexpr long double PI = std::acos(-1);
const auto THREAD_COUNT = std::thread::hardware_concurrency();
const int RENDER_DELAY = 1000;
const int ESCAPE_LIMIT = 2;
const int ZOOM_FACTOR = 2;
const int SCREEN_WIDTH = 800;

int iter_max = 1000;
int matrix [SCREEN_WIDTH] [SCREEN_WIDTH];
std::vector<std::thread> thread_pool;

void computePoint(const long double& zoom, const long double& zoom_x, const long double& zoom_y, int x, int y);
void renderParallel(const long double& zoom, const long double& zoom_x, const long double& zoom_y, int thread_offset);
void launchThreads(const long double& zoom, const long double& zoom_x, const long double& zoom_y);
void joinThreads();
void drawPixel(SDL_Renderer* renderer, int result, int x, int y);
void displayImage(SDL_Renderer* renderer);

void computePoint(const long double& zoom, const long double& zoom_x, const long double& zoom_y, int x, int y)
{
    int iter = 0;

    long double c_x = zoom * ((long double)x / (long double)SCREEN_WIDTH * 2.0 - 1.5) + zoom_x;
    long double c_y = zoom * ((long double)y / (long double)SCREEN_WIDTH * 2.0 - 1.0) + zoom_y;

    std::complex<long double> c (c_x , c_y);
    std::complex<long double> z (0, 0);

    while (iter < iter_max && std::abs(z) < ESCAPE_LIMIT * ESCAPE_LIMIT)
    {
        #if FRACTAL == 1 // Mandelbrot Set
            z = z * z + c;
        #elif FRACTAL == 2 // Burning Ship
            std::complex<long double> z_abs (std::fabs(z.real()), std::fabs(z.imag()));
            z = z_abs * z_abs + c;
        #elif FRACTAL == 3 // Collatz Conjecture
            z = ((z * 7.0 + 2.0) - std::cos(z * PI) * (z * 5.0 + 2.0)) * 0.25;
        #elif FRACTAL == 4 // discovered myself
            z = std::exp(std::cosh(z)) + c;
        #endif

        iter++;
    }
    
    matrix [y] [x] = std::abs(z) < ESCAPE_LIMIT * ESCAPE_LIMIT ? -1 : iter;
}

void renderParallel(const long double& zoom, const long double& zoom_x, const long double& zoom_y, int thread_offset)
{
    for (int y = thread_offset; y < SCREEN_WIDTH; y += THREAD_COUNT)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            computePoint(zoom, zoom_x, zoom_y, x, y);
        }
    }
}

void launchThreads(const long double& zoom, const long double& zoom_x, const long double& zoom_y)
{
    thread_pool.clear();

    for (int j = 0; j < THREAD_COUNT; j++)
    {
        thread_pool.push_back(std::thread(renderParallel, zoom, zoom_x, zoom_y, j));
    }
}

void joinThreads()
{
    for (int j = 0; j < THREAD_COUNT; j++)
    {
        thread_pool.at(j).join();
    }
}

void drawPixel(SDL_Renderer* renderer, int result, int x, int y)
{
    if (result < 0)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // shade black
        SDL_RenderDrawPoint(renderer, x, y);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, result % 255, 0, 255 - (result % 255), 255); // shade with iter color
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void displayImage(SDL_Renderer* renderer)
{
    for (int y = 0; y < SCREEN_WIDTH; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            drawPixel(renderer, matrix [y] [x], x, y);
        }
    }
}

int main(int argc, char* args[])
{
    int i = 0;
    int zoom_iter = 0;
    int mouse_x, mouse_y;
    bool quit = false;

    long double zoom = 1;
    long double zoom_x = 0;
    long double zoom_y = 0;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_WIDTH, 0, &window, &renderer);

    if (window == nullptr && renderer == nullptr)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    printf("Using %i threads for fractal computation\n\n\n\n", THREAD_COUNT);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    launchThreads(zoom, zoom_x, zoom_y);
    joinThreads();
    displayImage(renderer);

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_KEYDOWN:
                    quit = true;
                    break;

                case SDL_MOUSEBUTTONUP:
                    zoom = zoom / ZOOM_FACTOR;
                    zoom_iter++;
                    //iter_max++;

                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);

                    zoom_x = zoom * ((long double)mouse_x / (long double)SCREEN_WIDTH * 4.0 - 2.0) + zoom_x;
                    zoom_y = zoom * ((long double)mouse_y / (long double)SCREEN_WIDTH * 4.0 - 2.0) + zoom_y;

                    printf("zoom magnitude: %i^%i\n", ZOOM_FACTOR, zoom_iter);
                    printf("coordinates: (%Lf, %Lf)\n\n", zoom_x, zoom_y);

                    launchThreads(zoom, zoom_x, zoom_y);
                    joinThreads();
                    displayImage(renderer);

                    break;

                default:
                    break;
            }
        }

        i++;

        if (i == RENDER_DELAY)
        {
            SDL_RenderPresent(renderer);
            i = 0;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
