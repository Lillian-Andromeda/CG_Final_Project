#pragma once

#include <chrono>
#include <string>
#include <stdexcept>
#include <memory>

#include <glm/glm.hpp>

#include "./base/window.h"
#include "./base/input.h"
#include "./base/frame_rate_indicator.h"
#include "./base/stage.h"

struct Options {
	std::string windowTitle;
	int windowWidth;
	int windowHeight;
	bool windowResizable;
	bool vSync;
	bool msaa;
	std::pair<int, int> glVersion;
	glm::vec4 backgroundColor;
};

class Application {
public:
	Application(const Options& options);

	Application(const Application& rhs) = delete;

	Application(Application&& rhs) = delete;

	~Application();

	void run();

private:
	/* window info */
	std::unique_ptr<Window> _window;

	/* input handler */
	KeyboardInput _keyboardInput;
	MouseInput _mouseInput;

	/* timer for fps */
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastTimeStamp;
	float _deltaTime = 0.0f;
	FrameRateIndicator _fpsIndicator{ 64 };

	/* clear color */
	glm::vec4 _clearColor = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);

	/* stages */
	std::vector <std::unique_ptr<Stage>> _stages;
	int _activeStageIndex = 0;

private:
	void updateTime();

	void handleInput();

	void renderFrame();

	void showFpsInWindowTitle();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	static void cursorMovedCallback(GLFWwindow* window, double xPos, double yPos);

	static void mouseClickedCallback(GLFWwindow* window, int button, int action, int mods);

	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

	static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};