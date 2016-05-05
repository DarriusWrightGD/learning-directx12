#include "ShapesDemo.h"
#include <Data/Vertex.h>
#include <array>
#include <d3dcompiler.h>
#include <Util/HelperMath.h>
using namespace DirectX;


ShapesDemo::ShapesDemo(HINSTANCE window) : DirectXWindow(window)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
	XMStoreFloat4x4(&mView, XMMatrixIdentity());
	XMStoreFloat4x4(&mProjection, XMMatrixIdentity());
}


ShapesDemo::~ShapesDemo()
{
}

void ShapesDemo::Init()
{
	//reset the command list before using any commands 
	ThrowIfFailed(mCommandListAllocator->Reset());
	ThrowIfFailed(mCommandList->Reset(mCommandListAllocator.Get(), nullptr));

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeap();
	BuildConstantBuffer();
	BuildPSO();
	//close the command list before executing commands 
	mCommandList->Close();

	//execute the list of commandlists
	ID3D12CommandList * commandLists [] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	//wait until the command queue is empty before proceeding
	FlushCommandQueue();
}

void ShapesDemo::Update(const GameTimer& timer)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX v = XMMatrixLookAtLH(position, target, up);
	XMStoreFloat4x4(&mView, v);

	mCurrentFrameResourceIndex = (mCurrentFrameResourceIndex + 1) % mNumberOfFrameResources;
	mCurrentFrameResource = mFrameResources[mCurrentFrameResourceIndex].get();

	//Check if the GPU has finished processing commands up until this point,
	//if not wait until the GPU has completed commands at this mFence point
	if (mCurrentFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrentFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCBs(timer);
	UpdateMainPassCB(timer);
}

void ShapesDemo::Draw(const GameTimer& tinmer)
{
	auto cmdListAllocator = mCurrentFrameResource->CommandListAllocator;

	ThrowIfFailed(cmdListAllocator->Reset());

	auto * pso = mPSOs["opaque"].Get();

	if(mIsWireFrame)
	{
		pso = mPSOs["opaque_wireframe"].Get();
	}

	ThrowIfFailed(mCommandList->Reset(cmdListAllocator.Get(), pso));

	mCommandList->RSSetViewports(1, &mViewPort);
	mCommandList->RSSetScissorRects(1, &mScissorRect);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	float color[4] = { 0,0.2,0.6,1 };
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), color, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap * descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	int passCbvIndex = mPassOffset + mCurrentFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, mCbvDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

	DrawRenderItems(mCommandList.Get(), mOpaqueRenderItems);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList * commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists),commandLists);

	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % SwapChainBufferCount;

	mCurrentFrameResource->Fence = mCurrentFence++;
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);


}
void ShapesDemo::OnMouseMove(WPARAM state, int x, int y)
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

void ShapesDemo::OnMouseDown(WPARAM state, int x, int y)
{
	mLastMousePosition.x = static_cast<float>(x);
	mLastMousePosition.y = static_cast<float>(y);
	mIsWireFrame = true;

	SetCapture(mMainWindowHandle);
}

void ShapesDemo::OnMouseUp(WPARAM state, int x, int y)
{
	mIsWireFrame = false;
	ReleaseCapture();
}

void ShapesDemo::OnResize()
{
	DirectXWindow::OnResize();
	auto aspect = AspectRatio();
	XMMATRIX p = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), aspect, 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProjection, p);
}

void ShapesDemo::BuildDescriptorHeap()
{
	UINT objectCount = static_cast<UINT>(mOpaqueRenderItems.size());
	UINT numberOfDescriptors = (objectCount + 1) * mNumberOfFrameResources;

	mPassOffset = objectCount * mNumberOfFrameResources;

	//describe the constant buffer descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NodeMask = 0;
	cbvHeapDesc.NumDescriptors = numberOfDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//create it
	mDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap));
}

