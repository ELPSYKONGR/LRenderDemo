/**
 * @file Basic mesh directional-light pixel shader.
 * @depends BasicMeshVS output, BasicMeshEffect constant buffer
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

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
    float4 color : COLOR0;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    const float3 normal = normalize(input.worldNormal);
    const float diffuseIntensity = saturate(dot(normal, LightDirection.xyz));
    const float3 lighting = AmbientColor.rgb + LightColor.rgb * diffuseIntensity;
    const float3 surfaceColor = input.color.rgb * DiffuseColor.rgb;
    return float4(saturate(surfaceColor * lighting), input.color.a * DiffuseColor.a);
}
