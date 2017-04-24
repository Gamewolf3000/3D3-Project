/*PASS THROUGH SHADER DELUXE!*/

float4 main(uint index : SV_VertexID) : SV_POSITION
{
	float4 pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
	switch (index)
	{
		case 0:
			pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 1:
			pos = float4(1.f, -1.0f, 0.0f, 1.0f);
			break;
		case 2:
			pos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
			break;
		case 3:
			pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 4:
			pos = float4(1.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 5:
			pos = float4(1.f, -1.0f, 0.0f, 1.0f);
			break;
	}
	return pos;
}