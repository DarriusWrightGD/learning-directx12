#pragma once
#include <Window\DirectXWindow.h>
#include <DirectXMath.h>
#include <Util\UploadBuffer.h>
#include <Geometry/MeshGeometry.h>
#include "ObjectConstants.h"
#include <memory>
#include <vector>


class HelloRender : public DirectXWindow
{
public:
	HelloRender(HINSTANCE instanceHandle);
	virtual ~HelloRender();

	// Inherited via DirectXWindow
	void Init() override;
	void Update(const GameTimer & timer) override;
	void Draw(const GameTimer & timer) override;
	void OnMouseMove(WPARAM state, int x, int y) override;
	void OnMouseDown(WPARAM state, int x, int y) override;
	void OnMouseUp(WPARAM state, int x, int y) override;
	void OnResize() override;
private:
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
 

	D3D12_DESCRIPTOR_HEAP_DESC mCbvHeapDesc;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectConstantsCB = nullptr;
	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	ComPtr<ID3D12PipelineState> mPso = nullptr;

	DirectX::XMFLOAT2 mLastMousePosition;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mProjection;
	ComPtr<ID3DBlob> mVsByteCode = nullptr;
	ComPtr<ID3DBlob> mPsByteCode = nullptr;

	float mTheta = 1.5f * DirectX::XM_PI;
	float mPhi = DirectX::XM_PIDIV4;
	float mRadius = 15.0f;
};

