/**
 * @file Textured multi-light basic mesh pixel shader.
 * @depends BasicMeshVS output, common.hlsli
 */

#include "common.hlsli"

Texture2D BaseColorTexture : register(t0);
SamplerState BaseColorSampler : register(s0);

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL;
    float2 textureCoordinate : TEXCOORD0;
};

struct PixelOutput
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

PixelOutput PSMain(PixelInput input)
{
    const float3 normal = normalize(input.worldNormal);
    PixelOutput output;
    output.normal = float4(normal * 0.5F + 0.5F, 1.0F);
    const float3 viewDirection = normalize(C_CameraPosition.xyz - input.worldPosition);
    const float4 sampledColor = BaseColorTexture.Sample(BaseColorSampler, input.textureCoordinate);
    const bool useBaseColorTexture = C_MaterialParameters.w > 0.5F && C_MaterialParameters.w < 1.5F;
    const float3 surfaceColor = useBaseColorTexture ? sampledColor.rgb : C_BaseColor.rgb;
    const float alpha = useBaseColorTexture ? sampledColor.a : C_BaseColor.a;
    if (IfDelayedRenderMode())
    {
        output.color = float4(surfaceColor, alpha);
    }
    else
    {
        output.color = CalcBlinnPhongLightColor(
            normal,
            surfaceColor,
            viewDirection,
            input.worldPosition,
            alpha);
    }
    return output;
}
