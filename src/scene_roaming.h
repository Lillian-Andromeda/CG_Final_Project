#pragma once

#include <memory>
#include <vector>
#include <string>

#include "./base/stage.h"
#include "./base/glsl_program.h"
#include "./base/skybox.h"
#include "./base/light.h"
#include "./base/camera.h"
#include "./base/texture.h"
#include "./base/material.h"

#include "model.h"
#include "ball.h"
#include "cone.h"
#include "cube.h"
#include "cylinder.h"
#include "prism.h"

class SceneRoaming final: public Stage {
public:
	SceneRoaming() = default;
	
	~SceneRoaming() = default;

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
	int activeModelIndex = 0;

	std::unique_ptr<SkyBox> _skybox;

	std::unique_ptr<Ball> _ball;
	std::unique_ptr<Cone> _cone;
	std::unique_ptr<Cube> _cube;
	std::unique_ptr<Cylinder> _cylinder;
	std::unique_ptr<Prism> _prism;

	std::vector<std::unique_ptr<PhongMaterial>> _materials;

	// shaders
	std::unique_ptr<GLSLProgram> _phongShader;

	// lights
	std::unique_ptr<AmbientLight> _ambientLight;
	std::unique_ptr<DirectionalLight> _directionalLight;
	std::unique_ptr<SpotLight> _spotLight;

private:
	void initPhongShader();
};