#pragma once
#include <DirectXMath.h>
#include <Util/DxUtil.h>
#include <Geometry/MeshGeometry.h>

struct RenderItem
{
	RenderItem(int numberOfFrameResorces) : NumberOfFramesDirty(numberOfFrameResorces){}

	DirectX::XMFLOAT4X4 World;
	int NumberOfFramesDirty;
	UINT ObjectCBIndex = -1;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};
