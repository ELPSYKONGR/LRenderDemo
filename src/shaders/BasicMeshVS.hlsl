/**
 * @file Basic mesh vertex shader.
 * @depends MeshVertex, BasicMeshEffect constant buffer
 */

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
    output.position = mul(float4(input.position, 1.0F), WorldViewProjection);
    output.worldPosition = mul(float4(input.position, 1.0F), World).xyz;
    output.worldNormal = mul(float4(input.normal, 0.0F), WorldInverseTranspose).xyz;
    output.textureCoordinate = input.textureCoordinate;
    return output;
}
