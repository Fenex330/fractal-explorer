/*
g++ -std=c++17 -pedantic-errors -O3 -lsfml-graphics -lsfml-window -lsfml-system fractal-explorer-gpu.cpp -o fractal-explorer-gpu

Deps: sfml

current max. precision = 15 zoom iterations (2^15 magnification)



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

#include <cstdio>
#include <SFML/Graphics.hpp>

const int SCREEN_WIDTH = 800;
const int ZOOM_FACTOR = 2;

int main()
{
    float zoom = 1;
    float zoomX = 0;
    float zoomY = 0;
    int zoom_iter = 0;

    sf::RenderWindow window (sf::VideoMode(SCREEN_WIDTH, SCREEN_WIDTH), "Fractal Explorer");
    sf::RectangleShape square (sf::Vector2f((float)SCREEN_WIDTH, (float)SCREEN_WIDTH));

    if (!sf::Shader::isAvailable())
    {
        printf("ERROR: shaders not supported");
        return 1;
    }

    sf::Shader shader;

    if (!shader.loadFromFile("shader.frag", sf::Shader::Fragment))
    {
        printf("ERROR: shader file not found");
        return 1;
    }

    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || event.type == sf::Event::KeyPressed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                shader.setUniform("SCREEN_WIDTH", (float)SCREEN_WIDTH);
                shader.setUniform("zoom", zoom);
                shader.setUniform("zoomX", zoomX);
                shader.setUniform("zoomY", zoomY);

                window.clear();
                window.draw(square, &shader);
                window.display();

                printf("zoom magnitude: %i^%i\n", ZOOM_FACTOR, zoom_iter);
                printf("coordinates: (%f, %f)\n\n", zoomX, zoomY);

                zoom = zoom / ZOOM_FACTOR;
                zoom_iter++;

                sf::Vector2i localPosition = sf::Mouse::getPosition(window);

                zoomX = zoom * ((float)localPosition.x / (float)SCREEN_WIDTH * 4.0 - 2.0) + zoomX;
                zoomY = zoom * ((float)localPosition.y / (float)SCREEN_WIDTH * 4.0 - 2.0) + zoomY;
            }
        }
    }

    return 0;
}
