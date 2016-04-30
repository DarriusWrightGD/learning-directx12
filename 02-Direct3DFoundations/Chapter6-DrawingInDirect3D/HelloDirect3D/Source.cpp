#include "HelloRender.h"
#include <memory>

using std::unique_ptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR pCmdLine, int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		unique_ptr<DirectXWindow> window(new HelloRender(hInstance));
		window->Initialize();
		return window->Run();
	}
	catch (DxException & e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"Failed", MB_OK);
		return 0;
	}
}