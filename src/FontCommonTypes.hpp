#pragma once

#include <cstdint>
#include "AssetMacros.hpp"

namespace dash_tools
{
  // Typedefs
  using AssetPath = std::filesystem::path;
  using GlyphType = int;
  using GlyphKerningType = float;
  using KernPairData = std::map<std::pair<GlyphType, GlyphType>, GlyphKerningType>;

  // MSDF actually gives a options for the channel type amd number of channels per pixel. 
  // However, we really only care about RGBA for now 8 bit per channel for now
  using BitmapPixelType = uint32_t; 
  static constexpr uint32_t FONT_MATRIX_SIZE = 16;
  static constexpr uint32_t NUM_CHANNELS = 4;
  static constexpr uint32_t BYTES_PER_CHANNEL = 1;

  static constexpr uint32_t GLPYH_TEX_DIMS_X_ARRAY_INDEX = 0;
  static constexpr uint32_t GLPYH_TEX_DIMS_Y_ARRAY_INDEX = 1;
  static constexpr uint32_t GLPYH_KERNING_ARRAY_INDEX = 3;
  static constexpr uint32_t GLPYH_TEX_POS_X_ARRAY_INDEX = 4;
  static constexpr uint32_t GLPYH_TEX_POS_Y_ARRAY_INDEX = 5;
  static constexpr uint32_t GLYPH_SCALE_X_ARRAY_INDEX = 8;
  static constexpr uint32_t GLYPH_SCALE_Y_ARRAY_INDEX = 9;
  static constexpr uint32_t GLYPH_POS_X_ARRAY_INDEX = 10;
  static constexpr uint32_t GLYPH_POS_Y_ARRAY_INDEX = 11;


  struct GlyphIndexingData
  {
    GlyphType glyph;
    uint32_t containerIndex;
  };

  struct GlyphData
  {
    float data[FONT_MATRIX_SIZE];
  };

  struct PerKernPair
  {
    int lhs;
    int rhs;
    float kerning;
  };

  struct UnpackedFontData
  {
    // Maps glyph characters to index into glyph data vector
    std::map<GlyphType, uint32_t> glyphMappings;

    // Data containing character and uv transformation data and other misc data stored in a single matrix
    std::vector<GlyphData> glyphData;

    // Actual bitmap data. 
    // Will be writing width and height to binary file from here too
    //msdfgen::Bitmap<msdf_atlas::byte, 4> fontBitmap;
    std::vector<BitmapPixelType> fontBitmap;

    // Width of the bitmap
    uint32_t bitmapWidth;

    // Height of the bitmap
    uint32_t bitmapHeight;

    // Stores the kerning value for each letter pair (to be added to glyph advances if it exists). 
    // MSDF atlas gen uses double for kerning but to save space we'll use a float instead.
    // I could have stored a map of doubles and then static_cast later, but I wanted it to be more obvious what's going into the binary file.
    KernPairData kernPairs;

    UnpackedFontData (void) = default;
    UnpackedFontData (UnpackedFontData&& rhs) noexcept = default;

    UnpackedFontData(UnpackedFontData const& rhs) = delete;
    UnpackedFontData& operator=(UnpackedFontData const& rhs) = delete;
    UnpackedFontData& operator=(UnpackedFontData&& rhs) = delete;;

  };

}