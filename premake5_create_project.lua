project "LuaThreading"
    kind "StaticLib"
    location("projects/" .. os.target() .. "/" .. _ACTION)
    
    targetdir "%{prj.location}/%{cfg.architecture}/%{cfg.buildcfg}"
    debugdir "%{prj.location}/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "!%{prj.location}/%{cfg.architecture}/%{cfg.buildcfg}/intermediate/%{prj.name}"

    files {
        "src/**"
    }

    sysincludedirs(_GARRYSMOD_COMMON_DIRECTORY .. "/include")

    links "lua_shared"

    filter "system:linux or macosx"
        links "dl"