#pragma once

#include "esp_err.h"
#include <cstdint>

namespace st3215
{
    esp_err_t initialize();
    esp_err_t ping(std::uint8_t id);
} // namespace st3215
