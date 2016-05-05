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
	ENGINE_SHARED std::wstring ToString() const;
	ENGINE_SHARED HRESULT GetErrorCode() const;
	ENGINE_SHARED std::wstring GetFunctionName() const;
	ENGINE_SHARED std::wstring GetFileName() const;
	ENGINE_SHARED int GetLineNumber() const;
private:
	HRESULT mErrorCode = S_OK;
	std::wstring mFunctionName;
	std::wstring mFileName;
	int mLineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
	HRESULT hr__ = (x); \
	std::wstring filename = AnsiToWString(__FILE__); \
	if(FAILED(hr__)) {throw DxException(hr__, L#x, filename, __LINE__); } \
}
#endif