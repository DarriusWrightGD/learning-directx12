#include "DirectXDemo.h"



DirectXDemo::DirectXDemo(HINSTANCE instanceHandle) : DirectXWindow(instanceHandle)
{
}


DirectXDemo::~DirectXDemo()
{
}

void DirectXDemo::Init()
{
}

void DirectXDemo::Update(const GameTimer & timer)
{
}

void DirectXDemo::Draw(const GameTimer & timer)
{
	ThrowIfFailed(commandListAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(),nullptr));

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	commandList->RSSetViewports(1, &viewPort);
	commandList->RSSetScissorRects(1, &scissorRect);
	
	float cosTime = cosf(timer.TotalTime());
	float sinTime = sinf(timer.TotalTime());
	float color[4] = { 0.6f * cosTime,0.8f * sinTime,0.4f,1.0f };
	commandList->ClearRenderTargetView(
		CurrentBackBufferView(),
		color, 0, nullptr);
	commandList->ClearDepthStencilView(DepthBufferStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr);
	commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthBufferStencilView());
	commandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
			)
		);
	ThrowIfFailed(commandList->Close());

	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(swapChain->Present(0, 0));
	currentBackBuffer = (currentBackBuffer + 1) % swapChainBufferCount;

	FlushCommandQueue();
}
