#include "DXHelper.h"
#include <comdef.h>

DxException::DxException(HRESULT hr, const std::wstring & functionName, const std::wstring & fileName, int lineNumber) :
	mFileName(fileName), mFunctionName(functionName), mLineNumber(lineNumber)
{
}

std::wstring DxException::ToString() const
{
	_com_error err(GetErrorCode());
	std::wstring errorMessage = err.ErrorMessage();
	return GetFunctionName() + L" Failed in " + GetFileName() + L" on line " + std::to_wstring(GetLineNumber()) + L" with the error: " + errorMessage;
}

HRESULT DxException::GetErrorCode() const
{
	return mErrorCode;
}

std::wstring DxException::GetFunctionName() const
{
	return mFunctionName;
}

std::wstring DxException::GetFileName() const
{
	return mFileName;
}

int DxException::GetLineNumber() const
{
	return mLineNumber;
}
