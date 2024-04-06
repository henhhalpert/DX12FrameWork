#include "RootSignature.hlsl"

float3 color : register(b0);

[RootSignature(ROOTSIG)] // bind root sig. 
float4 main() : SV_TARGET
{
	return float4(color, 1.0f);
}