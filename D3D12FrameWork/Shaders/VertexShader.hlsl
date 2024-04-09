#include "RootSignature.hlsl"

// bind arg to semantic name which then binds itself to System-value semantic (SV_*)
[RootSignature(ROOTSIG)] // bind root sig. 
void main( 
           // Input
           in float2 pos    : Position,
           in float2 uv     : TexCoord,

           // Output
           out float2 o_uv  : Texcoord,
           out float4 o_pos : SV_Position
)
{
    o_pos = float4(pos.xy, 0.0f, 1.0f);
    o_uv = uv;
}