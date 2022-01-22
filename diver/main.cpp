

#define BACKWARD_SUPERMARKET
#include "../vendor/backward.hpp"
//
// #define SUPER_ENGINE_PROFILING_DISABLED

#include "../vendor/supermarket-engine/engine/app.h"
#include "../vendor/supermarket-engine/engine/fps_layer.h"
#include "../vendor/supermarket-engine/engine/keycodes.h"
#include "../vendor/supermarket-engine/engine/terminal_layer.h"
#include "diver_layer.h"

constexpr bool IS_DEBUG = true;

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // TODO until i fix the formatting,
    // we are only using this for SIGINT
    // lldb printer is so so much nicer id rather take the extra step
    // to open and run it
    backward::SignalHandling sh;

    App::create({
        .width = 1920,
        .height = 1080,
        .title = "Diver",
        .clearEnabled = true,
        .escClosesWindow = true,
    });

    // NOTE: Cant live in tests because app inits keyboard...
    M_ASSERT(Key::getMapping("Up") == Key::KeyCode::W, "up should be W");
    M_ASSERT(Key::getMapping("Down") == Key::KeyCode::S, "down should be s");
    M_ASSERT(Key::getMapping("Left") == Key::KeyCode::A, "left should be a");
    M_ASSERT(Key::getMapping("Right") == Key::KeyCode::D, "right should be d")

    Layer* terminal = new TerminalLayer();
    App::get().pushLayer(terminal);

    Layer* fps = new FPSLayer();
    App::get().pushLayer(fps);

    Layer* diver = new DiverLayer();
    App::get().pushLayer(diver);

    App::get().run();

    return 0;
}
