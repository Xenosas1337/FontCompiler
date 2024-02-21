#pragma once

#include "AssetMacros.hpp"
#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "FontCommonTypes.hpp"
#include <optional>

namespace dash_tools
{
  class FontCompiler
  {
  private:
    static void GenerateUnpackedFontData(UnpackedFontData&                             fontAsset, 
                                         std::vector<msdf_atlas::GlyphGeometry> const& glyphData, 
                                         msdf_atlas::FontGeometry const&               fontGeometry) noexcept;
  	
  public:
    static std::optional<AssetPath> LoadAndCompileFont   (msdfgen::FreetypeHandle* freetypeHandle, AssetPath path) noexcept;
    static UnpackedFontData const*  CompileFontToMemory  (msdfgen::FontHandle* fontHandle, AssetPath path) noexcept;
    static std::string              PackFontDataToFile  (AssetPath path, UnpackedFontData const& unpackedFontData) noexcept;
    
  };
}
