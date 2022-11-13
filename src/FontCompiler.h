#pragma once

#include "AssetMacros.h"
#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "FontAsset.h"

namespace SH_COMP
{
  class SHFontCompiler
  {
  private:
    static void WriteToFontAsset (FontAsset* fontAsset, std::vector<msdf_atlas::GlyphGeometry> const& glyphData, msdfgen::Bitmap<msdfgen::byte, 3> const& fontBitmap, msdf_atlas::FontGeometry const& fontGeometry) noexcept;
  	
  public:
    static std::optional<AssetPath> LoadAndCompileFont   (msdfgen::FreetypeHandle* freetypeHandle, AssetPath path) noexcept;
    static FontAsset const*         CompileFontToMemory  (msdfgen::FontHandle* fontHandle) noexcept;
    static std::string              CompileFontToBinary  (AssetPath path, FontAsset const& asset) noexcept;
    
  };
}
