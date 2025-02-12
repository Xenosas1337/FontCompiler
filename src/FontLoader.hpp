#pragma once

#include "Font.hpp"
#include <iostream>
#include <fstream>

namespace dash_tools
{
  class FontLoader
  {

  public:
    template <typename PointerType = Font*, typename = std::enable_if_t<
      std::is_same_v<PointerType, Font*> ||
      std::is_same_v<PointerType, std::shared_ptr<Font>> || 
      std::is_same_v<PointerType, std::unique_ptr<Font>>>>
      static PointerType ReadAndUnpackFileData(AssetPath path) noexcept
    {
      std::ifstream ifs{ path.c_str(), std::ios::binary };

      // if file successfully loaded
      if (ifs.is_open())
      {
        auto FILE_SIZE = std::filesystem::file_size(path);

        // Allocate a container the size of file size being read
        std::vector<uint8_t> binaryData(FILE_SIZE);

        // Read the contents from the file
        ifs.read(reinterpret_cast<char*>(binaryData.data()), FILE_SIZE);

        // Proceed to read contents chunk by chunk and save it into new font object.
        auto newFont = UnpackFromBinary<PointerType>(binaryData.data());

        // Return the new font object
        if constexpr (std::is_same_v<PointerType, Font*>)
          return newFont;
        else
          return std::move (newFont);
      }
      else
      {
        std::cout << "FontLoader::ReadAndUnpackFileData: Could not load file: " << path.string() << std::endl;
        return nullptr;
      }
    }

    template <typename PointerType, typename = std::enable_if_t<
      std::is_same_v<PointerType, Font*> ||
      std::is_same_v<PointerType, std::shared_ptr<Font>> ||
      std::is_same_v<PointerType, std::unique_ptr<Font>>>>
      static PointerType UnpackFromBinary(uint8_t const* binaryData) noexcept
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
        std::memcpy(&numGlyphs, binaryData + memoryCursor, sizeof(numGlyphs));
        memoryCursor += sizeof(numGlyphs);

        // Get glyph indexing data 
        for (uint32_t i = 0; i < numGlyphs; ++i)
        {
          GlyphIndexingData indexingData{};
          std::memcpy(&indexingData, binaryData + memoryCursor, sizeof(GlyphIndexingData));
          memoryCursor += sizeof(GlyphIndexingData);

          unpackedFontData.glyphMappings.emplace(indexingData.glyph, indexingData.containerIndex);
        }

        // Get glyph data (since it's all stored contiguously, 1 single memcpy will suffice)
        uint32_t const GLYPH_DATA_BYTES = sizeof(GlyphData) * numGlyphs;
        unpackedFontData.glyphData.resize(numGlyphs);
        std::memcpy(unpackedFontData.glyphData.data(), binaryData + memoryCursor, GLYPH_DATA_BYTES);
        memoryCursor += GLYPH_DATA_BYTES;

        // Get the bitmap's size in bytes
        uint32_t bitmapBytes{};
        std::memcpy(&bitmapBytes, binaryData + memoryCursor, sizeof(bitmapBytes));
        memoryCursor += sizeof(bitmapBytes);

        // Get the bitmap's dimensions
        std::memcpy(&unpackedFontData.bitmapWidth, binaryData + memoryCursor, sizeof(unpackedFontData.bitmapWidth));
        memoryCursor += sizeof(unpackedFontData.bitmapWidth);
        std::memcpy(&unpackedFontData.bitmapHeight, binaryData + memoryCursor, sizeof(unpackedFontData.bitmapHeight));
        memoryCursor += sizeof(unpackedFontData.bitmapHeight);

        // Get the actual bitmap data
        unpackedFontData.fontBitmap.resize(bitmapBytes);
        std::memcpy(unpackedFontData.fontBitmap.data(), binaryData + memoryCursor, bitmapBytes);
        memoryCursor += bitmapBytes;

        // Get number of kern pairs
        uint32_t numKernPairs{};
        std::memcpy(&numKernPairs, binaryData + memoryCursor, sizeof(numKernPairs));
        memoryCursor += sizeof(numKernPairs);

        // Get kern pair data and place into map one by one
        for (uint32_t i = 0; i < numKernPairs; ++i)
        {
          PerKernPair kernPairData{};
          std::memcpy(&kernPairData, binaryData + memoryCursor, sizeof(PerKernPair));
          unpackedFontData.kernPairs.emplace(std::pair{ kernPairData.lhs, kernPairData.rhs }, kernPairData.kerning);
          memoryCursor += sizeof(PerKernPair);
        }
      }

      
      // Move the unpacked font data into new font object
      PointerType newFont{nullptr};
      
      if constexpr (std::is_same_v<PointerType, Font*>)
      {
        newFont = new Font{ std::move(unpackedFontData) };
        return newFont;
      }
      else if constexpr (std::is_same_v<PointerType, std::unique_ptr<Font>>)
      {
        newFont = std::make_unique<Font>(std::move(unpackedFontData));
        return std::move (newFont);
      }
      else // shared ptr
      {
        newFont = std::make_shared<Font>(std::move(unpackedFontData));
        return std::move(newFont);
      }

    }
  };


}