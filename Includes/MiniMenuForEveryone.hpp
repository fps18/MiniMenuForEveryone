#pragma once

#include "CTRPluginFramework.hpp"
#include "types.h"

namespace CTRPluginFramework
{
    using VoidNoArgFunctionPointer = void(*)();

    void Implementation_MiniMenu    (const std::string &str = "MiniMenu", u32 major = 0, u32 minor = 0, u32 revision = 0, u32 Button = Key::Select, bool Optimize = false, MenuEntry *entry = nullptr);

    void new_entry                  (const std::string &name, short type, u32 Address, u32 Value, const std::string &note = "");
    void new_entry                  (const std::string &name, short type, VoidNoArgFunctionPointer cheat, const std::string &note ="");
}