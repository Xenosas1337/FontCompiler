#include "FontCompiler.hpp"
#include "msdfgen/include/lodepng.h"


#include <fstream>
#include <iostream>

namespace dash_tools
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
  void FontCompiler::GenerateUnpackedFontData(UnpackedFontData&                             unpackedFontData, 
                                              std::vector<msdf_atlas::GlyphGeometry> const& glyphGeometry, 
                                              msdf_atlas::FontGeometry const&               fontGeometry) noexcept
  {

    // Bitmap dimensions saved locally for convenience
    uint32_t const BITMAP_WIDTH = unpackedFontData.bitmapWidth;
    uint32_t const BITMAP_HEIGHT = unpackedFontData.bitmapHeight;

    uint32_t const NUM_GLYPHS = static_cast<uint32_t>(glyphGeometry.size());
    for (uint32_t i = 0; i < NUM_GLYPHS; ++i)
    {
      // bounding box of the glyph in atlas
    	double atlasL = 0.0, atlasR = 0.0, atlasT = 0.0, atlasB = 0.0;

      // bounding box of glyph as it should be placed on the baseline
      double atlasPL = 0.0, atlasPR = 0.0, atlasPT = 0.0, atlasPB = 0.0;

      // Get the glyph's bounding box in bitmap space. 
      glyphGeometry[i].getQuadAtlasBounds(atlasL, atlasB, atlasR, atlasT);

      // Get the quad's 
      glyphGeometry[i].getQuadPlaneBounds(atlasPL, atlasPB, atlasPR, atlasPT);

      // normalize the bounding box to 0.0f-1.0f (i.e. texture space) 
      atlasL /= BITMAP_WIDTH;
      atlasR /= BITMAP_WIDTH;
      atlasT /= BITMAP_HEIGHT;
      atlasB /= BITMAP_HEIGHT;

      // Normalized texture dimensions
      float const NORMALIZED_TEX_DIMS[2] = { static_cast<float> (atlasR - atlasL), static_cast<float> (atlasT - atlasB) };

      // When we render the quad, it has to correctly scale depending on what letter/glyph we are rendering. This is for that scale.
      float const QUAD_SCALE[2] { static_cast<float> (atlasPR - atlasPL), static_cast<float> (atlasPT - atlasPB) };

      // initialize a matrix for uv and quad transformation data
      GlyphData currentGlyphData
      { {

          // For scaling the tex coords
          NORMALIZED_TEX_DIMS[0],       0.0f,                     0.0f,                        0.0f,
          0.0f,                       NORMALIZED_TEX_DIMS[1],     0.0f,                        0.0f,

          // For translating the tex coords to correct offset in bitmap texture
          static_cast<float>(atlasL), static_cast<float>(atlasB), 1.0f,                        0.0f,

          // Stores the transformation for a quad to correctly shape the glyph (first 2 values) and the bearing (last 2)
          QUAD_SCALE[0],              QUAD_SCALE[1],              static_cast<float>(atlasPL), static_cast<float>(atlasPB)
          },
        static_cast<GlyphKerningType>(glyphGeometry[i].getAdvance())
      };

      // Push 1 set of data for a character/glyph into the asset.
      unpackedFontData.glyphData.push_back(currentGlyphData);
      unpackedFontData.glyphMappings.emplace(static_cast<GlyphType>(glyphGeometry[i].getCodepoint()), i);
    }

    // font geometry kerning
    auto const& FG_KERNING = fontGeometry.getKerning();

    // Copies every kern pair to a separate map.
    for (auto const& [PAIR, KERNING] : FG_KERNING)
      unpackedFontData.kernPairs.emplace(PAIR, static_cast<float> (KERNING));
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
    
    fontHandle = msdfgen::loadFont(freetypeHandle, path.string().c_str());

    if (fontHandle)
    {
      // Extract relevant memory from font handle
      auto* unpackedFontData = CompileFontToMemory(fontHandle, path);

      // No path to binary format
      if (!unpackedFontData)
        return {};

      return PackFontDataToFile(path, *unpackedFontData);
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
      A pointer to an object storing data meant for the binary file.
  
  */
  /***************************************************************************/
  UnpackedFontData const* FontCompiler::CompileFontToMemory(msdfgen::FontHandle* fontHandle, AssetPath path) noexcept
  {
    // Dynamically allocate new asset
    UnpackedFontData* newData = new UnpackedFontData();

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

    // Write to a separate image file that just contains the atlas for testing
    bool imageSaved = msdf_atlas::saveImage(generator.atlasStorage().operator msdfgen::BitmapConstRef<msdf_atlas::byte, 4>(),
                                            msdf_atlas::ImageFormat::PNG,
                                            path.replace_extension(".png").string().c_str(),
                                            msdf_atlas::YDirection::TOP_DOWN);

    if (!imageSaved)
      std::cout << "Tester code: Failed to save image. " << std::endl;

    //msdfgen::Bitmap<msdfgen::byte, 3> fontBitmap;
    msdfgen::Bitmap<msdf_atlas::byte, 4> fontBitmap = std::move(((msdfgen::Bitmap<msdf_atlas::byte, 4>&&)generator.atlasStorage()));

    // Copy the bitmap to unpacked data object
    uint32_t const BITMAP_BYTES = fontBitmap.width() * fontBitmap.height() * NUM_CHANNELS * BYTES_PER_CHANNEL;
    newData->bitmapWidth = fontBitmap.width();
    newData->bitmapHeight = fontBitmap.height();
    newData->fontBitmap.resize(BITMAP_BYTES);
    std::memcpy(newData->fontBitmap.data(), fontBitmap.operator msdf_atlas::byte*(), BITMAP_BYTES);

    // at this point we have all the required data to initialize a font asset.

    // Now we populate it with data
    GenerateUnpackedFontData(*newData, glyphData, fontGeometry);

    return newData;
  }

  /***************************************************************************/
  /*! 
   
    \brief
      After generating the asset we call this function to serialize the font
      data into binary data and then to file.
    
    \param path
      path to font file (?).

    \param asset
      Asset to write. 
   
    \return 
      Path the asset.
  
  */ 
  /***************************************************************************/
  std::string FontCompiler::PackFontDataToFile(AssetPath path, UnpackedFontData const& unpackedFontData) noexcept
  {
    std::string newPath{ path.string() };
    newPath = newPath.substr(0, newPath.find_last_of('.'));
    newPath += FONT_EXTENSION.data();

    // Bitmap dimensions saved locally for convenience
    uint32_t const BITMAP_WIDTH = unpackedFontData.bitmapWidth;
    uint32_t const BITMAP_HEIGHT = unpackedFontData.bitmapHeight;

    // Number of glyphs on stack for convenience
    uint32_t const NUM_GLYPHS = static_cast<uint32_t>(unpackedFontData.glyphData.size());

    uint32_t const GLYPH_MAPPING_BYTES = static_cast<uint32_t>(unpackedFontData.glyphMappings.size() * (sizeof(uint32_t) + sizeof(GlyphType)));

    // size required by bitmap
    uint32_t const BITMAP_BYTES = BITMAP_WIDTH * BITMAP_HEIGHT * BYTES_PER_CHANNEL * NUM_CHANNELS;

    // size required to store the glyph specific data
    uint32_t const GLYPHS_DATA_BYTES = static_cast<uint32_t>(sizeof(GlyphData) * unpackedFontData.glyphData.size());

    // Number of unique kern pairs
    uint32_t const NUM_KERN_PAIRS = static_cast<uint32_t>(unpackedFontData.kernPairs.size());

    // bytes required for kerning pairs
    uint32_t const KERN_PAIR_BYTES = static_cast<uint32_t>(sizeof(PerKernPair) * unpackedFontData.kernPairs.size());


    // number of bytes required to store binary data
    uint32_t const BYTES_REQUIRED = sizeof (NUM_GLYPHS) +        // number of glyphs
                                    GLYPH_MAPPING_BYTES +        // Bytes required to store GlyphIndex-uint32_t key value pairs
                                    GLYPHS_DATA_BYTES +          // Glyph data stored in matrix
                                    sizeof (BITMAP_BYTES) +      // Bytes required to store bitmap
                                    sizeof (BITMAP_WIDTH) +      // Width of bitmap 
                                    sizeof (BITMAP_HEIGHT) +     // Height of bitmap
                                    BITMAP_BYTES +               // Actual bitmap data
                                    sizeof(NUM_KERN_PAIRS) +     // Number of unique kern pairs
                                    KERN_PAIR_BYTES;             // Bytes required for Kerning pairs

    std::vector<uint8_t> toFileData{};
    uint32_t memoryCursor = 0;
    toFileData.resize(BYTES_REQUIRED);

    // Write number of glyphs
    std::memcpy (toFileData.data() + memoryCursor, &NUM_GLYPHS, sizeof(NUM_GLYPHS));
    memoryCursor += sizeof(NUM_GLYPHS);

    // write the glyph indexing data
    for (auto const& [GLYPH, INDEX] : unpackedFontData.glyphMappings)
    {
      GlyphIndexingData INDEXING_DATA{GLYPH, INDEX };

      // Write each glyph's unique index into the data container
      std::memcpy(toFileData.data() + memoryCursor, &INDEXING_DATA, sizeof(GlyphIndexingData));
      memoryCursor += sizeof(GlyphIndexingData);
    }

    // write the glyph data
    std::memcpy(toFileData.data() + memoryCursor, unpackedFontData.glyphData.data(), GLYPHS_DATA_BYTES);
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
    std::memcpy(toFileData.data() + memoryCursor, unpackedFontData.fontBitmap.data(), BITMAP_BYTES);
    memoryCursor += BITMAP_BYTES;

    // Number of glyph kern pairs
    std::memcpy(toFileData.data() + memoryCursor, &NUM_KERN_PAIRS, sizeof(NUM_KERN_PAIRS));
    memoryCursor += sizeof(NUM_KERN_PAIRS);

    // Write unique kerning pairs
    for (auto const& [PAIR, KERNING] : unpackedFontData.kernPairs)
    {
      PerKernPair const PER_KERN_PAIR{ PAIR.first, PAIR.second, KERNING };
      std::memcpy(toFileData.data() + memoryCursor, &PER_KERN_PAIR, sizeof(PerKernPair));
      memoryCursor += sizeof(PerKernPair);
    }
    
    // Open a file for writing
    std::ofstream file{ newPath, std::ios::binary | std::ios::out | std::ios::trunc };

    file.write (reinterpret_cast<char const*>(toFileData.data()), BYTES_REQUIRED);

    file.close();



    return newPath;
  }

}