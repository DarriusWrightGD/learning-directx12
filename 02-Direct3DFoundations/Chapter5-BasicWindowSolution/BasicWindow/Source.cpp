#include "DirectXDemo.h"
#include <DirectXColors.h>
#include <memory>

using std::unique_ptr;

using namespace Microsoft::WRL;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		std::unique_ptr<DirectXWindow> window(new DirectXDemo(hInstance));
		if (!window->Initialize())
		{
			return -1;
		}

		return window->Run();
	}
	catch (std::exception & e)
	{
		MessageBox(nullptr, L"There was an issue", L"Failed", MB_OK);
		return 0;
	}
}