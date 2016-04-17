#pragma once
#include "DirectXWindow.h"
class DirectXDemo : public DirectXWindow
{
public:
	DirectXDemo(HINSTANCE instanceHandle);
	virtual ~DirectXDemo();

protected:
	virtual void Init() override;
	virtual void Update(const GameTimer & timer) override;
	virtual void Draw(const GameTimer & timer) override;
};

