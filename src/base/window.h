#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window {
public:
	struct Options {
		std::pair<int, int> glVersion;
		std::string title;
		int width;
		int height;
		bool resizable;
		bool msaa;
		bool vSync;
	};

public:
	Window(const Options& options);

	~Window();

	int getWidth() const;

	int getHeight() const;

	int getResized() const;

	void setWidth(int width);

	void setHeight(int height);

	void setResized(bool resized);

	GLFWwindow* getHandle() const;

	bool shouldClose() const;

	void close() const;
	
	void setUserPointer(void* pointer);

	void swapBuffers() const;

	void showFpsInTitle(float fps) const;

private:
	GLFWwindow* _window = nullptr;
	std::string _title;
	int _width = 0;
	int _height = 0;
	bool _resized = false;

private:
	static void errorCallback(int error, const char* description);
};