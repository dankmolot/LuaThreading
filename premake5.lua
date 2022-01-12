local current_dir = _SCRIPT_DIR

function IncludeLuaThreading()
	if IncludePackage then
		IncludePackage('retro_lua-threading')
	end

	includedirs(current_dir)
end