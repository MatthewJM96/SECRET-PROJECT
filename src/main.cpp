#include <cstdio>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "graphics/SpriteBatcher.h"

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window;
    SDL_GLContext context;

    window = SDL_CreateWindow("SECRET_PROJECT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        return -1;
    }

    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        return -2;
    }

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        return -3;
    }

    printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClearDepth(1.0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(0);

    spg::SpriteBatcher sb;
    sb.init();
    sb.reserve(10);

    sb.begin();
    for (size_t i = 0; i < 10; ++i) {
        sb.draw(0, f32v2(40.0f * static_cast<f32>(i), 40.0f * static_cast<f32>(i)), f32v2(40.0f, 40.0f), { static_cast<ui8>(20 * i), 50, 128, 255 });
    }
    sb.end();

    while (true) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

        sb.render(f32v2(1200.0f, 800.0f));

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
