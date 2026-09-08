/**
 * @file Fullscreen triangle vertex shader for the sky pass.
 */

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD0;
};

PixelInput VSMain(uint vertexId : SV_VertexID)
{
    const float2 positions[3] = {
        float2(-1.0F, -1.0F), float2(-1.0F, 3.0F), float2(3.0F, -1.0F)
    };
    const float2 coordinates[3] = {
        float2(0.0F, 1.0F), float2(0.0F, -1.0F), float2(2.0F, 1.0F)
    };
    PixelInput output;
    output.position = float4(positions[vertexId], 1.0F, 1.0F);
    output.textureCoordinate = coordinates[vertexId];
    return output;
}
