/*PASS THROUGH SHADER DELUXE!*/

float4 main(uint index : SV_VertexID) : SV_POSITION0
{
	
	float4 pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
	switch (index)
	{
	case 0:
		pos.xy = float2(-1.0f, 1.0f);
		break;
	case 1:
		pos.xy = float2(1.f, -1.0f);
		break;
	case 2:
		pos.xy = float2(-1.0f, -1.0f);
		break;
	case 3:
		pos.xy = float2(-1.0f, 1.0f);
		break;
	case 4:
		pos.xy = float2(1.0f, 1.0f);
		break;
	case 5:
		pos.xy = float2(1.f, -1.0f);
		break;
	}
	return pos;
}