#include "HelloRender.h"
#include <Data\Vertex.h>
#include <Util\DxUtil.h>
#include <math.h>
#include <Util\HelperMath.h>
#include <array>
#include <d3dcompiler.h>

//BasicVertex vertices[] =
//{
//	{ XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(1.0f,1.0f,-1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(-1.0f,1.0f,1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(1.0f,1.0f,1.0f), XMFLOAT3(0.6f,0.3f,0.7f) },
//	{ XMFLOAT3(1.0f,-1.0f,1.0f), XMFLOAT3(0.6f,0.3f,0.7f) }
//};
//
//UINT verticesSize = sizeof(BasicVertex) * 8;
//
//std::int16_t indices[] = {
//	0,1,2,
//	0,2,3,
//
//	4,6,5,
//	4,7,6,
//
//	4,5,1,
//	4,1,0,
//
//	3,2,6,
//	3,6,7,
//
//	1,5,6,
//	1,6,2,
//
//	4,0,3,
//	4,3,7
//};
//
//UINT indicesSize = sizeof(std::int16_t) * 36;

HelloRender::HelloRender(HINSTANCE instanceHandle) : DirectXWindow(instanceHandle)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
	XMStoreFloat4x4(&mView, XMMatrixIdentity());
	XMStoreFloat4x4(&mProjection, XMMatrixIdentity());
}


HelloRender::~HelloRender()
{
}

//check
void HelloRender::Init()
{
	ThrowIfFailed(mCommandListAllocator->Reset());
	ThrowIfFailed(mCommandList->Reset(mCommandListAllocator.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();


	//close the command list to execute the commands
	ThrowIfFailed(mCommandList->Close());

	//execute the commands from the command list
	ID3D12CommandList * commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	// make sure the command queue is done
	FlushCommandQueue();

}

void HelloRender::BuildDescriptorHeaps()
{
	mCbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	mCbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	mCbvHeapDesc.NodeMask = 0;
	mCbvHeapDesc.NumDescriptors = 1;

	mDevice->CreateDescriptorHeap(&mCbvHeapDesc, IID_PPV_ARGS(&mCbvHeap));
}


void HelloRender::BuildConstantBuffers()
{
	//this will supply our shader with the wvp matrix

	//create an upload buffer
	mObjectConstantsCB = std::make_unique<UploadBuffer<ObjectConstants>>(mDevice.Get(), 1, true);
	
	//get a size that is a multiple of 256
	auto objectCBSize = DxUtil::GetConstantBufferPadding(sizeof(ObjectConstants));
	auto cbAddress = mObjectConstantsCB->Resource()->GetGPUVirtualAddress();

	auto boxCBIndex = 0;
	cbAddress += boxCBIndex * objectCBSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objectCBSize;

	//create the contstant buffer
	mDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void HelloRender::BuildRootSignature()
{
	/*
	The root signature defines the resources that the shaders require.
	The root signature can be thought of as defining the function signature.
	*/

	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	auto hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	ThrowIfFailed(hr);

	ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}

void HelloRender::BuildShadersAndInputLayout()
{
	//look into using the cso files for better performance
	mVsByteCode = DxUtil::CompileShader(L"cube.hlsl", nullptr, "VS", "vs_5_0");
	mPsByteCode = DxUtil::CompileShader(L"cube.hlsl", nullptr, "PS", "ps_5_0");

	for (auto i = 0; i < _countof(BasicVertexDescription); i++)
	{
		mInputLayout.push_back(BasicVertexDescription[i]);
	}
}

void HelloRender::BuildBoxGeometry()
{
	std::array<BasicVertex, 8> vertices =
	{
		BasicVertex({ XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT3(0.6f,0.5f,0.2f) }),
		BasicVertex({ XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT3(0.1f,0.15f,0.1f) }),
		BasicVertex({ XMFLOAT3(1.0f,1.0f,-1.0f), XMFLOAT3(0.6f,0.3f,0.7f) }),
		BasicVertex({ XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT3(0.1f,0.1f,0.7f) }),
		BasicVertex({ XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(0.6f,0.3f,0.1f) }),
		BasicVertex({ XMFLOAT3(-1.0f,1.0f,1.0f), XMFLOAT3(0.3f,0.3f,0.3f) }),
		BasicVertex({ XMFLOAT3(1.0f,1.0f,1.0f), XMFLOAT3(0.2f,0.5f,0.1f) }),
		BasicVertex({ XMFLOAT3(1.0f,-1.0f,1.0f), XMFLOAT3(0.8f,0.1f,0.2f) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		0,1,2,
		0,2,3,

		4,6,5,
		4,7,6,

		4,5,1,
		4,1,0,

		3,2,6,
		3,6,7,

		1,5,6,
		1,6,2,

		4,0,3,
		4,3,7
	};

	const auto vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(BasicVertex);
	const auto ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "mBoxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);


	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//how to create a buffer for vertices
	mBoxGeo->VertexBufferGPU = DxUtil::CreateDefaultBuffer(mDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);


	mBoxGeo->IndexBufferGPU = DxUtil::CreateDefaultBuffer(mDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);


	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->VertexByteStride = sizeof(BasicVertex);
	mBoxGeo->IndexBufferByteSize = ibByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;

	//to bind this data to the pipeline we will need a vertex buffer view 

	SubmeshGeometry submesh;
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->drawArgs["box"] = submesh;
}

void HelloRender::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	//what does zero memory do that {} cannot?
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), static_cast<UINT>(mInputLayout.size()) };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mVsByteCode->GetBufferPointer()),
		mVsByteCode->GetBufferSize()
	};

	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mPsByteCode->GetBufferPointer()),
		mPsByteCode->GetBufferSize()
	};

	auto rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPso)));
}

