#pragma once
#include <Window/DirectXWindow.h>
#include <Util/UploadBuffer.h>
#include <Geometry/MeshGeometry.h>
#include <Util/DxUtil.h>
#include <memory>
#include "ObjectConstants.h"
#include "FrameResource.h"
#include "RenderItem.h"
#include "GeometryGenerator.h"
#include <map>

class ShapesDemo : public DirectXWindow
{
public:
	ShapesDemo(HINSTANCE window);
	virtual ~ShapesDemo();

	void Init() override;
	void Update(const GameTimer & timer) override;
	void Draw(const GameTimer & tinmer) override;
	void OnMouseMove(WPARAM state, int x, int y) override;
	void OnMouseUp(WPARAM state, int x, int y) override;
	void OnMouseDown(WPARAM state, int x, int y) override;
	void OnResize() override;

private:
	void BuildDescriptorHeap();
	void BuildFrameResources();
	void BuildConstantBuffer();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSO();
	void BuildRenderItems();
	void UpdateObjectCBs(const GameTimer & timer);
	void UpdateMainPassCB(const GameTimer & timer);
	void DrawRenderItems(ID3D12GraphicsCommandList * commandList, const std::vector<RenderItem*>& renderItems);


	//Frame resource 
	static const int mNumberOfFrameResources = 5;
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrentFrameResource = nullptr;
	int mCurrentFrameResourceIndex = 0;
	std::vector<std::unique_ptr<RenderItem>> mAllRenderItems;
	std::vector<RenderItem*> mOpaqueRenderItems;
	std::vector<RenderItem*> mTransparentRenderItems;
	PassConstants mMainPassCB;
	bool mIsWireFrame = false;

	//Constant Buffer variables
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	//root signature stuff
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	//Shader variables
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, std::unique_ptr<RenderItem>> mRenderItems;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectConstantsCB = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	ComPtr<ID3D12PipelineState> mPso = nullptr;
	ComPtr<ID3DBlob> mVsByteCode = nullptr;
	ComPtr<ID3DBlob> mPsByteCode = nullptr;

	//Geometry variables
	std::map<std::string, std::unique_ptr<MeshGeometry>> mGeometry;
	std::vector<Geometry::MeshData> shapesMeshData;
	std::vector<SubmeshGeometry> shapesSubmesh;
	std::string shapeGeoName = "Shapes Geo";

	//Transform variables 
	DirectX::XMFLOAT2 mLastMousePosition;
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProjection;

	float mTheta = 1.5f* DirectX::XM_PI;
	float mPhi = 0.2f* DirectX::XM_PI;
	float mRadius = 15.0f;
	UINT mPassOffset;
};

