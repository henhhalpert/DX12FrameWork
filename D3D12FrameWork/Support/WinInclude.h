#pragma once

// disable win min max
#define NOMINMAX

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

// dxgi debug layer
#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif