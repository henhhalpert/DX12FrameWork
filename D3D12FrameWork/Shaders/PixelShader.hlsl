#include "RootSignature.hlsl"

[RootSignature(ROOTSIG)] // bind root sig. 
float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}