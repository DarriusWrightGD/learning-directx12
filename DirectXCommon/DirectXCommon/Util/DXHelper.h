#pragma once
#include <Windows.h>
#include <string>
#include <exception>
#include <Export.h>

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class DxException
{
public:
	ENGINE_SHARED DxException() = default;
	ENGINE_SHARED DxException(HRESULT hr, const std::wstring & functionName, const std::wstring & filename, int lineNumber);
	ENGINE_SHARED std::wstring ToString();
	ENGINE_SHARED HRESULT GetErrorCode();
	ENGINE_SHARED std::wstring GetFunctionName();
	ENGINE_SHARED std::wstring GetFileName();
	ENGINE_SHARED int GetLineNumber();
private:
	HRESULT errorCode = S_OK;
	std::wstring functionName;
	std::wstring fileName;
	int lineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
	HRESULT hr__ = (x); \
	std::wstring filename = AnsiToWString(__FILE__); \
	if(FAILED(hr__)) {throw DxException(hr__, L#x, filename, __LINE__); } \
}
#endif