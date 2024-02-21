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
  void FontCompiler::GenerateFontToBinaryData(FontToBinaryData&                             fontToBinaryData, 
                                              std::vector<msdf_atlas::GlyphGeometry> const& glyphData, 
                                              msdf_atlas::FontGeometry const&               fontGeometry) noexcept
  {

    // Bitmap dimensions saved locally for convenience
    uint32_t BITMAP_WIDTH = fontToBinaryData.fontBitmap.width();
    uint32_t BITMAP_HEIGHT = fontToBinaryData.fontBitmap.height();

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
      atlasL /= BITMAP_WIDTH;
      atlasR /= BITMAP_WIDTH;
      atlasT /= BITMAP_HEIGHT;
      atlasB /= BITMAP_HEIGHT;

      // Normalized texture dimensions
      float const NORMALIZED_TEX_DIMS[2] = { static_cast<float> (atlasR - atlasL), static_cast<float> (atlasT - atlasB) };

      // When we render the quad, it has to correctly scale depending on what letter/glyph we are rendering. This is for that scale.
      float const QUAD_SCALE[2] { static_cast<float> (atlasPR - atlasPL), static_cast<float> (atlasPT - atlasPB) };

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
      fontToBinaryData.glyphTransformations.push_back(transformData);
      fontToBinaryData.glyphs.push_back(glyphData[i].getCodepoint());
    }

    // font geometry kerning
    auto const& FG_KERNING = fontGeometry.getKerning();

    // Copies every kern pair to a separate map.
    for (auto const& [PAIR, KERNING] : FG_KERNING)
      fontToBinaryData.kernPairs.emplace(PAIR, static_cast<float> (KERNING));

    // Stores the glyph advances indexable by the glyph
    auto const& GLYPHS = fontGeometry.getGlyphs();
    for (auto const& GLYPH : GLYPHS)
      fontToBinaryData.glyphKerning.emplace(GLYPH.getCodepoint(), GLYPH.getAdvance());

    

    //// copy data from bitmap to asset. Each channel is a 32 bit float and there are 3 channels.
    //fontAsset.bitmapData = std::make_unique<unsigned char[]>(bytesRequired);
    //std::memcpy (fontAsset.bitmapData.get(), fontBitmap.operator msdf_atlas::byte *(), bytesRequired);
    //
    //static int testWriteIndex = 0;

    //msdf_atlas::saveImage(fontBitmap.operator msdfgen::BitmapConstRef<msdf_atlas::byte, 4>(), msdf_atlas::ImageFormat::PNG, std::string ("Fonts/testPNG" + std::to_string(testWriteIndex++) + ".png").c_str(), msdf_atlas::YDirection::TOP_DOWN);

    //fontAsset.bitmapWidth = fontAsset.fontBitmap.width();
    //fontAsset.bitmapHeight = fontBitmap.height();
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
      auto* fontData = CompileFontToMemory(fontHandle);

      // No path to binary format
      if (!fontData)
        return {};

      return CompileFontToBinary(path, *fontData);
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
  FontToBinaryData const* FontCompiler::CompileFontToMemory(msdfgen::FontHandle* fontHandle) noexcept
  {
    // Dynamically allocate new asset
    FontToBinaryData* newData = new FontToBinaryData();

    // Individual glyph geometry
    std::vector<msdf_atlas::GlyphGeometry> glyphData;

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
    msdf_atlas::ImmediateAtlasGenerator<float, 4, msdf_atlas::mtsdfGenerator, msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>> generator(width, height);
    msdf_atlas::GeneratorAttributes genAttribs;
    generator.setAttributes(genAttribs);
    generator.setThreadCount(4);
    generator.generate(glyphData.data(), static_cast<int>(glyphData.size()));

    newData->fontBitmap = std::move(((msdfgen::Bitmap<msdf_atlas::byte, 4>&&)generator.atlasStorage()));
    //fontBitmap = std::move(((msdfgen::Bitmap<msdfgen::byte, 3>&&)generator.atlasStorage()));

    // at this point we have all the required data to initialize a font asset.


    // Now we populate it with data
    GenerateFontToBinaryData(*newData, glyphData, fontGeometry);

    return newData;
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
  std::string FontCompiler::CompileFontToBinary(AssetPath path, FontToBinaryData const& fontToBinaryData) noexcept
  {
    std::string newPath{ path.string() };
    newPath = newPath.substr(0, newPath.find_last_of('.'));
    newPath += FONT_EXTENSION.data();

    // Bitmap dimensions saved locally for convenience
    uint32_t const BITMAP_WIDTH = fontToBinaryData.fontBitmap.width();
    uint32_t const BITMAP_HEIGHT = fontToBinaryData.fontBitmap.height();

    // Number of glyphs on stack for convenience
    uint32_t const NUM_GLYPHS = static_cast<uint32_t>(fontToBinaryData.glyphTransformations.size());

    // size required by bitmap
    uint32_t const BITMAP_BYTES = BITMAP_WIDTH * BITMAP_HEIGHT * BYTES_PER_CHANNEL * NUM_CHANNELS;

    // size required for glyphs themselves
    uint32_t const GLYPHS_BYTES = static_cast<uint32_t>(sizeof(msdfgen::unicode_t) * fontToBinaryData.glyphs.size());

    uint32_t const GLYPHS_DATA_BYTES = static_cast<uint32_t>(sizeof(GlyphData) * fontToBinaryData.glyphTransformations.size());

    // Number of glyph kernings
    uint32_t const NUM_GLYPH_KERNINGS = static_cast<uint32_t>(fontToBinaryData.glyphKerning.size());

    // bytes required to store glyph specific kernings
    uint32_t const GLYPH_KERNING_BYTES = static_cast<uint32_t> (sizeof(PerGlyphKerning) * fontToBinaryData.glyphKerning.size());

    // Number of unique kern pairs
    uint32_t const NUM_KERN_PAIRS = static_cast<uint32_t>(fontToBinaryData.kernPairs.size());

    // bytes required for kerning pairs
    uint32_t const KERN_PAIR_BYTES = static_cast<uint32_t>(sizeof(PerKernPair) * fontToBinaryData.kernPairs.size());


    // number of bytes required to store binary data
    uint32_t const BYTES_REQUIRED = sizeof (NUM_GLYPHS) +        // number of glyphs
                                    GLYPHS_BYTES +               // Actual glyph data
                                    GLYPHS_DATA_BYTES +          // Glyph data stored in matrix
                                    sizeof (BITMAP_BYTES) +      // Bytes required to store bitmap
                                    sizeof (BITMAP_WIDTH) +      // Width of bitmap 
                                    sizeof (BITMAP_HEIGHT) +     // Height of bitmap
                                    BITMAP_BYTES +               // Actual bitmap data
                                    sizeof(NUM_GLYPH_KERNINGS) + // Number of unique glyph kernings
                                    GLYPH_KERNING_BYTES +        // Bytes required for Glyph specific kernings
                                    sizeof(NUM_KERN_PAIRS) +     // Number of unique kern pairs
                                    KERN_PAIR_BYTES;             // Bytes required for Kerning pairs

    std::vector<char> toFileData{};
    uint32_t memoryCursor = 0;
    toFileData.resize(BYTES_REQUIRED);

    // Write number of glyphs
    std::memcpy (toFileData.data() + memoryCursor, &NUM_GLYPHS, sizeof(NUM_GLYPHS));
    memoryCursor += sizeof(NUM_GLYPHS);

    // Write glyphs themselves
    std::memcpy(toFileData.data() + memoryCursor, fontToBinaryData.glyphs.data(), GLYPHS_BYTES);
    memoryCursor += GLYPHS_BYTES;

    // write the glyph data
    std::memcpy(toFileData.data() + memoryCursor, fontToBinaryData.glyphTransformations.data(), GLYPHS_BYTES);
    memoryCursor += GLYPHS_DATA_BYTES;

    // Write the bitmap's size in bytes
    std::memcpy(toFileData.data() + memoryCursor, &BITMAP_BYTES, sizeof(BITMAP_BYTES));
    memoryCursor += sizeof(BITMAP_BYTES);

    // Write the bitmap's width
    std::memcpy(toFileData.data() + memoryCursor, &BITMAP_WIDTH, sizeof(BITMAP_WIDTH));
    memoryCursor += sizeof(BITMAP_WIDTH);

    // Write the bitmap's height
    std::memcpy(toFileData.data() + memoryCursor, &BITMAP_HEIGHT, sizeof(BITMAP_HEIGHT));
    memoryCursor += sizeof(BITMAP_HEIGHT);

    // Write the bitmap data
    std::memcpy(toFileData.data() + memoryCursor, fontToBinaryData.fontBitmap.operator msdf_atlas::byte const* (), BITMAP_BYTES);
    memoryCursor += BITMAP_BYTES;

    // Number of glyph kernings
    std::memcpy (toFileData.data() + memoryCursor, &NUM_GLYPH_KERNINGS, sizeof(NUM_GLYPH_KERNINGS));
    memoryCursor += sizeof(NUM_GLYPH_KERNINGS);

    // Write each per glyph kerning
    for (auto const& [GLYPH, KERNING]  : fontToBinaryData.glyphKerning)
    {
      PerGlyphKerning const GLYPH_KERNING{ GLYPH, KERNING};
      std::memcpy(toFileData.data() + memoryCursor, &GLYPH_KERNING, sizeof(PerGlyphKerning));
      memoryCursor += sizeof(PerGlyphKerning);
    }

    // Number of glyph kernings
    std::memcpy(toFileData.data() + memoryCursor, &NUM_KERN_PAIRS, sizeof(NUM_KERN_PAIRS));
    memoryCursor += sizeof(NUM_KERN_PAIRS);

    // Write unique kerning pairs
    for (auto const& [PAIR, KERNING] : fontToBinaryData.kernPairs)
    {
      PerKernPair const PER_KERN_PAIR{ PAIR.first, PAIR.second, KERNING };
      std::memcpy(toFileData.data() + memoryCursor, &PER_KERN_PAIR, sizeof(PerKernPair));
      memoryCursor += sizeof(PerKernPair);
    }
    
    // Open a file for writing
    std::ofstream file{ newPath, std::ios::binary | std::ios::out | std::ios::trunc };

    file.write (reinterpret_cast<char const*>(toFileData.data()), BYTES_REQUIRED);
    //// Write number of glyphs
    //file.write(reinterpret_cast<char const*>(&NUM_GLYPHS), sizeof (uint32_t));

    //// Write the glphys (same order as above)
    //file.write(reinterpret_cast<char const*>(fontToBinaryData.glyphs.data()), GLYPHS_BYTES);

    //// Write the data of each glyph in the same order as the glyphs (stored as matrix)
    //file.write(reinterpret_cast<char const*>(fontToBinaryData.glyphTransformations.data()), GLYPHS_DATA_BYTES);

    //file.write(reinterpret_cast<char const*>(&BITMAP_BYTES), sizeof(uint32_t));

    //// Write width of bitmap
    //file.write(reinterpret_cast<char const*>(&BITMAP_WIDTH), sizeof (uint32_t));

    //// Write height of bitmap
    //file.write(reinterpret_cast<char const*>(&BITMAP_HEIGHT), sizeof(uint32_t));

    //// now we write the actual bitmap
    ////file.write(reinterpret_cast<char const*>(asset.bitmapData.get()), BITMAP_BYTES);
    //file.write(reinterpret_cast<char const*>(fontToBinaryData.fontBitmap.operator msdf_atlas::byte const* ()), BITMAP_BYTES);

    file.close();

    // Write to a separate image file that just contains the atlas
    bool imageSaved = msdf_atlas::saveImage(fontToBinaryData.fontBitmap.operator msdfgen::BitmapConstRef<msdf_atlas::byte, 4>(), 
                          msdf_atlas::ImageFormat::PNG, 
                          path.replace_extension(".png").string().c_str(),
                          msdf_atlas::YDirection::TOP_DOWN);

    if (!imageSaved)
      std::cout << "Failed to save image. " << std::endl;


    return newPath;
  }

}