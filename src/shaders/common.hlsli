/**
 * @file Shared Shader ABI used by every render effect.
 */

#ifndef LRENDER_COMMON_HLSLI
#define LRENDER_COMMON_HLSLI

cbuffer FrameInfo : register(b0)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float4 Mode;
    float4 CameraPosition;
    float4 Viewport;
};

cbuffer ObjectInfo : register(b1)
{
    row_major float4x4 WorldViewProjection;
    row_major float4x4 World;
    row_major float4x4 WorldInverseTranspose;
};

cbuffer MaterialInfo : register(b2)
{
    float4 BaseColor;
    float4 SpecularColor;
    float4 MaterialParameters;
};

cbuffer LightInfo : register(b3)
{
    float4 AmbientColor;
    float4 DirectionalDirectionAndIntensity;
    float4 DirectionalColorAndEnabled;
    float4 PointLightData[8];
};

#endif
