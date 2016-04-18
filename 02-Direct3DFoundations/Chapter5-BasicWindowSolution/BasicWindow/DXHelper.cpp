#include "DXHelper.h"
#include <comdef.h>

DxException::DxException(HRESULT hr, const std::wstring & functionName, const std::wstring & fileName, int lineNumber) :
	fileName(fileName), functionName(functionName), lineNumber(lineNumber)
{
}

std::wstring DxException::ToString()
{
	_com_error err(GetErrorCode());
	std::wstring errorMessage = err.ErrorMessage();
	return GetFunctionName() + L" Failed in " + GetFileName() + L" on line " + std::to_wstring(GetLineNumber()) + L" with the error: " + errorMessage;
}

HRESULT DxException::GetErrorCode()
{
	return errorCode;
}

std::wstring DxException::GetFunctionName()
{
	return functionName;
}

std::wstring DxException::GetFileName()
{
	return fileName;
}

int DxException::GetLineNumber()
{
	return lineNumber;
}
