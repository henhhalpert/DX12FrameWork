#pragma once

#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>

#define __ImageLoader__CAR(expr)	 \
do									 \
{									 \
	if(FAILED(expr)) {return false;} \
} while(false)

class ImageLoader
{
	public:
		struct ImageData
		{
			// metadata 
			std::vector<char> data;
			uint_fast32_t width;
			uint_fast32_t height;
			uint_fast32_t bpp; // bits per pixel - how many a single pixel has 
			uint_fast32_t cc;  // channel count - how many channel are

			//! unique signature that can be mapped to actual pixel format
			//! (using GUID_to_dxgi struct below for conversion)
			GUID		winPixelFormat; 
			DXGI_FORMAT giPixelFormat;  

		};

		static bool LoadImageFromDisk(const std::filesystem::path& imgPath, ImageData& data);
	private:
		struct GUID_to_dxgi
		{
			GUID wic;
			DXGI_FORMAT gi;
		};
		static const std::vector<GUID_to_dxgi> s_lookupTable;

	private:
		ImageLoader() = default;
		ImageLoader(const ImageLoader&) = default;
		ImageLoader& operator=(const ImageLoader&) = default;
};

