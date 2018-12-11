#include <cstdio>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "graphics/SpriteBatcher.h"

// Rendering is roughly split into three steps (each of which has potentially many sub-steps):
//     Drawing    - where we create objects with properties like size, position, colour, etc. in RAM.
//     Rendering  - where we send what we previously drew to the GPU and pass it through shaders to create a framebuffer.
//     Displaying - where we send the framebuffer we previously created and pass it to the window, to be displayed by the monitor.
// Note: The names I gave are only useful now for getting the process straight, often rendering and drawing refer to (the same) processes
// performed on the GPU when used by people familiar with graphics.
//     Note^2: All functions in our code will use the above naming conventions, for clarity.

int main() {
    // Prepares SDL library, which handles windows and user input.
    SDL_Init(SDL_INIT_EVERYTHING);

    // A couple of handles we need for rendering and modifying our window.
    SDL_Window* window;
    SDL_GLContext context;

    // Create the window - notifying SDL that we are using OpenGL and want the window to be resizable in addition to the window name, where to put the window on our screel initiailly and the resolution of the window.
    window = SDL_CreateWindow("SECRET_PROJECT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        return -1;
    }

    // Create an OpenGL context associated with the window - we use this whenever we're done rendering and want SDL to put what we've rendered on our screen.
    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        return -2;
    }

    // Initialise wrapper of OpenGL - this provides all the functions we call in the OpenGL library.
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        return -3;
    }

    printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    // Enable depth testing, set the clear colour and depth.
    //   This means whenever we call glClear, the colour buffer will be entirely reset to this colour - i.e. if we then rendered nothing new, the window would turn entirely this colour.
    //   It also means when we call glClear, the depth buffer will be cleared up to a depth of 1.0 (which is also the deepest an object can be - i.e. how far "behind" the screen it can be).
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.7f, 0.3f, 1.0f);
    glClearDepth(1.0);

    // Set SDL to use double buffering, this means the GPU has two framebuffers - which are the arrays of pixel colours (i.e. what get sent to our physical monitor to be drawn).
    //   By using two framebuffers, we can simultaneously have one being drawn to on the GPU and one being sent to the monitor to be displayed.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // This enables "VSync" which is to say that each time a framebuffer is sent to the monitor, it will only begin being displayed as the top pixels begin to update.
    //   This prevents screen "tearing" where the monitor begins displaying a framebuffer as it is updating pixels in the middle of the screen, and by the time it gets
    //   round to updating the top pixels it has been given a new framebuffer to draw. It does, however, lock the framerate at the refresh rate of the monitor (60Hz for
    //   most modern LCDs) or a lower multiple thereof (e.g. 30 frames per second, or even 15 in the case of a 60Hz monitor).
    SDL_GL_SetSwapInterval(1);

    // Create a test sprite batcher, initialise it and reserve space for 10 sprites.
    spg::SpriteBatcher sb;
    sb.init();
    sb.reserve(10);

    // Begin the drawing mode of the sprite batcher, draw 10 sprites, then end the draw mode - at which point the sprites are sorted and turned into batches for rendering.
    sb.begin();
    for (size_t i = 0; i < 10; ++i) {
        sb.draw(0, f32v2(40.0f * static_cast<f32>(i), 40.0f * static_cast<f32>(i)), f32v2(40.0f, 40.0f), { static_cast<ui8>(20 * i), 50, 128, 255 });
    }
    sb.end();

    while (true) {
        // Clear whatever we last rendered.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

        // Render the sprites we drew earlier.
        sb.render(f32v2(1200.0f, 800.0f));

        // Swap the framebuffers so the one we just rendered to is now to be displayed on the monitor.
        SDL_GL_SwapWindow(window);
    }

    return 0;
}
