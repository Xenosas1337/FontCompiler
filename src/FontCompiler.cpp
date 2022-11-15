#include "FontCompiler.h"
#include "msdfgen/include/lodepng.h"


#include <fstream>
#include <iostream>

namespace SH_COMP
{
  /***************************************************************************/
  /*!
  
    \brief
      Given a valid ptr to a font asset and relevant data, initialize the data
      inside the font asset. See SHFontAsset for details.
    
    \param fontAsset
      The ptr to the font asset.

    \param glyphData
      Individual glyph data.

    \param fontBitmap
      Actual bitmap data

    \param fontGeometry
     Font geometry required to get advance

  */
  /***************************************************************************/
  void FontCompiler::WriteToFontAsset(FontAsset* fontAsset, std::vector<msdf_atlas::GlyphGeometry> const& glyphData, msdfgen::Bitmap<msdf_atlas::byte, 3>& fontBitmap, msdf_atlas::FontGeometry const& fontGeometry) noexcept
  {
    if (!fontAsset)
      return;

    uint32_t numGlyphs = static_cast<uint32_t>(glyphData.size());
    for (uint32_t i = 0; i < numGlyphs; ++i)
    {
      // bounding box of the glyph in atlas
    	double atlasL = 0.0, atlasR = 0.0, atlasT = 0.0, atlasB = 0.0;

      // bounding box of glyph as it should be placed on the baseline
      double atlasPL = 0.0, atlasPR = 0.0, atlasPT = 0.0, atlasPB = 0.0;

      // initialize the bounding boxes
      glyphData[i].getQuadAtlasBounds(atlasL, atlasB, atlasR, atlasT);
      glyphData[i].getQuadPlaneBounds(atlasPL, atlasPB, atlasPR, atlasPT);

      // normalize the bounding box to (0 - 1).
      atlasL /= fontBitmap.width();
      atlasR /= fontBitmap.width();
      atlasT /= fontBitmap.height();
      atlasB /= fontBitmap.height();

      // Normalized texture dimensions
      float const NORMALIZED_TEX_DIMS[2] = { static_cast<float> (atlasR - atlasL), static_cast<float> (atlasT - atlasB) };

      // When we render the quad, it has to correctly scale depending on what letter/glyph we are rendering. This is for that scale.
      float const QUAD_SCALE[2] { static_cast<float> (atlasPR - atlasL), static_cast<float> (atlasT - atlasB) };

      // initialize a matrix for uv and quad transformation data
      GlyphData transformData
      {
        // For scaling the tex coords
        NORMALIZED_TEX_DIMS[0],       0.0f,                     0.0f,                        0.0f, 
        0.0f,                       NORMALIZED_TEX_DIMS[1],     0.0f,                        0.0f, 

        // For translating the tex coords
        static_cast<float>(atlasL), static_cast<float>(atlasB), 1.0f,                        0.0f, 

        // Stores the transformation for a quad to correctly shape the glyph (first 2 values) and the bearing (last 2)
        QUAD_SCALE[0],              QUAD_SCALE[1],              static_cast<float>(atlasPL), static_cast<float>(atlasPB)
      };

      // Push 1 set of data for a character/glyph into the asset.
      fontAsset->glyphTransformations.push_back(transformData);
      fontAsset->glyphs.push_back(glyphData[i].getCodepoint());
    }

    uint32_t bytesRequired = fontBitmap.width() * fontBitmap.height() * NUM_CHANNELS * BYTES_PER_CHANNEL;

    // copy data from bitmap to asset. Each channel is a 32 bit float and there are 3 channels.
    fontAsset->bitmapData = std::make_unique<unsigned char[]>(bytesRequired);
    std::memcpy (fontAsset->bitmapData.get(), fontBitmap.operator msdf_atlas::byte *(), bytesRequired);
    
    msdf_atlas::saveImage(fontBitmap.operator msdfgen::BitmapConstRef<msdf_atlas::byte, 3>(), msdf_atlas::ImageFormat::PNG, "test_font/testPNG.png", msdf_atlas::YDirection::TOP_DOWN);

    fontAsset->bitmapWidth = fontBitmap.width();
    fontAsset->bitmapHeight = fontBitmap.height();
  }

  /***************************************************************************/
  /*!
  
    \brief
      Loads and compiles a font to binary format. Returns a path to the binary
      data.
    
    \param path
      Path to the font file (truetype font file) to load.
   
    \return 
      Path to newly created binary data.
  
  */
  /***************************************************************************/
  std::optional<AssetPath> FontCompiler::LoadAndCompileFont(msdfgen::FreetypeHandle* freetypeHandle, AssetPath path) noexcept
  {
    msdfgen::FontHandle* fontHandle = nullptr;
    
    // XQ I need your help for path manipulation to actually load the msdfgen::FontHandle here. Am I doing this correctly?
    fontHandle = msdfgen::loadFont(freetypeHandle, path.string().c_str());

    if (fontHandle)
    {
      // Compile a font asset
      auto* fontAsset = CompileFontToMemory(fontHandle);

      // No path to binary format
      if (!fontAsset)
        return {};

      return CompileFontToBinary(path, *fontAsset);
    }

    std::cout << "Unable to open font file: " << path.string() << std::endl;

    return {};
  }

