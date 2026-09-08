/**
 * @file TextureCube sampling pixel shader for the sky pass.
 */
#include "common.hlsli"

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD0;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    float2 clipPosition = float2(
        input.textureCoordinate.x * 2.0F - 1.0F,
        1.0F - input.textureCoordinate.y * 2.0F);
    float4 worldPosition = mul(float4(clipPosition, 1.0F, 1.0F), C_InverseViewProjection);
    float3 direction = normalize(worldPosition.xyz / worldPosition.w - C_CameraPosition.xyz);
    return C_SkyCubeTexture.Sample(C_SkyCubeSampler, direction);
}
