#pragma once
#include <d3dx12.h>
#include <d3d12.h>
#include <wrl.h>
#include <Export.h>
#include <string>
#include <d3dcommon.h>

class DxUtil
{
public:
	DxUtil() = default;
	~DxUtil();
	static ENGINE_SHARED Microsoft::WRL::ComPtr<ID3D12Resource> 
		CreateDefaultBuffer(ID3D12Device * device, ID3D12GraphicsCommandList * commandList, const void * data, UINT64 size,
			Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
	static ENGINE_SHARED UINT GetConstantBufferPadding(UINT size) { return (size + 255) & ~255; }
	static ENGINE_SHARED Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target
		);
	static ENGINE_SHARED Microsoft::WRL::ComPtr<ID3DBlob> LoadShaderBinary(const std::wstring& filename);
};

