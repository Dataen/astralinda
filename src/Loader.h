
#pragma once

#include <optional>
#include <vector>
#include <Level.h>

namespace Loader
{
    std::optional<std::vector<Level>>
    load_levels(const std::string& file);
}