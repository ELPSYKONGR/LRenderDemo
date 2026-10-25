/**
 * @file Neighborhood-clamped temporal accumulation pixel shader.
 */

Texture2D CurrentSceneTexture : register(t0);
Texture2D HistoryTexture : register(t4);
SamplerState TAAInputSampler : register(s0);

cbuffer TAAInfo : register(b4)
{
    float4 C_TAAParameters; // x:历史权重 y:历史有效标志 zw:纹理单像素尺寸
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD0;
};

float3 SampleCurrent(float2 coordinate)
{
    return CurrentSceneTexture.Sample(TAAInputSampler, coordinate).rgb;
}

float4 PSMain(PixelInput input) : SV_TARGET
{
    const float2 texelSize = C_TAAParameters.zw;
    const float3 currentColor = SampleCurrent(input.textureCoordinate);
    float3 minimumColor = currentColor;
    float3 maximumColor = currentColor;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            const float3 neighbor = SampleCurrent(input.textureCoordinate + float2(x, y) * texelSize);
            minimumColor = min(minimumColor, neighbor);
            maximumColor = max(maximumColor, neighbor);
        }
    }

    if (C_TAAParameters.y < 0.5F)
    {
        return float4(currentColor, 1.0F);
    }

    const float3 historyColor = HistoryTexture.Sample(TAAInputSampler, input.textureCoordinate).rgb;
    const float3 clampedHistory = clamp(historyColor, minimumColor, maximumColor);
    return float4(lerp(currentColor, clampedHistory, C_TAAParameters.x), 1.0F);
}
