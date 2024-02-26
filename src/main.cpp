#include "FontCompiler.hpp"

#include <vector>
#include <filesystem>
#include <iostream>
#include "FontLoader.hpp"

int main(int argc, char* argv[])
{
  msdfgen::FreetypeHandle* freetypeHandle = msdfgen::initializeFreetype();

  std::vector<std::string> paths;

  if (argc == 1)
  {
    if (std::filesystem::is_directory(dash_tools::ASSET_ROOT))
    {
      for (auto& dir :
        std::filesystem::recursive_directory_iterator{ dash_tools::ASSET_ROOT })
      {
        if (dir.path().extension().string() == dash_tools::TTF_EXTENSION)
        {
          auto path = dir.path();
          path.make_preferred();
          paths.push_back(path.string());
        }
      }
    }
    else
    {
      std::cout << "Default path not found!" << std::endl;
      return 1;
    }
  }
  else if (argc > 1)
  {
    for (int i{ 1 }; i < argc; ++i)
    {
      paths.emplace_back(argv[i]);
    }
  }

  for (auto const& path : paths)
  {
    dash_tools::FontCompiler::LoadAndCompileFont(freetypeHandle, path);
  }

  //SH_COMP::FontCompiler::LoadAndCompileFont(freetypeHandle, "test_font/SegoeUI.ttf");
  msdfgen::deinitializeFreetype(freetypeHandle);

  dash_tools::AssetPath dashFontPath = "Fonts/times.dash_font";

  //auto newFont = dash_tools::FontLoader::ReadAndUnpackFileData<std::unique_ptr<dash_tools::Font>>(dashFontPath);
  auto newFont = dash_tools::FontLoader::ReadAndUnpackFileData<dash_tools::Font*>(dashFontPath);
  (void)newFont;

  return 0;
}
