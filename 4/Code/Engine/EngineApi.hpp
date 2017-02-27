#pragma once

#include "Common.hpp"
#include "Base.hpp"
#include "Math.hpp"
#include "Thread.hpp"
#include "File.hpp"
#include "Debug.hpp"
#include "Object.hpp"
#include "Device.hpp"
#include "Graphics.hpp"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "Bullet.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "stb.lib")
#pragma comment(lib, "glslang.lib")

#pragma comment(lib, "opengl32.lib")
#ifdef _WIN32
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "version.lib")
#endif
