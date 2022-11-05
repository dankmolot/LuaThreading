PROJECT_GENERATOR_VERSION = 3

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	value = "../../garrysmod_common"
})

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")

include(gmcommon)
include("../premake5.lua")

CreateWorkspace({name = "LuaThreadingTest", path = "projects/" .. os.target() .. "/" .. _ACTION})
	CreateProject({serverside = true, manual_files = true})
		IncludeLuaShared()
        IncludeLuaThreading()
        IncludeSDKCommon()
        IncludeSDKTier0()

        files { "main.cpp" }
