#include <cstdio>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

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

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    SDL_GL_SetSwapInterval(0);

    while (true) {
        glClear(GL_COLOR_BUFFER_BIT);   
        SDL_GL_SwapWindow(window);
    }

    return 0;
}
