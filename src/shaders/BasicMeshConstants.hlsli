/**
 * @file Shared b0 layout; changes rebuild both BasicMesh vertex and pixel shaders.
 */

#ifndef LRENDER_BASIC_MESH_CONSTANTS_HLSLI
#define LRENDER_BASIC_MESH_CONSTANTS_HLSLI

cbuffer BasicMeshConstants : register(b0)
{
    row_major float4x4 WorldViewProjection;
    row_major float4x4 World;
    row_major float4x4 WorldInverseTranspose;
    float4 BaseColor;
    float4 CameraPosition;
    float4 AmbientColor;
    float4 DirectionalDirectionAndIntensity;
    float4 DirectionalColorAndEnabled;
    float4 PointLightData[8];
    float4 SpecularColor;
    float4 MaterialParameters;
};

#endif
