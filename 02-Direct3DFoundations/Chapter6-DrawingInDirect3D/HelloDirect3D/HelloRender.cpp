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
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&view, XMMatrixIdentity());
	XMStoreFloat4x4(&projection, XMMatrixIdentity());
}


HelloRender::~HelloRender()
{
}

//check
void HelloRender::Init()
{
	ThrowIfFailed(commandListAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(), nullptr));

	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();


	//close the command list to execute the commands
	ThrowIfFailed(commandList->Close());

	//execute the commands from the command list
	ID3D12CommandList * commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	// make sure the command queue is done
	FlushCommandQueue();

}

void HelloRender::BuildDescriptorHeaps()
{
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.NodeMask = 0;
	cbvHeapDesc.NumDescriptors = 1;

	device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap));
}


void HelloRender::BuildConstantBuffers()
{
	//this will supply our shader with the wvp matrix

	//create an upload buffer
	objectConstantsCB = std::make_unique<UploadBuffer<ObjectConstants>>(device.Get(), 1, true);
	
	//get a size that is a multiple of 256
	auto objectCBSize = DxUtil::GetConstantBufferPadding(sizeof(ObjectConstants));
	auto cbAddress = objectConstantsCB->Resource()->GetGPUVirtualAddress();

	auto boxCBIndex = 0;
	cbAddress += boxCBIndex * objectCBSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = objectCBSize;

	//create the contstant buffer
	device->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());
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

	ThrowIfFailed(device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void HelloRender::BuildShadersAndInputLayout()
{
	//look into using the cso files for better performance
	mVsByteCode = DxUtil::CompileShader(L"cube.hlsl", nullptr, "VS", "vs_5_0");
	mPsByteCode = DxUtil::CompileShader(L"cube.hlsl", nullptr, "PS", "ps_5_0");

	for (auto i = 0; i < _countof(BasicVertexDescription); i++)
	{
		inputLayout.push_back(BasicVertexDescription[i]);
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

	boxGeo = std::make_unique<MeshGeometry>();
	boxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &boxGeo->VertexBufferCPU));
	CopyMemory(boxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);


	ThrowIfFailed(D3DCreateBlob(ibByteSize, &boxGeo->IndexBufferCPU));
	CopyMemory(boxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//how to create a buffer for vertices
	boxGeo->VertexBufferGPU = DxUtil::CreateDefaultBuffer(device.Get(), commandList.Get(), vertices.data(), vbByteSize, boxGeo->VertexBufferUploader);


	boxGeo->IndexBufferGPU = DxUtil::CreateDefaultBuffer(device.Get(), commandList.Get(), indices.data(), ibByteSize, boxGeo->IndexBufferUploader);


	boxGeo->VertexBufferByteSize = vbByteSize;
	boxGeo->VertexByteStride = sizeof(BasicVertex);
	boxGeo->IndexBufferByteSize = ibByteSize;
	boxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;

	//to bind this data to the pipeline we will need a vertex buffer view 

	SubmeshGeometry submesh;
	submesh.IndexCount = static_cast<UINT>(indices.size());
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	boxGeo->drawArgs["box"] = submesh;
}

void HelloRender::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	//what does zero memory do that {} cannot?
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };
	psoDesc.pRootSignature = rootSignature.Get();
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
	psoDesc.RTVFormats[0] = backbufferFormat;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = depthStencilFormat;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

//check
void HelloRender::Update(const GameTimer & timer)
{
	float x = radius * sinf(phi) * cosf(theta);
	float z = radius * sinf(phi) * sinf(theta);
	float y = radius * cosf(phi);

	//float x = 0;
	//float y = 0;
	//float z = -3;

	XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX v = XMMatrixLookAtLH(position, target, up);
	XMStoreFloat4x4(&view, v);

	XMMATRIX w = XMLoadFloat4x4(&world);
	XMMATRIX p = XMLoadFloat4x4(&projection);

	XMMATRIX wvpMatrix = w * v * p;

	ObjectConstants objectConstants;
	//XMStoreFloat4x4(&objectConstants.worldViewProjection, XMMatrixTranspose(wvpMatrix));
	XMStoreFloat4x4(&objectConstants.worldViewProjection, XMMatrixTranspose(wvpMatrix));
	objectConstants.time = timer.TotalTime();
	objectConstantsCB->CopyData(0, objectConstants);
}

//check
void HelloRender::Draw(const GameTimer & timer)
{
	//be sure to reset the commandlist and allocator first before starting to store commands again
	ThrowIfFailed(commandListAllocator->Reset());
	//be sure that if you want to actually render something that you use the pipeline state object here.
	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(), pso.Get()));

	//go from presenting the to rendering

	//set the view port and scissor rects
	commandList->RSSetViewports(1, &viewPort);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


	float color[4] = { 0.1f,0.4f,0.7f * cosf(timer.TotalTime()),1.0f };
	//clear the color and depth buffer
	commandList->ClearRenderTargetView(CurrentBackBufferView(), color, 0, nullptr);
	commandList->ClearDepthStencilView(DepthBufferStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//set the render targets
	commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthBufferStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { cbvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	commandList->IASetVertexBuffers(0, 1, &boxGeo->VertexBufferView());
	commandList->IASetIndexBuffer(&boxGeo->IndexBufferView());
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());

	//auto& subMesh = boxGeo->drawArgs["box"];
	commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	// switch from rendering to presenting
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//close the command list to execute the commands
	ThrowIfFailed(commandList->Close());

	//execute the commands from the command list
	ID3D12CommandList * commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	//present the back buffer
	ThrowIfFailed(swapChain->Present(0, 0));
	currentBackBuffer = (currentBackBuffer + 1) % swapChainBufferCount;

	// make sure the command queue is done
	FlushCommandQueue();
}

void HelloRender::OnMouseMove(WPARAM state, int x, int y)
{
	if ((state & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - lastMousePosition.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - lastMousePosition.y));

		theta += dx;
		phi += dy;


		phi = clamp(phi, 0.1f, 3.14f);
	}
	else if ((state & MK_RBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - lastMousePosition.x);
		float dy = 0.005f * static_cast<float>(y - lastMousePosition.y);

		radius += dx - dy;
		radius = clamp(radius, 3.0f, 15.0f);
	}

	lastMousePosition.x = static_cast<float>(x);
	lastMousePosition.y = static_cast<float>(y);
}

void HelloRender::OnMouseDown(WPARAM state, int x, int y)
{
	lastMousePosition.x = x;
	lastMousePosition.y = y;

	SetCapture(mainWindowHandle);
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
	XMStoreFloat4x4(&projection, p);
}



