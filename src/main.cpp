#include <cstdio>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL_ttf/SDL_ttf.h>

#include "graphics/SpriteBatcher.h"

// Rendering is roughly split into three steps (each of which has potentially many sub-steps):
//     Drawing    - where we construct objects with properties like size, position, colour, etc. in RAM likely passing to a
//                  buffer in the GPU's memory.
//     Rendering  - where we send what we previously drew to the GPU if we haven't already, and then pass it through shaders
//                  to create a framebuffer (an array of all the colours for every pixel to be displayed on our monitors).
//     Displaying - where we send the framebuffer we previously created and send it to be displayed by the monitor.
// All of our graphics functions should use the above naming conventions for clarity.

int main(int, char*[]) {
    // Prepares SDL library, which handles windows and user input.
    SDL_Init(SDL_INIT_EVERYTHING);

    // Initialise library for loading, manipulating, and drawing fonts.
    if (TTF_Init() < 0) {
        return -1;
    }

    // A couple of handles we need for rendering and modifying our window.
    SDL_Window* window;
    SDL_GLContext context;

    // Create the window - notifying SDL that we are using OpenGL and want the window to be resizable in addition to the window name, where to put the window on our screel initiailly and the resolution of the window.
    window = SDL_CreateWindow("SECRET_PROJECT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        return -2;
    }

    // Create an OpenGL context associated with the window - we use this whenever we're done rendering and want SDL to put what we've rendered on our screen.
    context = SDL_GL_CreateContext(window);
    if (context == nullptr) {
        return -3;
    }

    // Initialise wrapper of OpenGL - this provides all the functions we call in the OpenGL library.
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        return -4;
    }

    printf("*** OpenGL Version:  %s ***\n", glGetString(GL_VERSION));
    printf("*** OpenGL Renderer: %s ***\n", glGetString(GL_RENDERER));

    // Enable depth testing, set the clear colour and depth.
    //   This means whenever we call glClear, the colour buffer will be entirely reset to this colour - i.e. if we then rendered nothing new, the window would turn entirely this colour.
    //   It also means when we call glClear, the depth buffer will be cleared up to a depth of 1.0 (which is also the deepest an object can be - i.e. how far "behind" the screen it can be).
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.7f, 0.3f, 1.0f);
    glClearDepth(1.0);

    // // Enable blending.
    //     Blending is how OpenGL handles transparency in textures.
    glEnable(GL_BLEND);
    // Tell OpenGL to blend the colour currently stored for that pixel in the framebuffer with a new transparent texture by
    // multiplying each (colour & alpha) channel of the texture by the (normalised) value in its alpha channel and multiplying
    // the each channel of the colour currently stored for that pixel by 1 - that alpha value.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    // Set SDL to use double buffering, this means the GPU has two framebuffers - which are the arrays of pixel colours (i.e. what get sent to our physical monitor to be drawn).
    //   By using two framebuffers, we can simultaneously have one being drawn to on the GPU and one being sent to the monitor to be displayed.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // This enables "VSync" which is to say that each time a framebuffer is sent to the monitor, it will only begin being displayed as the top pixels begin to update.
    //   This prevents screen "tearing" where the monitor begins displaying a framebuffer as it is updating pixels in the middle of the screen, and by the time it gets
    //   round to updating the top pixels it has been given a new framebuffer to draw. It does, however, lock the framerate at the refresh rate of the monitor (60Hz for
    //   most modern LCDs) or a lower multiple thereof (e.g. 30 frames per second, or even 15 in the case of a 60Hz monitor).
    SDL_GL_SetSwapInterval(1);

    // Create a font cache and load a test font.
    spg::FontCache fontCache;
    fontCache.registerFont("Orbitron", "fonts/Orbitron-Bold.ttf");

    // Save our font REAL BIG.
    fontCache.fetchFontInstance("Orbitron", 80).saveAsPng("debug/orbitron.png");

    // Create a test sprite batcher, initialise it and reserve space for 10 sprites.
    spg::SpriteBatcher sb;
    sb.init(&fontCache);
    sb.reserve(10);

    // Begin the drawing mode of the sprite batcher, draw 10 sprites, then end the draw mode - at which point the sprites are sorted and turned into batches for rendering.
    sb.begin();

    // sb.drawString("Hello, World!", f32v4(40.0f, 40.0f, 1120.0f, 720.0f), { spg::StringSizingKind::SCALED, { f32v2(1.0f, 1.0f) } }, { 0, 50, 128, 255 }, "Orbitron", 40, spg::TextAlign::TOP_LEFT);

    // for (size_t i = 0; i < 10; ++i) {
    //     sb.draw(0, f32v2(350.0f + 40.0f * static_cast<f32>(i), 140.0f + 40.0f * static_cast<f32>(i)), f32v2(40.0f, 40.0f), { static_cast<ui8>(20 * i), 50, 128, 255 });
    // }

    // sb.drawString("Bye, World!", f32v4(800.0f, 600.0f, 400.0f, 200.0f), { spg::StringSizingKind::SCALED, { f32v2(1.0f, 1.0f) } }, { 200, 50, 128, 255 }, "Orbitron", 40);

    sb.drawString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.", f32v4(40.0f, 40.0f, 1120.0f, 720.0f), { spg::StringSizingKind::SCALED, { f32v2(1.0f, 1.0f) } }, { 124, 87, 20, 255 }, "Orbitron", 24, spg::TextAlign::TOP_CENTER, spg::WordWrap::GREEDY);

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
