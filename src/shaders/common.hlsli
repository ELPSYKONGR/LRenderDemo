/**
 * @file Shared Shader ABI used by every render effect.
 */

#ifndef LRENDER_COMMON_HLSLI
#define LRENDER_COMMON_HLSLI

cbuffer FrameInfo : register(b0)
{
    row_major float4x4 C_View;
    row_major float4x4 C_Projection;
    float4 C_Mode; //X:material.displayMode Y:frame.renderMode
    float4 C_CameraPosition;
    float4 C_Viewport;
};

cbuffer ObjectInfo : register(b1)
{
    row_major float4x4 C_WorldViewProjection;
    row_major float4x4 C_World;
    row_major float4x4 C_WorldInverseTranspose;
};

cbuffer MaterialInfo : register(b2)
{
    float4 C_BaseColor;
    float4 C_SpecularColor;
    float4 C_MaterialParameters;
};

cbuffer LightInfo : register(b3)
{
    float4 C_AmbientColor;
    float4 C_DirectionalDirectionAndIntensity;
    float4 C_DirectionalColorAndEnabled;
    float4 C_PointLightData[8];
};

bool IfDelayedRenderMode()
{
    return C_Mode.y > 0.5F && C_Mode.y < 1.5F;
}

bool UseTexColor()
{
    return C_MaterialParameters.w > 0.5F && C_MaterialParameters.w < 1.5F;
}

bool UseBaseColorTexture()
{
    return C_Mode.z > 0.5F;
}

float3 EvaluateLight(
    float3 normal, float3 viewDirection, float3 lightDirection,
    float3 lightColor, float intensity, float3 baseColor)
{
    const float diffuse =
        saturate(dot(normal, lightDirection)) * C_MaterialParameters.z;
    const float3 halfDirection = normalize(lightDirection + viewDirection);
    const float specular =
        pow(saturate(dot(normal, halfDirection)), C_MaterialParameters.y) *
        C_MaterialParameters.x;
    return (baseColor * diffuse + C_SpecularColor.rgb * specular) * lightColor * intensity;
}

float4 CalcBlinnPhongLightColor(
    float3 normal,
    float3 surfaceColor,
    float3 viewDirection,
    float3 worldPosition,
    float alpha)
{
    float3 result = surfaceColor * C_AmbientColor.rgb;
    if (UseTexColor())
    {
        result = saturate(surfaceColor);
    }
    else
    {
        if (C_DirectionalColorAndEnabled.w > 0.5F)
        {
            result += EvaluateLight(
                normal,
                viewDirection,
                normalize(-C_DirectionalDirectionAndIntensity.xyz),
                C_DirectionalColorAndEnabled.rgb,
                C_DirectionalDirectionAndIntensity.w,
                surfaceColor);
        }

        [unroll]
        for (uint lightIndex = 0; lightIndex < 4; ++lightIndex)
        {
            const float4 positionAndRange = C_PointLightData[lightIndex * 2];
            const float4 colorAndIntensity = C_PointLightData[lightIndex * 2 + 1];
            const float3 offset = positionAndRange.xyz - worldPosition;
            const float distanceToLight = length(offset);
            const float attenuation =
                pow(saturate(1.0F - distanceToLight / positionAndRange.w), 2.0F);
            if (colorAndIntensity.w > 0.0F && attenuation > 0.0F)
            {
                result += EvaluateLight(
                    normal,
                    viewDirection,
                    offset / max(distanceToLight, 0.0001F),
                    colorAndIntensity.rgb,
                    colorAndIntensity.w * attenuation,
                    surfaceColor);
            }
        }
    }
    return float4(saturate(result), alpha);
}

#endif
