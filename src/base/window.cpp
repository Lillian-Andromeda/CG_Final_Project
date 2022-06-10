#include <iostream>
#include <stdexcept>

#include "window.h"

Window::Window(const Window::Options& options)
	: _title(options.title), _width(options.width), _height(options.height) {
	glfwSetErrorCallback(errorCallback);
	if (glfwInit() != GLFW_TRUE) {
		throw std::runtime_error("init glfw failure");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, options.glVersion.first);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, options.glVersion.second);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, options.resizable);

	if (options.msaa) {
		glfwWindowHint(GLFW_SAMPLES, 4);
	}

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	_window = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);
	if (_window == nullptr) {
		glfwTerminate();
		throw std::runtime_error("create glfw window failure");
	}

	glfwMakeContextCurrent(_window);

	if (options.vSync) {
		glfwSwapInterval(1);
	} else {
		glfwSwapInterval(0);
	}

	glfwGetFramebufferSize(_window, &_width, &_height);
}

Window::~Window() {
	if (_window != nullptr) {
		glfwDestroyWindow(_window);
		_window = nullptr;
	}

	glfwTerminate();
}

void Window::setUserPointer(void* pointer) {
	glfwSetWindowUserPointer(_window, pointer);
}

int Window::getWidth() const {
	return _width;
}

int Window::getHeight() const {
	return _height;
}

int Window::getResized() const {
	return _resized;
}

void Window::setWidth(int width) {
	_width = width;
}

void Window::setHeight(int height) {
	_height = height;
}

void Window::setResized(bool resized) {
	_resized = resized;
}

GLFWwindow* Window::getHandle() const {
	return _window;
}

bool Window::shouldClose() const {
	return glfwWindowShouldClose(_window);
}

void Window::close() const {
	return glfwSetWindowShouldClose(_window, true);
}

void Window::swapBuffers() const {
	glfwSwapBuffers(_window);
}

void Window::showFpsInTitle(float fps) const {
	std::string detailTitle = _title + ": " + std::to_string(fps) + " fps";
	glfwSetWindowTitle(_window, detailTitle.c_str());
}

void Window::errorCallback(int error, const char* description) {
	std::cerr << description << std::endl;
}