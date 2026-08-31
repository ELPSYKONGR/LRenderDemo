/**
 * @file Passthrough color processor used to validate the post-process path.
 * @depends QuadViewVS output, common.hlsli
 */

#include "common.hlsli"

Texture2D SceneColorTexture : register(t0);
SamplerState SceneColorSampler : register(s0);

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD0;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    return SceneColorTexture.Sample(SceneColorSampler, input.textureCoordinate);
}
