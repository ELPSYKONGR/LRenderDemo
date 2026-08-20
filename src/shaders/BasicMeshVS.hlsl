/**
 * @file Basic mesh vertex shader.
 * @depends VertexPositionNormalColor, BasicMeshEffect constant buffer
 */

cbuffer BasicMeshConstants : register(b0)
{
    row_major float4x4 WorldViewProjection;
    row_major float4x4 WorldInverseTranspose;
    float4 DiffuseColor;
    float4 LightDirection;
    float4 LightColor;
    float4 AmbientColor;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR0;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float4 color : COLOR0;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(float4(input.position, 1.0F), WorldViewProjection);
    output.worldNormal = mul(float4(input.normal, 0.0F), WorldInverseTranspose).xyz;
    output.color = input.color;
    return output;
}
