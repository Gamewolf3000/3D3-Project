/*Finish the Rasterization*/
struct PSOutput
{
    float4 normal : SV_TARGET0;
    float4 colour : SV_TARGET1;
    float4 worldPos : SV_TARGET2;
};

Texture2D sampleTexture : register(t0);
sampler samplerState : register(s0);

struct vertexData
{
    float4 pos : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORDS;
};

PSOutput main(in vertexData data) : SV_Target
{
    PSOutput outPut;
    outPut.colour = float4(sampleTexture.Sample(samplerState, data.uv).xyz, 1.0f);
    outPut.normal = data.normal;
    outPut.worldPos = data.pos;
    return outPut;
}