void ShapesDemo::BuildConstantBuffer()
{
	UINT objectCBByteSize = DxUtil::GetConstantBufferPadding(sizeof(ObjectConstants));
	UINT objectCount = static_cast<UINT>(mOpaqueRenderItems.size());

	for (auto frameIndex = 0; frameIndex < mNumberOfFrameResources; frameIndex++)
	{
		auto objectCB = mFrameResources[frameIndex]->ObjectCB->Resource();
		for (auto i = 0u; i < objectCount; i++)
		{
			auto cbAddress = objectCB->GetGPUVirtualAddress();
			cbAddress += i*objectCBByteSize;

			int heapIndex = frameIndex*objectCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, mCbvDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objectCBByteSize;

			mDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}
	
	UINT passCBByteSize = DxUtil::GetConstantBufferPadding(sizeof(PassConstants));
	for (auto frameIndex = 0; frameIndex < mNumberOfFrameResources; frameIndex++)
	{
		auto passCB = mFrameResources[frameIndex]->PassCB->Resource();

		auto cbAddress = passCB->GetGPUVirtualAddress();

		int heapIndex = mPassOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, mCbvDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;
		mDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void ShapesDemo::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	CD3DX12_DESCRIPTOR_RANGE cbvTable1;

	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	auto hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSignature.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	ThrowIfFailed(hr);
	ThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf())));
	
}

void ShapesDemo::BuildShadersAndInputLayout()
{
	//compile the shaders
	mVsByteCode = DxUtil::CompileShader(L"cube.hlsl", nullptr, "VS", "vs_5_0");
	mPsByteCode = DxUtil::CompileShader(L"cube.hlsl", nullptr, "PS", "ps_5_0");

	//setup the inputLayout
	for (auto i = 0; i < _countof(BasicVertexDescription); i++)
	{
		mInputLayout.push_back(BasicVertexDescription[i]);
	}
}
#include "GeometryGenerator.h"
void ShapesDemo::BuildGeometry()
{
	using namespace Geometry;

	MeshData box = GeometryGenerator::CreateBox(1.5f, 0.5f, 1.5f, 3);
	MeshData grid = GeometryGenerator::CreateGrid(20.0f, 30.0f, 60, 40);
	MeshData sphere = GeometryGenerator::CreateGeoSphere(0.5f, 3);
	MeshData cylinder = GeometryGenerator::CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	shapesMeshData.push_back(box);
	shapesMeshData.push_back(grid);
	shapesMeshData.push_back(sphere);
	shapesMeshData.push_back(cylinder);

	UINT vertexOffset = 0;
	UINT indexOffset = 0;

	for(const auto & meshData : shapesMeshData )
	{
		SubmeshGeometry submesh;
		submesh.IndexCount = meshData.Indices32.size();
		submesh.BaseVertexLocation = vertexOffset;
		submesh.StartIndexLocation = indexOffset;
		shapesSubmesh.push_back(submesh);

		vertexOffset += meshData.Vertices.size();
		indexOffset += meshData.Indices32.size();
	}

	auto totalVertexCount = vertexOffset;
	auto totalIndexCount = indexOffset;

	std::vector<BasicVertex> vertices(totalVertexCount);
	std::vector<uint16> indices;

	UINT vertexCount = 0;
	for(auto & meshData : shapesMeshData)
	{
		float color [] = { random(), random(), random() };
		//float color[] = { 1,0,0 };
 		for (auto i = 0u; i < meshData.Vertices.size(); i++)
		{
			vertices[vertexCount].position = meshData.Vertices[i].Position;
			vertices[vertexCount].color = XMFLOAT3(color[0], color[1], color[2]);
			vertexCount++;
		}

		indices.insert(indices.end(), std::begin(meshData.GetIndices16()), std::end(meshData.GetIndices16()));
	}

	const UINT vertexBufferByteSize = totalVertexCount * sizeof(BasicVertex);
	const UINT indexBufferByteSize = totalIndexCount * sizeof(uint16);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = shapeGeoName;

	ThrowIfFailed(D3DCreateBlob(vertexBufferByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(),vertices.data(), vertexBufferByteSize);

	ThrowIfFailed(D3DCreateBlob(indexBufferByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), indexBufferByteSize);

	geo->VertexBufferGPU = DxUtil::CreateDefaultBuffer(mDevice.Get(), mCommandList.Get(), vertices.data(), vertexBufferByteSize, geo->VertexBufferUploader);
	geo->IndexBufferGPU = DxUtil::CreateDefaultBuffer(mDevice.Get(), mCommandList.Get(), indices.data(), indexBufferByteSize, geo->IndexBufferUploader);

	geo->VertexBufferByteSize = vertexBufferByteSize;
	geo->VertexByteStride = sizeof(BasicVertex);
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = indexBufferByteSize;

	geo->drawArgs["box"] = shapesSubmesh[0];
	geo->drawArgs["grid"] = shapesSubmesh[1];
	geo->drawArgs["sphere"] = shapesSubmesh[2];
	geo->drawArgs["cylinder"] = shapesSubmesh[3];

	mGeometry[geo->Name] = std::move(geo);
}

void ShapesDemo::BuildPSO()
{
	//describe the pipeline state
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
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

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.NodeMask = 0;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DSVFormat = mDepthStencilFormat;
	psoDesc.RTVFormats[0] = mBackBufferFormat;

	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPSOs["opaque"].GetAddressOf())));
	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePso = psoDesc;
	
	wireframePso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	wireframePso.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&wireframePso, IID_PPV_ARGS(mPSOs["opaque_wireframe"].GetAddressOf())));
}

