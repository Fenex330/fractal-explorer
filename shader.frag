#version 330 core

precision highp float;

uniform float SCREEN_WIDTH;
uniform float zoom;
uniform float zoomX;
uniform float zoomY;

void main()
{
    int iter = 0;
    int iter_max = 1000;

    float x0 = zoom * (gl_FragCoord.x / SCREEN_WIDTH * 2.0 - 1.5) + zoomX;
    float y0 = zoom * (gl_FragCoord.y / SCREEN_WIDTH * 2.0 - 1.0) + zoomY;

    float x = 0.0;
    float y = 0.0;

    while (iter < iter_max && x * x + y * y <= 4.0)
    {
        float xtemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtemp;

        iter++;
    }

    if (x * x + y * y <= 4.0)
    {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // shade black
    }
    else
    {
        gl_FragColor = vec4((iter % 255) / 255.0, 0.0, (255 - (iter % 255)) / 255.0, 1.0); // shade with iter color
    }
}
