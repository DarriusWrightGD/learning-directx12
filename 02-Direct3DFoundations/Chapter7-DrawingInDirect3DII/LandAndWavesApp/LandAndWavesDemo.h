#pragma once
#include "Window/DirectXWindow.h"

class LandAndWavesDemo : public DirectXWindow
{
public:
	LandAndWavesDemo(HINSTANCE window);
	~LandAndWavesDemo();

protected:
	void Init() override;
	void Update(const GameTimer& timer) override;
	void Draw(const GameTimer& timer) override;
	void OnResize() override;
	void OnMouseUp(WPARAM state, int x, int y) override;
	void OnMouseDown(WPARAM state, int x, int y) override;
	void OnMouseMove(WPARAM state, int x, int y) override;

private:

};

