#pragma once

#include <thread>

#include "core.h"

// other
#define MAIN_SAVEFILE_NAME "settings.txt"

// main parameters base
#define MAIN_MAX_LEN_PARAMS 16
#define MAIN_PREFIX_PARAM "-"
#define MAIN_PARAMHELP_HEADER "******************************"

// main parameters definition
#define MAIN_PARAM_STARTMAXIMIZED MAIN_PREFIX_PARAM "m"
#define MAIN_PARAM_STARTMAXIMIZED_DEFAULT_VAL 0
#define MAIN_PARAM_STARTMAXIMIZED_DESC "Whether to start maximized (optional, default: " STRINGIFY_VALUE(MAIN_PARAM_STARTMAXIMIZED_DEFAULT_VAL) ")"

#define MAIN_PARAM_SAVEFILEPATH MAIN_PREFIX_PARAM "s"
#define MAIN_PARAM_SAVEFILEPATH_DESC "Specify the save file path, please include save file name and extension (optional, default: program location --> '" MAIN_SAVEFILE_NAME "')"