/******************************************************************************
 * \file    SHAssetMacros.h
 * \author  Loh Xiao Qi
 * \brief   Macros and typedefs for assets
 * 
 * \copyright	Copyright (c) 2022 Digipen Institute of Technology. Reproduction
 *						or disclosure of this file or its contents without the prior
 *						written consent of Digipen Institute of Technology is prohibited
 ******************************************************************************/
#ifndef SH_ASSET_MACROS_H
#define SH_ASSET_MACROS_H

#include <cstdint>
#include <string>
#include <filesystem>

 // Typedefs
using AssetPath = std::filesystem::path;

//Directory
constexpr std::string_view ASSET_ROOT {"Fonts"};

// ASSET EXTENSIONS
constexpr std::string_view FONT_EXTENSION{ ".dash_font" };

// EXTERNAL EXTENSIONS
constexpr std::string_view TTF_EXTENSION{ ".ttf" };

constexpr std::string_view EXTERNALS[] = {
  TTF_EXTENSION
};

#endif // !SH_ASSET_MACROS_H
