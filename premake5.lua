local sep = "/"
local ext = ""
if os.ishost("windows") then
	sep = "\\"
	ext = ".exe"
end

-- Workspace definition.

workspace("GL_Template")
	-- Configurations
	configurations({ "Debug", "Release"})
	location("build")
	targetdir ("build/%{prj.name}/%{cfg.longname}")
	debugdir ("build/%{prj.name}/%{cfg.longname}")
	architecture("x64")

	-- Configuration specific settings.
	filter("configurations:Debug")
		defines({ "DEBUG" })
		symbols("On")
	filter("configurations:Release")
		defines({ "NDEBUG" })
		optimize("On")
	filter({})


-- Helper functions for the projects.

function InstallProject(projectName, destination)
	filter("configurations:Debug")
		postbuildcommands({
			path.translate( "{CHDIR} "..os.getcwd(), sep),
			path.translate( "{COPY} build/"..projectName.."/Debug/"..projectName..ext.." "..destination, sep)
		})
	filter("configurations:Release")
		postbuildcommands({
			path.translate( "{CHDIR} "..os.getcwd(), sep),
			path.translate( "{COPY} build/"..projectName.."/Release/"..projectName..ext.." "..destination, sep)
		})
	filter({})
end

function CPPSetup()
	language("C++")
	buildoptions({ "-std=c++11","-Wall" })
end	

function GraphicsSetup()
	CPPSetup()

	-- To support angled brackets in Xcode.
	sysincludedirs({ "src/libs/", "src/libs/glfw/include/" })

	-- Libraries for each platform.
	if os.istarget("macosx") then
		libdirs({"src/libs/glfw/lib-mac/"})
		links({"glfw3", "OpenGL.framework", "Cocoa.framework", "IOKit.framework", "CoreVideo.framework"})
	elseif os.istarget("windows") then
		libdirs({"src/libs/glfw/lib-win-vc2015-64/"})
		links({"glfw3", "opengl32"})
	else -- Assume linux
		-- Libraries needed: OpenGL and glfw3.  glfw3 require X11, Xi, and so on...	
		libdirs({ os.findlib("glfw3") })
		links({"glfw3", "GL", "X11", "Xi", "Xrandr", "Xxf86vm", "Xinerama", "Xcursor", "rt", "m", "pthread", "dl"})
	end

end


function ShaderValidation()
	-- Run the shader validator on all existing shaders.
	-- Output IDE compatible error messages.
	dependson({"ShaderValidator"})
	prebuildcommands({ 
		-- Move to the build directory.
		path.translate("{CHDIR} "..os.getcwd().."/build", sep),
		-- Run the shader validator on the resources directory.
		path.translate( "./shader_validator"..ext.." ../resources", sep)
	})
end	

function AppSetup()
	GraphicsSetup()
	includedirs({ "src/engine" })
	links({"Engine"})
	kind("ConsoleApp")
	ShaderValidation()
end	

function ToolSetup()
	GraphicsSetup()
	includedirs({ "src/engine" })
	links({"Engine"})
	kind("ConsoleApp")
	--location("build/tools")
end	

-- Projects

project("Engine")
	GraphicsSetup()
	kind("StaticLib")
	files({ "src/engine/**.hpp", "src/engine/**.cpp",
			"resources/shaders/**.vert", "resources/shaders/**.frag",
			"src/libs/*/*.hpp", "src/libs/*/*.cpp", "src/libs/*/*.h"
	})


group("Apps")

project("PBRDemo")
	AppSetup()
	files({ "src/apps/pbrdemo/**.hpp", "src/apps/pbrdemo/**.cpp", })

project("Playground")
	AppSetup()
	files({ "src/apps/playground/**.hpp", "src/apps/playground/**.cpp", })

project("Atmosphere")
	AppSetup()
	files({ "src/apps/atmosphere/**.hpp", "src/apps/atmosphere/**.cpp", })


group("Tools")

project("AtmosphericScatteringEstimator")
	ToolSetup()
	files({ "src/tools/AtmosphericScatteringEstimator.cpp" })

project("BRDFEstimator")
	ToolSetup()
	includedirs({ "src/apps/pbrdemo" })
	files({ "src/tools/BRDFEstimator.cpp" })

project("SHExtractor")
	ToolSetup()	
	files({ "src/tools/SHExtractor.cpp" })

project("ShaderValidator")
	ToolSetup()	
	files({ "src/tools/ShaderValidator.cpp" })
	-- Install the shader validition utility in the root build directory.
	InstallProject("%{prj.name}", "build/shader_validator"..ext)
	filter({})


group("Meta")

project("ALL")
	CPPSetup()
	kind("ConsoleApp")
	dependson( {"Engine", "PBRDemo", "Playground", "Atmosphere", "AtmosphericScatteringEstimator", "BRDFEstimator", "SHExtractor" })

-- Actions

newaction {
   trigger     = "clean",
   description = "Clean the build directory",
   execute     = function ()
      print("Cleaning...")
      os.rmdir("./build")
      print("Done.")
   end
}

-- TODO: package resources automatically.
	