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

float3 EvaluateLight(
    float3 normal, float3 viewDirection, float3 lightDirection,
    float3 lightColor, float intensity, float3 baseColor)
{
    const float diffuse = saturate(dot(normal, lightDirection)) * MaterialParameters.z;
    const float3 halfDirection = normalize(lightDirection + viewDirection);
    const float specular = pow(saturate(dot(normal, halfDirection)), MaterialParameters.y) *
        MaterialParameters.x;
    return (baseColor * diffuse + SpecularColor.rgb * specular) * lightColor * intensity;
}

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
    const float3 viewDirection = normalize(CameraPosition.xyz - input.worldPosition);
    const float4 sampledColor = BaseColorTexture.Sample(BaseColorSampler, input.textureCoordinate);
    const float3 surfaceColor = sampledColor.rgb * BaseColor.rgb;
    if (MaterialParameters.w > 0.5F && MaterialParameters.w < 1.5F)
    {
        output.color = float4(saturate(surfaceColor), sampledColor.a * BaseColor.a);
        return output;
    }
    float3 result = surfaceColor * AmbientColor.rgb;

    if (DirectionalColorAndEnabled.w > 0.5F)
    {
        result += EvaluateLight(
            normal, viewDirection, normalize(-DirectionalDirectionAndIntensity.xyz),
            DirectionalColorAndEnabled.rgb, DirectionalDirectionAndIntensity.w, surfaceColor);
    }

    [unroll]
    for (uint lightIndex = 0; lightIndex < 4; ++lightIndex)
    {
        const float4 positionAndRange = PointLightData[lightIndex * 2];
        const float4 colorAndIntensity = PointLightData[lightIndex * 2 + 1];
        const float3 offset = positionAndRange.xyz - input.worldPosition;
        const float distanceToLight = length(offset);
        const float attenuation = pow(saturate(1.0F - distanceToLight / positionAndRange.w), 2.0F);
        if (colorAndIntensity.w > 0.0F && attenuation > 0.0F)
        {
            result += EvaluateLight(
                normal, viewDirection, offset / max(distanceToLight, 0.0001F),
                colorAndIntensity.rgb, colorAndIntensity.w * attenuation, surfaceColor);
        }
    }
    output.color = float4(saturate(result), sampledColor.a * BaseColor.a);
    return output;
}
