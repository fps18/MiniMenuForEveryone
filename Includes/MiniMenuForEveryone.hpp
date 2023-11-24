#pragma once

#include "CTRPluginFramework.hpp"
#include "types.h"

namespace CTRPluginFramework
{
    void Implementation_MiniMenu    (const std::string &str, u32 major, u32 minor, u32 revision, u32 Button, bool Optimize = false);

    void new_entry (const std::string &name, short type, u32 Address, u32 Value, const std::string &note = "");
}