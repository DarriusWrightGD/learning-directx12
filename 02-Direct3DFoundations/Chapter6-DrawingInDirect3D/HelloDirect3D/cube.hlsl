cbuffer cbPerObject : register(b0)
{
	float4x4 worldViewProj;
	float time;
};

struct VertexIn
{
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VertexOut VS(VertexIn vertexIn)
{
	VertexOut vertexOut;
	vertexIn.position.x = vertexIn.position.x + sin(time);
	vertexIn.position.y = vertexIn.position.y + cos(time);
	vertexOut.position= mul(float4(vertexIn.position,1.0f), worldViewProj);
	//vertexOut.position= float4(vertexIn.position,1.0f);
	vertexOut.color = float4(vertexIn.color,1.0f);

	return vertexOut;
}

float4 PS(VertexOut pixelShaderIn) : SV_TARGET
{
	return pixelShaderIn.color;
}