function file_exists(name)
	local f=io.open(name,"r")
	if f~=nil then io.close(f) return true else return false end
end

-- TODO: do this smarter
GetBasePath = function ()
	if file_exists("/usr/include/nodejs/deps/v8/include/v8.h") then
		return "/usr/include/nodejs/deps/v8"
	end
	return PathDir(ModuleFilename())
end

V8 = {
	basepath = GetBasePath(),

	OptFind = function (name, required)
		local check = function(option, settings)
			option.value = false
			option.use_winlib = 0
			option.lib_path = nil

			if platform == "win32" then
				option.value = true
				option.use_winlib = 32
			elseif platform == "win64" then
				option.value = true
				option.use_winlib = 64
			end
		end

		local apply = function(option, settings)
			if option.use_winlib > 0 then
				settings.cc.includes:Add(V8.basepath .. "/include")
				if option.use_winlib == 32 then
					settings.link.libpath:Add(V8.basepath .. "/windows/lib32")
				else
					settings.link.libpath:Add(V8.basepath .. "/windows/lib64")
				end

				settings.link.libs:Add("v8.dll")
				settings.link.libs:Add("v8_libbase.dll")
				--settings.link.libs:Add("icui18n.dll")
				--settings.link.libs:Add("icuuc.dll")
				settings.link.libs:Add("v8_libplatform.dll")
				settings.link.libs:Add("zlib.dll")
			else
				settings.link.flags:Add("-lv8")
				settings.cc.includes:Add(V8.basepath .. "/include")
			end
		end

		local save = function(option, output)
			output:option(option, "value")
			output:option(option, "use_winlib")
		end

		local display = function(option)
			if option.value == true then
				if option.use_winlib == 32 then return "using supplied win32 libraries" end
				if option.use_winlib == 64 then return "using supplied win64 libraries" end
				return "using unknown method (black magic)"
			else
				if option.required then
					return "not found (required)"
				else
					return "not found (optional)"
				end
			end
		end

		local o = MakeOption(name, 0, check, save, display)
		o.Apply = apply
		o.include_path = nil
		o.lib_path = nil
		o.required = required
		return o
	end
}
