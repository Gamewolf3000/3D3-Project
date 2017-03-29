RWTexture2D<float> map : register(u0);
Texture2D<float> depth : register(t0);

[numthreads(16, 16, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	map[threadID.xy] = 1;
	int a = 0;
	a++;

	//map[threadID.xy] = (1 - length(threadID.xy - float2(640, 360)) / 640.0f);
	map[threadID.xy] = depth[threadID.xy];
}