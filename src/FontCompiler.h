#pragma once

#include "AssetMacros.h"
#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "FontToBinaryData.h"

namespace SH_COMP
{
  class FontCompiler
  {
  private:
    static void GenerateFontToBinaryData(FontToBinaryData&                             fontAsset, 
                                         std::vector<msdf_atlas::GlyphGeometry> const& glyphData, 
                                         msdf_atlas::FontGeometry const&               fontGeometry) noexcept;
  	
  public:
    static std::optional<AssetPath> LoadAndCompileFont   (msdfgen::FreetypeHandle* freetypeHandle, AssetPath path) noexcept;
    static FontToBinaryData const*  CompileFontToMemory  (msdfgen::FontHandle* fontHandle) noexcept;
    static std::string              CompileFontToBinary  (AssetPath path, FontToBinaryData const& asset) noexcept;
    
  };
}
