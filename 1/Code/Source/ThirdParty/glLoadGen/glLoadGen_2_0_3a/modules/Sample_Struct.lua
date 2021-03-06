local struct = require "Structure"
local common = require "CommonStruct"

local my_struct = 
{
	{ type="file", style="hdr", name="GetFilename(basename, spec, options)",
		{ type="block", name="IncludeGuard",
			{ type="write", name="Guards(hFile, spec, options)", },
			{ type="blank" },
			{ type="write", name="Typedefs(hFile, specData, spec, options)",},
			{ type="blank" },
			{ type="block", name="Extern(hFile)",
				{ type="ext-iter",
					{ type="write", name="Extension(hFile, extName, spec, options)", },
				},
				{ type="blank" },
				common.Enumerators(),
				{ type="blank" },
				common.Functions(),
				{ type="blank" },
				{ type="write", name="MainLoaderFunc(hFile, spec, options)",},
			},
		},
	},
	{ type="file", style="src", name="GetFilename(basename, spec, options)",
		{ type="write", name="Includes(hFile, basename, spec, options)", },
		{ type="blank" },
		{ type="write", name="LoaderFunc(hFile, spec, options)", },
		{ type="blank" },
		{ type="ext-iter",
			{ type="write", name="Extension(hFile, extName, spec, options)", },
		},
		{ type="blank" },
		common.Functions(),
		{ type="ext-iter",
			{ type="block", name="ExtFuncLoader(hFile, extName, spec, options)", cond="func-iter",
				{ type="func-iter",
					{ type="write", name="LoadFunction(hFile, func, spec, options)", },
				},
			},
			{ type="blank", cond="func-iter",},
		},
		{ type="block", name="CoreLoader(hFile, spec, options)", cond="core-funcs",
			{ type="version-iter",
				{type="func-iter",
					{ type="write", name="LoadFunction(hFile, func, spec, options)", },
				},
			},
		},
		{ type="blank", },
		{ type="write", name="MainLoaderFunc(hFile, specData, spec, options)",},
	},
}

my_struct = struct.BuildStructure(my_struct)
return my_struct
