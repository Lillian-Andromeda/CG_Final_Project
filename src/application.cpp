#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "application.h"
#include "scene_roaming.h"
#include "whack_moles.h"

Application::Application(const Options& options)
	: _clearColor(options.backgroundColor) {
	// window
	_window.reset(new Window({
		options.glVersion,
		options.windowTitle,
		options.windowWidth,
		options.windowHeight,
		options.windowResizable,
		options.msaa,
		options.vSync
	}));

	std::cout << this << std::endl;
	_window->setUserPointer(this);
	std::cout << glfwGetWindowUserPointer(_window->getHandle()) << std::endl;

	// OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("initialize glad failure");
	}

	glViewport(0, 0, _window->getWidth(), _window->getHeight());

	if (options.msaa) {
		glEnable(GL_MULTISAMPLE);
	}

	// callback functions
	glfwSetFramebufferSizeCallback(_window->getHandle(), framebufferResizeCallback);
	glfwSetKeyCallback(_window->getHandle(), keyboardCallback);
	glfwSetMouseButtonCallback(_window->getHandle(), mouseClickedCallback);
	glfwSetCursorPosCallback(_window->getHandle(), cursorMovedCallback);
	glfwSetScrollCallback(_window->getHandle(), scrollCallback);

	// time stamp
	_lastTimeStamp = std::chrono::high_resolution_clock::now();

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window->getHandle(), true);
	ImGui_ImplOpenGL3_Init();

	// init stages
	_stages.emplace_back(new SceneRoaming);
	_stages.emplace_back(new WhackMoles);

	_stages[_activeStageIndex]->init(*_window, _mouseInput);

	std::cout << "Application Address: " << this << std::endl;
} 

Application::~Application() {
	// destroy imgui context
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Application::run() {
	while (!_window->shouldClose()) {
		updateTime();
		handleInput();
		renderFrame();

		_window->swapBuffers();
		glfwPollEvents();
	}
}

void Application::updateTime() {
	auto now = std::chrono::high_resolution_clock::now();
	_deltaTime = 0.001f * std::chrono::duration<float, std::milli>(now - _lastTimeStamp).count();
	_lastTimeStamp = now;
	_fpsIndicator.push(1.0f / _deltaTime);
}

void Application::showFpsInWindowTitle() {
	float fps = _fpsIndicator.getAverageFrameRate();
	_window->showFpsInTitle(fps);
}

void Application::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_window->setWidth(width);
	app->_window->setHeight(height);
	app->_window->setResized(true);
	glViewport(0, 0, width, height);
}

void Application::cursorMovedCallback(GLFWwindow* window, double xPos, double yPos) {
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_mouseInput.move.xCurrent = xPos;
	app->_mouseInput.move.yCurrent = yPos;
}

void Application::mouseClickedCallback(GLFWwindow* window, int button, int action, int mods) {
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				app->_mouseInput.click.left = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				app->_mouseInput.click.middle = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				app->_mouseInput.click.right = true;
				break;
		}
	} else if (action == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			app->_mouseInput.click.left = false;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			app->_mouseInput.click.middle = false;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			app->_mouseInput.click.right = false;
			break;
		}
	}
}

void Application::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_mouseInput.scroll.x += xOffset;
	app->_mouseInput.scroll.y += yOffset;
}

void Application::keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key != GLFW_KEY_UNKNOWN) {
		Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		app->_keyboardInput.keyStates[key] = action;
	}
}

void Application::handleInput() {
	//std::cout << _keyboardInput.keyStates[GLFW_KEY_ENTER] << std::endl;
	if (_keyboardInput.keyStates[GLFW_KEY_ENTER] != GLFW_RELEASE) {
		std::cout << "switch stage" << std::endl;
		_stages[_activeStageIndex]->deinit();
		_activeStageIndex ^= 1;
		_stages[_activeStageIndex]->init(*_window, _mouseInput);

		_keyboardInput.keyStates[GLFW_KEY_ENTER] = GLFW_RELEASE;
	}

	_stages[_activeStageIndex]->handleInput(*_window, _keyboardInput, _mouseInput, _deltaTime);
}

void Application::renderFrame() {
	_stages[_activeStageIndex]->renderFrame();
}