void ShapesDemo::BuildFrameResources()
{
	for (auto i = 0; i < mNumberOfFrameResources; i++)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(mDevice.Get(), 1, static_cast<UINT>(mAllRenderItems.size())));
	}
}

void ShapesDemo::BuildRenderItems()
{
	mRenderItems["box"] = std::make_unique<RenderItem>(mNumberOfFrameResources);
	auto & box = mRenderItems["box"];
	XMStoreFloat4x4(&box->World, XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	box->ObjectCBIndex = 0;
	box->Geo = mGeometry[shapeGeoName].get();
	box->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	box->IndexCount = box->Geo->drawArgs["box"].IndexCount;
	box->StartIndexLocation = box->Geo->drawArgs["box"].StartIndexLocation;
	box->BaseVertexLocation = box->Geo->drawArgs["box"].BaseVertexLocation;
	mAllRenderItems.push_back(std::move(box));

	mRenderItems["grid"] = std::make_unique<RenderItem>(mNumberOfFrameResources);
	auto & grid = mRenderItems["grid"];
	XMStoreFloat4x4(&grid->World, XMMatrixIdentity());
	grid->ObjectCBIndex = 1;
	grid->Geo = mGeometry[shapeGeoName].get();
	grid->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	grid->IndexCount = grid->Geo->drawArgs["grid"].IndexCount;
	grid->StartIndexLocation = grid->Geo->drawArgs["grid"].StartIndexLocation;
	grid->BaseVertexLocation = grid->Geo->drawArgs["grid"].BaseVertexLocation;
	mAllRenderItems.push_back(std::move(grid));


	UINT objCBIndex = 2;
	for (int i = 0; i < 5; ++i)
	{
		auto leftCylRitem = std::make_unique<RenderItem>(mNumberOfFrameResources);
		auto rightCylRitem = std::make_unique<RenderItem>(mNumberOfFrameResources);
		auto leftSphereRitem = std::make_unique<RenderItem>(mNumberOfFrameResources);
		auto rightSphereRitem = std::make_unique<RenderItem>(mNumberOfFrameResources);

		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f);

		XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
		leftCylRitem->ObjectCBIndex = objCBIndex++;
		leftCylRitem->Geo = mGeometry[shapeGeoName].get();
		leftCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRitem->IndexCount = leftCylRitem->Geo->drawArgs["cylinder"].IndexCount;
		leftCylRitem->StartIndexLocation = leftCylRitem->Geo->drawArgs["cylinder"].StartIndexLocation;
		leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->drawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
		rightCylRitem->ObjectCBIndex = objCBIndex++;
		rightCylRitem->Geo = mGeometry[shapeGeoName].get();
		rightCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRitem->IndexCount = rightCylRitem->Geo->drawArgs["cylinder"].IndexCount;
		rightCylRitem->StartIndexLocation = rightCylRitem->Geo->drawArgs["cylinder"].StartIndexLocation;
		rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->drawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
		leftSphereRitem->ObjectCBIndex = objCBIndex++;
		leftSphereRitem->Geo = mGeometry[shapeGeoName].get();
		leftSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRitem->IndexCount = leftSphereRitem->Geo->drawArgs["sphere"].IndexCount;
		leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->drawArgs["sphere"].StartIndexLocation;
		leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->drawArgs["sphere"].BaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
		rightSphereRitem->ObjectCBIndex = objCBIndex++;
		rightSphereRitem->Geo = mGeometry[shapeGeoName].get();
		rightSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRitem->IndexCount = rightSphereRitem->Geo->drawArgs["sphere"].IndexCount;
		rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->drawArgs["sphere"].StartIndexLocation;
		rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->drawArgs["sphere"].BaseVertexLocation;

		mAllRenderItems.push_back(std::move(leftCylRitem));
		mAllRenderItems.push_back(std::move(rightCylRitem));
		mAllRenderItems.push_back(std::move(leftSphereRitem));
		mAllRenderItems.push_back(std::move(rightSphereRitem));
	}

	// All the render items are opaque.
	for (auto& e : mAllRenderItems)
		mOpaqueRenderItems.push_back(e.get());

}

