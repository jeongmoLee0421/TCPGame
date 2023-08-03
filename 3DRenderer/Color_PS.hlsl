// 2023 08 02 이정모 home

// 색을 입히기 위한 간단한 Pixel Shader

struct PS_INPUT
{
	float4 Pos: SV_POSITION;
	float4 Color: COLOR;
};

float4 PS(PS_INPUT input) : SV_Target
{
	return input.Color;
}