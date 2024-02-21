#pragma once

#include "Font.hpp"

namespace dash_tools
{
  class FontLoader
  {

  public:
    Font* ReadAndUnpackFileData (AssetPath path) noexcept;

  private:
    Font* unpackFontBinary (std::vector<uint8_t> const& binaryData) noexcept;
  };
}