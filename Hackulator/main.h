#pragma once

#include "core.h"

// main parameters base
#define MAX_LEN_MAINPARAMS 256
#define PREFIX_MAINPARAM "-"
#define MAINPARAMHELP_HEADER "******************************"

// main parameters definition
#define MAINPARAM_STARTFULLSCREEN PREFIX_MAINPARAM "f"
#define MAINPARAM_STARTFULLSCREEN_DEFAULT_VAL 0
#define MAINPARAM_STARTFULLSCREEN_DESC "Whether to start in fullscreen mode (optional, default " STRINGIFY_VALUE(MAINPARAM_STARTFULLSCREEN_DEFAULT_VAL) ")"