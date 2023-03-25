MSDFInclude = "%{prj.location}\\Dependencies\\msdf" 
MSDFGenInclude = "%{prj.location}\\Dependencies\\msdf\\msdfgen" 

-- outputdir = "%{wks.location}/bin/%{cfg.buildcfg}"
-- interdir = "%{wks.location}/bin_int"
-- workspace "fontcompile"
-- architecture "x64"
  -- configurations
  -- {
    -- "release",
    -- "debug"
  -- }

  -- group "Dependencies"
	-- include "Dependencies/msdf"
  -- group ""


project "FontCompiler"
  kind "ConsoleApp"
  language "C++"
  cppdialect "C++20"
  targetdir (outputdir)
  objdir    (interdir)
  systemversion "latest"
  
  
  -- include "Dependencies/msdf"

  files  
  {
    "%{prj.location}/src/**.h",
    "%{prj.location}/src/**.cpp",
  }

  externalincludedirs
  {
	  "%{MSDFInclude}",
	  "%{MSDFGenInclude}"
  }
  
  includedirs
  {
    "%{prj.location}/src",
  }
  
  links
  {
    "msdfgen",
    "msdf-atlas-gen"
}
  
  dependson
  {
    "msdfgen",
    "msdf-atlas-gen",
  }
  
  externalwarnings "Off"

  flags
  {
  	"MultiProcessorCompile"
  }

  warnings 'Extra'
  
  filter "configurations:Debug"
    symbols "On"
    defines {"_DEBUG"}
  
  filter "configurations:Release"
    optimize "On"
    defines{"_RELEASE"}
  
    filter "configurations:Publish"
      flags {"ExcludeFromBuild"}