#pragma once
#include <DirectXMath.h>
#include <Util/DxUtil.h>
#include <Geometry/MeshGeometry.h>

struct RenderItem
{
	RenderItem(int numberOfFrameResorces) : NumberOfFramesDirty(numberOfFrameResorces){}

	DirectX::XMFLOAT4X4 World;
	//if the world position, or something else gets changes about an
	//object, then we need to update this to the number of frameResources
	//so that each frame resource can be updated to the correct result.
	int NumberOfFramesDirty;
	UINT ObjectCBIndex = -1;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};
