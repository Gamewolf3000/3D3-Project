RWTexture2D<float> map : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	map[threadID.xy] = 1;
	int a = 0;
	a++;
}