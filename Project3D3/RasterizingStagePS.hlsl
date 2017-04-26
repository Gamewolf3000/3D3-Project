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
	float4 wPos : WORLDPOS;
	float4 normal : NORMAL;
	float2 uv : TEXCOORDS;
};

PSOutput main(in vertexData data) : SV_Target
{
	PSOutput outPut;
	outPut.colour = float4(sampleTexture.Sample(samplerState, data.uv).xyz, 1.0f);
	outPut.normal = data.normal;
	outPut.worldPos = data.wPos;
	bool test = outPut.worldPos != 0;
	bool test2 = outPut.worldPos != 0;
	bool test3 = outPut.worldPos != 0;

	for (int i = 0; i < 2000; i++)
	{
		test = float4(sampleTexture.Sample(samplerState, (data.uv + float2(i *0.000002, i *0.000002))).xyz, 1.0f) + test3;
		test2 = float4(sampleTexture.Sample(samplerState, (data.uv + float2(i *0.000002, i *0.000002))).xyz, 1.0f) + test2;
		test3 = float4(sampleTexture.Sample(samplerState, (data.uv + float2(i*0.000002, i*0.000002))).xyz, 1.0f) + test;
		outPut.worldPos = test + test2 + test3;
	}
	if (test && test2 && test3)
		outPut.worldPos = data.wPos;
	return outPut;
}