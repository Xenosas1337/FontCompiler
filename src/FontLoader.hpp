#pragma once

#include "Font.hpp"

namespace dash_tools
{
  class FontLoader
  {

  public:
    static Font* ReadAndUnpackFileData (AssetPath path) noexcept;

  private:
    static Font* unpackFontBinary (std::vector<uint8_t> const& binaryData) noexcept;
  };
}