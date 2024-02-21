#include <iostream>
#include <fstream>
#include "FontLoader.hpp"

namespace dash_tools
{
  Font* FontLoader::ReadAndUnpackFileData(AssetPath path) noexcept
  {
    std::ifstream ifs{path.c_str(), std::ios::binary};

    // if file successfully loaded
    if (ifs.is_open())
    {
      auto FILE_SIZE = std::filesystem::file_size(path);

      // Allocate a container the size of file size being read
      std::vector<uint8_t> binaryData(FILE_SIZE);

      // Read the contents from the file
      ifs.read(reinterpret_cast<char*>(binaryData.data()), FILE_SIZE);

      // Proceed to read contents chunk by chunk and save it into new font object.
      auto* newFont = unpackFontBinary(binaryData);

      // Return the new font object
      return newFont;
    }
    else
    {
      std::cout << "FontLoader::ReadAndUnpackFileData: Could not load file: " << path.string() << std::endl; 
      return nullptr;
    }
  }

  Font* FontLoader::unpackFontBinary(std::vector<uint8_t> const& binaryData) noexcept
  {
    // Prepare object to initialize font object
    UnpackedFontData unpackedFontData{};

    // Read from binary and unpack data
    // Everything here should be read in the same order the FontCompiler packs the unpacked data
    {
      // For traversing the binary data
      uint32_t memoryCursor{ 0 };

      // Get number of glyphs available
      uint32_t numGlyphs{ 0 };
      std::memcpy(&numGlyphs, binaryData.data() + memoryCursor, sizeof(numGlyphs));
      memoryCursor += sizeof(numGlyphs);

      // Get glyph indexing data 
      for (uint32_t i = 0; i < numGlyphs; ++i)
      {
        GlyphIndexingData indexingData{};
        std::memcpy(&indexingData, binaryData.data() + memoryCursor, sizeof(GlyphIndexingData));
        memoryCursor += sizeof(GlyphIndexingData);

        unpackedFontData.glyphMappings.emplace(indexingData.glyph, indexingData.containerIndex);
      }

      // Get glyph data (since it's all stored contiguously, 1 single memcpy will suffice)
      uint32_t const GLYPH_DATA_BYTES = sizeof(GlyphData) * numGlyphs;
      unpackedFontData.glyphData.resize(numGlyphs);
      std::memcpy(unpackedFontData.glyphData.data(), binaryData.data() + memoryCursor, GLYPH_DATA_BYTES);
      memoryCursor += GLYPH_DATA_BYTES;

      // Get the bitmap's size in bytes
      uint32_t bitmapBytes{};
      std::memcpy(&bitmapBytes, binaryData.data() + memoryCursor, sizeof(bitmapBytes));
      memoryCursor += sizeof(bitmapBytes);

      // Get the bitmap's dimensions
      std::memcpy(&unpackedFontData.bitmapWidth, binaryData.data() + memoryCursor, sizeof(unpackedFontData.bitmapWidth));
      memoryCursor += sizeof(unpackedFontData.bitmapWidth);
      std::memcpy(&unpackedFontData.bitmapHeight, binaryData.data() + memoryCursor, sizeof(unpackedFontData.bitmapHeight));
      memoryCursor += sizeof(unpackedFontData.bitmapHeight);

      // Get the actual bitmap data
      unpackedFontData.fontBitmap.resize(bitmapBytes);
      std::memcpy(unpackedFontData.fontBitmap.data(), binaryData.data() + memoryCursor, bitmapBytes);
      memoryCursor += bitmapBytes;

      uint32_t numKernPairs{};
      std::memcpy(&numKernPairs, binaryData.data() + memoryCursor, sizeof(numKernPairs));
      memoryCursor += sizeof(numKernPairs);

      // Get kern pair data and place into map one by one
      for (uint32_t i = 0; i < numKernPairs; ++i)
      {
        PerKernPair kernPairData{};
        std::memcpy(&kernPairData, binaryData.data() + memoryCursor, sizeof(PerKernPair));
        unpackedFontData.kernPairs.emplace(std::pair{ kernPairData.lhs, kernPairData.rhs }, kernPairData.kerning);
      }
    }

    // Move the unpacked font data into new font object
    Font* newFont = new Font{ std::move(unpackedFontData) };

    // Return the new font object
    return newFont;
  }

}

