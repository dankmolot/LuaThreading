local current_dir = _SCRIPT_DIR

function IncludeLuaThreading()
	local refcount = IncludePackage("LuaThreading")

	local _project = project()

	links { "LuaThreading", "lua_shared" }
	includedirs(current_dir .. "/src")

	filter "system:linux or macosx"
		links "dl"

	if refcount == 1 then
		dofile(current_dir .. "/premake5_create_project.lua")
		project(_project.name)
	end
end