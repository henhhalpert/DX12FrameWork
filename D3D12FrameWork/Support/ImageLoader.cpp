#include "ImageLoader.h"

const std::vector<ImageLoader::GUID_to_dxgi> ImageLoader::s_lookupTable =
{
	{ GUID_WICPixelFormat32bppBGRA,  DXGI_FORMAT_B8G8R8A8_UNORM},
	{ GUID_WICPixelFormat32bppRGBA,  DXGI_FORMAT_R8G8B8A8_UNORM}
};

bool ImageLoader::LoadImageFromDisk(const std::filesystem::path& imgPath, ImageData& data)
{
	// WIC factory 
	ComPointer<IWICImagingFactory> wicFactory;
	__ImageLoader__CAR(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory)));

	// Load image 
	ComPointer<IWICStream> wicFileStream;
	__ImageLoader__CAR(wicFactory->CreateStream(&wicFileStream));
	__ImageLoader__CAR(wicFileStream->InitializeFromFilename(imgPath.wstring().c_str(), GENERIC_READ));

	ComPointer<IWICBitmapDecoder> wicDecoder;
	__ImageLoader__CAR(wicFactory->CreateDecoderFromStream(wicFileStream, nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder));

	ComPointer<IWICBitmapFrameDecode> wicFrameDecoder;
	__ImageLoader__CAR(wicDecoder->GetFrame(0, &wicFrameDecoder));

	// Get image metadata 
	__ImageLoader__CAR(wicFrameDecoder->GetSize(&data.width, &data.height));

	__ImageLoader__CAR(wicFrameDecoder->GetPixelFormat(&data.winPixelFormat));

	// Metadata of pixel format
	ComPointer<IWICComponentInfo> wicComponentInfo;
	__ImageLoader__CAR(wicFactory->CreateComponentInfo(data.winPixelFormat, &wicComponentInfo));

	ComPointer<IWICPixelFormatInfo> wicPixelFormat;
	__ImageLoader__CAR(wicComponentInfo->QueryInterface(&wicPixelFormat));

	__ImageLoader__CAR(wicPixelFormat->GetBitsPerPixel(&data.bpp));
	__ImageLoader__CAR(wicPixelFormat->GetChannelCount(&data.cc));

	// DXGI pixel format - using lookup table
	// search for an element in s_lookupTable
	auto findIt = std::find_if(s_lookupTable.begin(), s_lookupTable.end(),
		[&](const GUID_to_dxgi& entry) // all data within lambda accessed by ref for the purpose of finding a valid entry. 
		{
			return memcmp(&entry.wic, &data.winPixelFormat, sizeof(GUID)) == 0;
		}
	);

	if (findIt == s_lookupTable.end())
	{
		return false;
	}
	data.giPixelFormat = findIt->gi;

	// Image loading
	WICRect rect{};
	uint_fast32_t stride = ((data.bpp + 7) / 8) * data.width; // # of bytes per row
	uint_fast32_t bufSize = stride * data.height;				  // total number of bytes per image 
	data.data.resize(bufSize);
	WICRect copyRect;
	copyRect.Height = data.height;
	copyRect.Width = data.width;

	__ImageLoader__CAR(wicFrameDecoder->CopyPixels(&copyRect, stride, bufSize, (BYTE*)data.data.data()));
}
