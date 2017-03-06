
/*No input data as of now!*/

float4 main(uint index : SV_VertexID ) : SV_POSITION
{
    float4 pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
    switch(index)
    {
        case 0:
            pos = float4(0.0f, 2.0f, 0.0f, 1.0f);
            break;
        case 1:
            pos = float4(2.0f, -1.0f, 0.0f, 1.0f);
            break;
        case 2:
            pos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
            break;
    }


	return pos;
}