  /***************************************************************************/
  /*!
  
    \brief
      This function takes in a font handle and generates a font asset from it.
      It first generates an atlas and all relevant data before creating the
      asset.
    
    \param fontHandle
      MSDF font handle required to initialize member variables in SHFontAsset.
   
    \return 
      A pointer to a brand new font asset.
  
  */
  /***************************************************************************/
  FontAsset const* FontCompiler::CompileFontToMemory(msdfgen::FontHandle* fontHandle) noexcept
  {
    // Individual glyph geometry
    std::vector<msdf_atlas::GlyphGeometry> glyphData;

    // Actual bitmap data
    //msdfgen::Bitmap<msdfgen::byte, 3> fontBitmap;
    msdfgen::Bitmap<msdf_atlas::byte, 3> fontBitmap;

    // Font geometry required to get advance
    msdf_atlas::FontGeometry fontGeometry (&glyphData);

    // Load char set
    fontGeometry.loadCharset(fontHandle, 1.0, msdf_atlas::Charset::ASCII);

    // Apply MSDF edge coloring
    const double maxCornerAngle = 3.0;
    for (msdf_atlas::GlyphGeometry& glyph : glyphData)
      glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

    // configure parameters for atlas generation
    msdf_atlas::TightAtlasPacker atlasPacker;
    atlasPacker.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);

    atlasPacker.setMinimumScale(64.0);
    atlasPacker.setPixelRange(2.0);
    atlasPacker.setMiterLimit(1.0);
    atlasPacker.pack(glyphData.data(), static_cast<int>(glyphData.size()));

    // Get the dimensions after applying parameters
    int width = 0, height = 0;
    atlasPacker.getDimensions(width, height);

   // generate the atlas
    msdf_atlas::ImmediateAtlasGenerator<float, 3, msdf_atlas::msdfGenerator, msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>> generator(width, height);
    msdf_atlas::GeneratorAttributes genAttribs;
    generator.setAttributes(genAttribs);
    generator.setThreadCount(4);
    generator.generate(glyphData.data(), static_cast<int>(glyphData.size()));

    fontBitmap = std::move(((msdfgen::Bitmap<msdf_atlas::byte, 3>&&)generator.atlasStorage()));
    //fontBitmap = std::move(((msdfgen::Bitmap<msdfgen::byte, 3>&&)generator.atlasStorage()));

    // at this point we have all the required data to initialize a font asset.

    // Dynamically allocate new asset
    FontAsset* newAsset = new FontAsset();

    // Now we populate it with data
    WriteToFontAsset(newAsset, glyphData, fontBitmap, fontGeometry);

    return newAsset;
  }

  /***************************************************************************/
  /*! 
   
    \brief
      After generating the asset we call this function to serialize the font
      data into binary data.
    
    \param path
      path to font file (?).

    \param asset
      Asset to write. 
   
    \return 
      Path the asset.
  
  */ 
  /***************************************************************************/
  std::string FontCompiler::CompileFontToBinary(AssetPath path, FontAsset const& asset) noexcept
  {
    std::string newPath{ path.string() };
    newPath = newPath.substr(0, newPath.find_last_of('.'));
    newPath += FONT_EXTENSION.data();

    std::ofstream file{ newPath, std::ios::binary | std::ios::out | std::ios::trunc };

    uint32_t numGlyphs = asset.glyphTransformations.size();

    // Write number of glyphs first
    file.write(reinterpret_cast<char const*>(&numGlyphs), sizeof (uint32_t));

    for (uint32_t i = 0; i < numGlyphs; ++i)
    {
      // write the glyph first
      file.write(reinterpret_cast<char const*>(&asset.glyphs[i]), sizeof(msdfgen::unicode_t));
    }

    for (uint32_t i = 0; i < numGlyphs; ++i)
    {
      // then write the data next to it
      file.write (reinterpret_cast<char const*>(&asset.glyphTransformations[i]), sizeof (GlyphData));
    }

    // Write width of bitmap
    file.write(reinterpret_cast<char const*>(&asset.bitmapWidth), sizeof (uint32_t));

    // Write height of bitmap
    file.write(reinterpret_cast<char const*>(&asset.bitmapHeight), sizeof(uint32_t));

    // write size required by bitmap
    uint32_t bytesRequired = asset.bitmapWidth * asset.bitmapHeight * BYTES_PER_CHANNEL * NUM_CHANNELS;
    file.write(reinterpret_cast<char const*>(&bytesRequired), sizeof(uint32_t));


    // now we write the actual bitmap
    file.write(reinterpret_cast<char const*>(asset.bitmapData.get()), bytesRequired);


    file.close();

    return newPath;
  }

}