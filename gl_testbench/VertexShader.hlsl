
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_OUT main(uint id : SV_VertexID) //float4 pos : POSITION ) : SV_POSITION
{
	VS_OUT output;
	output.pos = float4(0, 0, 0, 1);
	output.uv = float2(0,0);

	if (id == 0)
	{
		output.pos = float4(0.0f,  0.05, 0.0f, 1.0f);
		output.uv = float2(0.5f,  -0.99f);
	}
	if (id == 1)
	{
		output.pos = float4(0.05, -0.05, 0.0f, 1.0f);
		output.uv = float2(1.49f, 1.1f);
	}
	if (id == 2)
	{
		output.pos = float4(-0.05, -0.05, 0.0f, 1.0f);
		output.uv = float2(-0.51, 1.1f);
	}

	return output;//float4((id == 2 || id == 3) * 2 - 1, (id == 1 || id == 3) * 2 - 1, 0, 1);
}
