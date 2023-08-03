// 2023 08 02 이정모 home

// 색을 입히기 위한 간단한 Vertex Shader

cbuffer ConstantBuffer: register(b0)
{
	matrix World;
	matrix View;
	matrix Proj;
}

struct VS_INPUT
{
	float4 Pos: POSITION;
	float4 Color: COLOR;
};

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float4 Color: COLOR;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	output.Pos = mul(input.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Proj);

	output.Color = input.Color;

	return output;
}