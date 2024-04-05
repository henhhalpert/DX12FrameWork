#include "RootSignature.hlsl"

// bind arg to semantic name which then binds itself to System-value semantic (SV_*)
[RootSignature(ROOTSIG)] // bind root sig. 
float4 main(float2 pos : Position) : SV_Position
{
	// convert float2 to float4 
    return float4(pos.xy, 0.0f, 1.0f);
}