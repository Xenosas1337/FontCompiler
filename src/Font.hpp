#pragma once

#include <vector>
#include <map>
#include "FontCommonTypes.hpp"

namespace dash_tools
{
  class Font
  {

  public:
    //*************************************************************************
    // CONSTRUCTORS AND DESTRUCTORS
    //*************************************************************************
    Font (UnpackedFontData&& unpackedFontData) noexcept;

    //*************************************************************************
    // SETTERS AND GETTERS
    //*************************************************************************
    std::map<GlyphType, uint32_t> const& GetGlyphMappings (void) const noexcept;
    std::vector<GlyphData> const&        GetGlyphData     (void) const noexcept;
    std::vector<BitmapPixelType> const&  GetFontBitmap    (void) const noexcept;
    uint32_t                             GetBitmapWidth   (void) const noexcept;
    uint32_t                             GetBitmapHeight  (void) const noexcept;
    std::optional<float>                 GetKerning       (GlyphType lhs, GlyphType rhs) const noexcept;

  private:
    //*************************************************************************
    // PRIVATE MEMBER VARIABLES
    //*************************************************************************
    // Font data readily available to be used
    UnpackedFontData m_fontData;

  };
}