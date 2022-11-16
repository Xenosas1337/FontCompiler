/*************************************************************************//**
 * \file    FontAsset.h
 * \author  Brandon Mak
 * \date    5 November 2022
 * \brief
 *
 * Copyright (C) 2022 DigiPen Institute of Technology. Reproduction or
 * disclosure of this file or its contents without the prior written consent
 * of DigiPen Institute of Technology is prohibited.
 *****************************************************************************/
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

  struct FontAsset
  {
    //! Glyphs. Corresponds to the transformation container below.
    std::vector<msdf_atlas::unicode_t> glyphs;

    //! Data containing character and uv transformation data and other misc data
    std::vector<GlyphData> glyphTransformations;

    //! The actual data of the atlas to go into the binary
    std::unique_ptr<unsigned char[]> bitmapData;

    //! Width of the bitmap
    uint32_t bitmapWidth;

    //! Height of the bitmap
    uint32_t bitmapHeight;
  };
}