//check
void HelloRender::Update(const GameTimer & timer)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	//float x = 0;
	//float y = 0;
	//float z = -3;

	XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX v = XMMatrixLookAtLH(position, target, up);
	XMStoreFloat4x4(&mView, v);

	XMMATRIX w = XMLoadFloat4x4(&mWorld);
	XMMATRIX p = XMLoadFloat4x4(&mProjection);

	XMMATRIX wvpMatrix = w * v * p;

	ObjectConstants objectConstants;
	//XMStoreFloat4x4(&objectConstants.worldViewProjection, XMMatrixTranspose(wvpMatrix));
	XMStoreFloat4x4(&objectConstants.WorldViewProjection, XMMatrixTranspose(wvpMatrix));
	objectConstants.Time = timer.TotalTime();
	mObjectConstantsCB->CopyData(0, objectConstants);
}

//check
void HelloRender::Draw(const GameTimer & timer)
{
	//be sure to reset the commandlist and allocator first before starting to store commands again
	ThrowIfFailed(mCommandListAllocator->Reset());
	//be sure that if you want to actually render something that you use the pipeline state object here.
	ThrowIfFailed(mCommandList->Reset(mCommandListAllocator.Get(), mPso.Get()));

	//go from presenting the to rendering

	//set the view port and scissor rects
	mCommandList->RSSetViewports(1, &mViewPort);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


	float color[4] = { 0.1f,0.4f,0.7f * cosf(timer.TotalTime()),1.0f };
	//clear the color and depth buffer
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), color, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//set the render targets
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	//auto& subMesh = mBoxGeo->drawArgs["box"];
	mCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	// switch from rendering to presenting
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//close the command list to execute the commands
	ThrowIfFailed(mCommandList->Close());

	//execute the commands from the command list
	ID3D12CommandList * commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	//present the back buffer
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % SwapChainBufferCount;

	// make sure the command queue is done
	FlushCommandQueue();
}

void HelloRender::OnMouseMove(WPARAM state, int x, int y)
{
	if ((state & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePosition.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePosition.y));

		mTheta += dx;
		mPhi += dy;


		mPhi = clamp(mPhi, 0.1f, 3.14f);
	}
	else if ((state & MK_RBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - mLastMousePosition.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePosition.y);

		mRadius += dx - dy;
		mRadius = clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePosition.x = static_cast<float>(x);
	mLastMousePosition.y = static_cast<float>(y);
}

void HelloRender::OnMouseDown(WPARAM state, int x, int y)
{
	mLastMousePosition.x = x;
	mLastMousePosition.y = y;

	SetCapture(mMainWindowHandle);
}

void HelloRender::OnMouseUp(WPARAM state, int x, int y)
{
	ReleaseCapture();
}

void HelloRender::OnResize()
{
	DirectXWindow::OnResize();
	auto aspect = AspectRatio();
	XMMATRIX p = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), aspect, 1.0f, 100.0f);
	XMStoreFloat4x4(&mProjection, p);
}



