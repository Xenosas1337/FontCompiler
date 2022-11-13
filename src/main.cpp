#include "FontCompiler.h"

#include <vector>
#include <filesystem>
#include <iostream>

int main(int argc, char* argv[])
{
  msdfgen::FreetypeHandle* freetypeHandle = msdfgen::initializeFreetype();

  //std::vector<std::string> paths;

  //if (argc == 1)
  //{
  //  if (std::filesystem::is_directory(ASSET_ROOT))
  //  {
  //    for (auto& dir :
  //      std::filesystem::recursive_directory_iterator{ ASSET_ROOT })
  //    {
  //      if (dir.path().extension().string() == TTF_EXTENSION)
  //      {
  //        auto path = dir.path();
  //        path.make_preferred();
  //        paths.push_back(path.string());
  //      }
  //    }
  //  }
  //  else
  //  {
  //    std::cout << "Default path not found!" << std::endl;
  //    return 1;
  //  }
  //}
  //else if (argc > 1)
  //{
  //  for (int i{ 1 }; i < argc; ++i)
  //  {
  //    paths.emplace_back(argv[i]);
  //  }
  //}

  //for (auto const& path : paths)
  {
    SH_COMP::FontCompiler::LoadAndCompileFont(freetypeHandle, "test_font/SegoeUI.ttf");
  }

  msdfgen::deinitializeFreetype(freetypeHandle);

  return 0;
}