void ShapesDemo::UpdateObjectCBs(const GameTimer & timer)
{
	auto currentObjectCB = mCurrentFrameResource->ObjectCB.get();
	for(auto& renderItem : mAllRenderItems)
	{
		if(renderItem->NumberOfFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&renderItem->World);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

			currentObjectCB->CopyData(renderItem->ObjectCBIndex, objConstants);
			renderItem->NumberOfFramesDirty--;
		}
	}
}

void ShapesDemo::UpdateMainPassCB(const GameTimer& timer)
{
	XMMATRIX v = XMLoadFloat4x4(&mView);
	XMMATRIX p = XMLoadFloat4x4(&mProjection);

	XMMATRIX vp = XMMatrixMultiply(v, p);
	XMMATRIX invV = XMMatrixInverse(&XMMatrixDeterminant(v), v);
	XMMATRIX invP = XMMatrixInverse(&XMMatrixDeterminant(p), p);
	XMMATRIX invVP = XMMatrixInverse(&XMMatrixDeterminant(vp), vp);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(v));
	XMStoreFloat4x4(&mMainPassCB.Projection, XMMatrixTranspose(p));
	XMStoreFloat4x4(&mMainPassCB.ViewProjection, XMMatrixTranspose(vp));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invV));
	XMStoreFloat4x4(&mMainPassCB.InvProjection, XMMatrixTranspose(invP));
	XMStoreFloat4x4(&mMainPassCB.InvViewProjection, XMMatrixTranspose(invVP));

	mMainPassCB.EyePosition = XMFLOAT3(0, 12, -8);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.RenderTargetSize = XMFLOAT2(static_cast<float>(mWidth), static_cast<float>(mHeight));
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / static_cast<float>(mWidth), 1.0f / static_cast<float>(mHeight));
	mMainPassCB.TotalTime = timer.TotalTime();
	mMainPassCB.DeltaTime = timer.DeltaTime();

	auto currentPassCB = mCurrentFrameResource->PassCB.get();
	currentPassCB->CopyData(0, mMainPassCB);
	
}

void ShapesDemo::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& renderItems)
{
	UINT objectCBByteSize = DxUtil::GetConstantBufferPadding(sizeof(ObjectConstants));
	auto objectCB = mCurrentFrameResource->ObjectCB->Resource();

	for (auto i = 0u; i < renderItems.size(); i++)
	{
		auto renderItem = renderItems[i];
		cmdList->IASetVertexBuffers(0, 1, &renderItem->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&renderItem->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(renderItem->PrimitiveType);

		UINT cbvIndex = mCurrentFrameResourceIndex*static_cast<UINT>(mOpaqueRenderItems.size()) + renderItem->ObjectCBIndex;
		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, mCbvDescriptorSize);

		cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		cmdList->DrawIndexedInstanced(renderItem->IndexCount, 1, renderItem->StartIndexLocation, renderItem->BaseVertexLocation, 0);
	}
	
}
