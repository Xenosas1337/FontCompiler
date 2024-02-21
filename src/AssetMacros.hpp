#pragma once

#include <cstdint>
#include <string>
#include <filesystem>

namespace dash_tools
{
  //Directory
  constexpr std::string_view ASSET_ROOT {"Fonts"};

  // ASSET EXTENSIONS
  constexpr std::string_view FONT_EXTENSION{ ".dash_font" };

  // EXTERNAL EXTENSIONS
  constexpr std::string_view TTF_EXTENSION{ ".ttf" };

  constexpr std::string_view EXTERNALS[] = {
    TTF_EXTENSION
  };
}


