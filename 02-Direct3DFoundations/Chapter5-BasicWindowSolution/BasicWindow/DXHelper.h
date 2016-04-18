#pragma once
#include <Windows.h>
#include <string>
#include <exception>

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring & functionName, const std::wstring & filename, int lineNumber);
	std::wstring ToString();
	HRESULT GetErrorCode();
	std::wstring GetFunctionName();
	std::wstring GetFileName();
	int GetLineNumber();
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