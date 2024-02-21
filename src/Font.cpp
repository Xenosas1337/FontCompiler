#include "Font.hpp"

namespace dash_tools
{

  Font::Font(UnpackedFontData&& unpackedFontData) noexcept
    : m_fontData{std::move(unpackedFontData)}
  {

  }

  std::map<dash_tools::GlyphType, uint32_t> const& Font::GetGlyphMappings(void) const noexcept
  {
    return m_fontData.glyphMappings;
  }

  std::vector<dash_tools::GlyphData> const& Font::GetGlyphData(void) const noexcept
  {
    return m_fontData.glyphData;
  }

  std::vector<dash_tools::BitmapPixelType> const& Font::GetFontBitmap(void) const noexcept
  {
    return m_fontData.fontBitmap;
  }

  uint32_t Font::GetBitmapWidth(void) const noexcept
  {
    return m_fontData.bitmapWidth;
  }

  uint32_t Font::GetBitmapHeight(void) const noexcept
  {
    return m_fontData.bitmapHeight;
  }

  std::optional<float> Font::GetKerning(GlyphType lhs, GlyphType rhs) const noexcept
  {
    if (m_fontData.glyphMappings.count(lhs) <= 0)
      return {};

    // Get the advance of the glyph
    float advance = m_fontData.glyphData[m_fontData.glyphMappings.at(lhs)].data[GLPYH_KERNING_ARRAY_INDEX];

    // Add with kern pair advance if exists
    std::pair<GlyphType, GlyphType> glyphPair{lhs, rhs};
    if (auto it = m_fontData.kernPairs.find (glyphPair); it != m_fontData.kernPairs.end())
      advance += it->second;

    return advance;
  }

}