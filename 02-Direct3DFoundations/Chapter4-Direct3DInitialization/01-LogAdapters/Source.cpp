#include <Windows.h>
#include <d3d12.h>
#include <iostream>
#include <vector>
#include <dxgi1_4.h>
#include <string>

using std::cout;
using std::wcout;
using std::endl;
using std::vector;
using std::wstring;

void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM)
{
	UINT count = 0;
	UINT flags = 0;

	output->GetDisplayModeList(format, flags, &count, nullptr);
	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);
	for (auto& mode : modeList)
	{
		UINT n = mode.RefreshRate.Numerator;
		UINT d = mode.RefreshRate.Denominator;

		wstring text =
			L"Width = " + std::to_wstring(mode.Width) + L", "+
			L"Height = " + std::to_wstring(mode.Height) + L", " +
			L"Refresh Rate = " + std::to_wstring(n) + L"/" + std::to_wstring(d) + L"";
		wcout << text << endl;
	}
	wcout << "\n" << endl;

}

void LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	//monitors...
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		//hey monitor tell me something about yourself
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"Output: ";
		text += desc.DeviceName;
		text += L"\n";

		wcout << text.c_str() << endl;
		LogOutputDisplayModes(output);

		output->Release();
		output = nullptr;

		i++;
	}
}

void main()
{
	//allows you to generate DXGI objects? at least that is what the documentation says.
	//also not sure why the fourth version works but not the first 
	IDXGIFactory4* factory = nullptr;
	HRESULT hr= CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	if (FAILED(hr))
	{
		cout << "Failed" << endl;
		exit(1);
	}

	//this will contain about the cards that can render 
	IDXGIAdapter* adapter = nullptr;
	UINT i = 0;
	vector<IDXGIAdapter*> adapterList;



	while (factory->EnumAdapters(i,&adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		wstring text = L"Adapter: ";
		text += desc.Description;
		text += L"\n";

		wcout << text.c_str() << endl;
		LogAdapterOutputs(adapter);

		adapterList.push_back(adapter);
		i++;
	}

	for (auto adapter : adapterList)
	{
		adapter->Release();
		adapter = nullptr;
	}

	factory->Release();
	factory = nullptr;
	system("pause");
}