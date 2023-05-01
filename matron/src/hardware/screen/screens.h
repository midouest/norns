#pragma once

#include "hardware/io.h"

#ifndef EMSCRIPTEN
extern screen_ops_t screen_fbdev_ops;
#ifdef NORNS_DESKTOP
extern screen_ops_t screen_sdl_ops;
#endif
#endif
extern screen_ops_t screen_static_ops;
