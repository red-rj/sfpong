#pragma once
// common stuff

#include "spdlog/spdlog.h"

namespace red
{
    inline auto LOGGER_NAME = "sfPong";
    inline auto gamelog() { return spdlog::get(LOGGER_NAME); }
}
