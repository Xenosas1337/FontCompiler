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
typedef std::filesystem::path AssetPath;


//Directory
#ifdef _PUBLISH
constexpr std::string_view ASSET_ROOT{ "Assets" };
constexpr std::string_view BUILT_IN_ASSET_ROOT {"Built_In"};
#else
constexpr std::string_view ASSET_ROOT {"../../Assets"};
constexpr std::string_view BUILT_IN_ASSET_ROOT{ "../../Built_In" };
#endif

// ASSET EXTENSIONS
constexpr std::string_view FONT_EXTENSION{ ".shfont" };

// EXTERNAL EXTENSIONS
constexpr std::string_view TTF_EXTENSION{ ".ttf" };

constexpr std::string_view EXTERNALS[] = {
  TTF_EXTENSION
};

#endif // !SH_ASSET_MACROS_H
