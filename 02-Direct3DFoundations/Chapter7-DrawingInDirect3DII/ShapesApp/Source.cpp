#include "ShapesDemo.h"
#include <memory>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR pCmdLine, int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		auto window = std::make_unique<ShapesDemo>(hInstance);
		window->Initialize();
		return window->Run();
	}
	catch(DxException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"Failed", MB_OK);
		return 0;
	}
}