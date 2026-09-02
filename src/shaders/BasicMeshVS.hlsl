/**
 * @file Basic mesh vertex shader.
 * @depends MeshVertex, common.hlsli
 */

#include "common.hlsli"

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 textureCoordinate : TEXCOORD0;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL;
    float2 textureCoordinate : TEXCOORD0;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(float4(input.position, 1.0F), C_WorldViewProjection);
    output.worldPosition = mul(float4(input.position, 1.0F), C_World).xyz;
    output.worldNormal = mul(float4(input.normal, 0.0F), C_WorldInverseTranspose).xyz;
    output.textureCoordinate = input.textureCoordinate;
    return output;
}
