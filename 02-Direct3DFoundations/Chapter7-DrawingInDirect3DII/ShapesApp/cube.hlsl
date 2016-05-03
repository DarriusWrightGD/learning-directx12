cbuffer cbPerObject : register(b0)
{
	float4x4 world;
};

cbuffer cbPass : register(b1)
{
	float4x4 view;
	float4x4 projection;
	float4x4 invView;
	float4x4 invProjection;
	float4x4 viewProjection;
	float4x4 invViewProjection;
	float3 eyePosition;
	float pad1;
	float2 renderTargetSize;
	float2 invRenderTargetSize;
	float nearZ;
	float farZ;
	float totalTime;
	float deltaTime;
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

	float4x4 wvp = mul(world,viewProjection);
	vertexOut.position= mul(float4(vertexIn.position,1.0f), wvp);
	vertexOut.color = float4(vertexIn.color,1.0f);

	return vertexOut;
}

float4 PS(VertexOut pixelShaderIn) : SV_TARGET
{
	return pixelShaderIn.color;
}