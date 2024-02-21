#pragma once

#include <vector>
#include <string>
#include <tuple>
#include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace SH_COMP
{
  static constexpr uint32_t FONT_MATRIX_SIZE = 16;
  static constexpr uint32_t NUM_CHANNELS = 4;
  static constexpr uint32_t BYTES_PER_CHANNEL = 1;


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

  struct PerGlyphKerning
  {
    int glyph;
    float kerning;
  };

  struct FontToBinaryData
  {
    // Glyphs. Corresponds to the transformation container below.
    std::vector<msdf_atlas::unicode_t> glyphs;

    // Data containing character and uv transformation data and other misc data stored in a single matrix
    std::vector<GlyphData> glyphTransformations;

    // Actual bitmap data. 
    // Will be writing width and height to binary file from here too
    msdfgen::Bitmap<msdf_atlas::byte, 4> fontBitmap;

    // All glyphs will have their own glyph advances
    std::map<int, float> glyphKerning;

    // Stores the kerning value for each letter pair (to be added to glyph advances if it exists). 
    // MSDF atlas gen uses double for kerning but to save space we'll use a float instead.
    // I could have stored a map of doubles and then static_cast later, but I wanted it to be more obvious what's going into the binary file.
    std::map<std::pair<int, int>, float> kernPairs;


  };
}
