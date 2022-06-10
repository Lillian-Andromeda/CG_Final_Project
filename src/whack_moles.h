#pragma once

#include <vector>
#include <memory>

#include "./base/stage.h"
#include "./base/camera.h"
#include "./base/light.h"
#include "./base/glsl_program.h"
#include "model.h"
#include "./base/material.h"
#include "./base/skybox.h"


class WhackMoles final : public Stage {
public:
	WhackMoles() = default;

	~WhackMoles() = default;

	void init(Window& window, MouseInput& mouseInput) override;

	void deinit() override;

	void handleInput(
		Window& window,
		KeyboardInput& keyboardInput, MouseInput& mouseInput,
		float deltaTime) override;

	void renderFrame() override;

private:
	// 3D objects
	std::vector<std::unique_ptr<Camera>> _cameras;
	int activeCameraIndex = 0;

	std::vector<std::unique_ptr<Model>> _models;
	std::vector<std::unique_ptr<PhongMaterial>> _materials;

	std::unique_ptr<AmbientLight> _ambientLight;
	std::unique_ptr<DirectionalLight> _directionalLight;
	std::unique_ptr<SpotLight> _spotLight;

	std::unique_ptr<GLSLProgram> _phongShader;

	std::unique_ptr<SkyBox> _skybox;

private:
	void initPhongShader();
};