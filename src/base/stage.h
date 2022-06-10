#pragma once

#include "window.h"
#include "input.h"

class Stage {
public:
	Stage() = default;

	virtual ~Stage() = default;

	virtual void init(Window& window, MouseInput& mouseInput) = 0;

	virtual void deinit() = 0;

	virtual void handleInput(
		Window& window,
		KeyboardInput& keyboardInput, MouseInput& mouseInput, 
		float deltaTime) = 0;

	virtual void renderFrame() = 0;
};
