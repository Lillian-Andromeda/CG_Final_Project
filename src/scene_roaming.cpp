#include <algorithm>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "scene_roaming.h"

const std::string cabinPath = "./media/cabin.obj";

const std::string bunnyPath = "./media/bunny.obj";
const std::string cabinTexturePath = "./media/wood.jpg";
const std::string bunnyTexturePath = "./media/flower.jpg";

const std::vector<std::string> skyboxTexturePaths = {
	"./media/field/posx.jpg",
	"./media/field/negx.jpg",
	"./media/field/posy.jpg",
	"./media/field/negy.jpg",
	"./media/field/posz.jpg",
	"./media/field/negz.jpg"
};

bool CollisionDetection(BoundingBox box, glm::mat4 modelMatrix, glm::vec3 cameraPos) {
	glm::vec3 boxMax = glm::vec3(modelMatrix * glm::vec4(box.max, 1.0f));
	glm::vec3 boxMin = glm::vec3(modelMatrix * glm::vec4(box.min, 1.0f));

	if (boxMin.x < cameraPos.x && boxMin.y < cameraPos.y && boxMin.z < cameraPos.z) {
		if (cameraPos.x < boxMax.x && cameraPos.y < boxMax.y && cameraPos.z < boxMax.z) {
			return true;
		}
		else 
			return false;
	}
	return false;
}

void SceneRoaming::init(Window& window, MouseInput& mouseInput) {
	int windowWidth = window.getWidth();
	int windowHeight = window.getHeight();

	// set input mode
	mouseInput.move.xOld = mouseInput.move.xCurrent = 0.5 * window.getWidth();
	mouseInput.move.yOld = mouseInput.move.yCurrent = 0.5 * window.getHeight();
	glfwSetCursorPos(window.getHandle(), mouseInput.move.xCurrent, mouseInput.move.yCurrent);

	// init cameras
	_cameras.resize(2);
	const float aspect = 1.0f * windowWidth / windowHeight;
	constexpr float znear = 0.1f;
	constexpr float zfar = 10000.0f;

	// camera 0: Rotate around local coordinate
	_cameras[0].reset(new PerspectiveCamera(
		glm::radians(60.0f), aspect, 0.1f, 10000.0f));
	_cameras[0]->position = glm::vec3(0.0f, 5.0f, 20.0f);
	
	
	// camera 1: Rotate around (0, 0, 0)
	_cameras[1].reset(new PerspectiveCamera(
		glm::radians(60.0f), aspect, 0.1f, 10000.0f));
	_cameras[1]->position = glm::vec3(0.0f, 5.0f, 20.0f);


	// init skybox
	_skybox.reset(new SkyBox(skyboxTexturePaths));

	// init models
	_models.resize(2);
	// bunny
	_models[0].reset(new Model(bunnyPath));
	_models[0]->position = glm::vec3(8.8f, 7.0f, 2.0f);
	// cabin
	_models[1].reset(new Model(cabinPath));
	_models[1]->position = glm::vec3(0.0f, 0.0f, -10.0f);
	
	// init textures
	std::shared_ptr<Texture2D> bunnyTexture = std::make_shared<Texture2D>(bunnyTexturePath);
	std::shared_ptr<Texture2D> cabinTexture = std::make_shared<Texture2D>(cabinTexturePath);

	// init materials
	_materials.resize(2);
	for (int i = 0; i < _materials.size(); i++) {
		_materials[i].reset(new PhongMaterial);
		_materials[i]->ka = glm::vec3(0.03f, 0.03f, 0.03f);
		_materials[i]->kd = glm::vec3(1.0f, 1.0f, 1.0f);
		_materials[i]->ks = glm::vec3(1.0f, 1.0f, 1.0f);
		_materials[i]->ns = 10.0f;
	}
	_materials[0]->mapKd = bunnyTexture;
	_materials[1]->mapKd = cabinTexture;

	// init geometries
	_ball.reset(new Ball());
	_cone.reset(new Cone());
	_cube.reset(new Cube());
	_cylinder.reset(new Cylinder());
	_prism.reset(new Prism());


	// init lights
	_ambientLight.reset(new AmbientLight);
	_ambientLight->color = glm::vec3(0.8f, 0.8f, 0.8f);

	_directionalLight.reset(new DirectionalLight);
	_directionalLight->position = glm::vec3(10.0f, 10.0f, 10.0f);
	_directionalLight->rotation = glm::angleAxis(glm::radians(45.0f), -glm::vec3(1.0f, 1.0f, 1.0f));
	_directionalLight->color = glm::vec3(0.5f, 0.5f, 0.5f);

	_spotLight.reset(new SpotLight);
	_spotLight->position = glm::vec3(0.0f, 0.0f, 5.0f);
	_spotLight->rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	// init shaders
	initPhongShader();
}

void SceneRoaming::deinit() {

}

void SceneRoaming::handleInput(Window& window, KeyboardInput& keyboardInput, MouseInput& mouseInput, float deltaTime) {
	constexpr float cameraMoveSpeed = 5.0f;
	constexpr float modelMoveSpeed = 5.0f;
	constexpr float cameraRotateSpeed = 0.2f;
	constexpr float modelRotateSpeed = 5.0f;

	int windowWidth = window.getWidth();
	int windowHeight = window.getHeight();

	// "ESC" - exit the program
	if (keyboardInput.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		window.close();
		return;
	}

	// "Space" - switch camera
	if (keyboardInput.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS) {
		std::cout << "switch camera" << std::endl;
		activeCameraIndex = (activeCameraIndex + 1) % _cameras.size();
		keyboardInput.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
		return;
	}
	Camera* camera = _cameras[activeCameraIndex].get();
	glm::vec3 oldCameraPosition = camera->position;


	// "K" - switch model
	if (keyboardInput.keyStates[GLFW_KEY_K] == GLFW_PRESS) {
		std::cout << "switch model" << std::endl;
		activeModelIndex = (activeModelIndex + 1) % _models.size();
		keyboardInput.keyStates[GLFW_KEY_K] = GLFW_RELEASE;
		return;
	}

	// "middle mouse button" - zoom to fit
	if (mouseInput.click.middle == true) {
		std::cout << "Zoom To Fit" << std::endl;
		const float aspect = 1.0f * windowWidth / windowHeight;
		camera->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		camera->position = glm::vec3(0.0f, 0.0f, 15.0f);
		mouseInput.scroll.y = 0;
		mouseInput.click.middle = false;
		return;
	}

	// "W" - camera moves up	
	if (keyboardInput.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
		std::cout << "W" << std::endl;
		camera->position += cameraMoveSpeed * camera->getUp()* deltaTime;
	}
	// "A" - camera moves left
	if (keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
		std::cout << "A" << std::endl;
		camera->position -= cameraMoveSpeed * camera->getRight() * deltaTime;
	}
	// "S" - camera moves down
	if (keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
		std::cout << "S" << std::endl;
		camera->position -= cameraMoveSpeed * camera->getUp()* deltaTime;
	}
	// "D" - camera moves right
	if (keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
		std::cout << "D" << std::endl;
		camera->position += cameraMoveSpeed * camera->getRight() * deltaTime;
	}

	// "¡ü" - model moves up
	if (keyboardInput.keyStates[GLFW_KEY_UP] != GLFW_RELEASE) {
		std::cout << "¡ü" << std::endl;
		_models[activeModelIndex]->position += modelMoveSpeed * glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime;
	}
	// "¡û" - model moves left
	if (keyboardInput.keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE) {
		std::cout << "¡û" << std::endl;
		_models[activeModelIndex]->position -= glm::vec3(1.0f, 0.0f, 0.0f) * modelMoveSpeed * deltaTime;
	}
	// "¡ý" - model moves down
	if (keyboardInput.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) {
		std::cout << "¡ý" << std::endl;
		_models[activeModelIndex]->position -= modelMoveSpeed * glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime;
	}
	// "¡ú" - model moves right
	if (keyboardInput.keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE) {
		std::cout << "¡ú" << std::endl;
		_models[activeModelIndex]->position += glm::vec3(1.0f, 0.0f, 0.0f) * modelMoveSpeed * deltaTime;
	}
	// "<" - model moves foward
	if (keyboardInput.keyStates[GLFW_KEY_COMMA] != GLFW_RELEASE) {
		std::cout << "<" << std::endl;
		_models[activeModelIndex]->position -= modelMoveSpeed * glm::vec3(0.0f, 0.0f, 1.0f) * deltaTime;
	}
	// ">" - model moves backward
	if (keyboardInput.keyStates[GLFW_KEY_PERIOD] != GLFW_RELEASE) {
		std::cout << ">" << std::endl;
		_models[activeModelIndex]->position += glm::vec3(0.0f, 0.0f, 1.0f) * modelMoveSpeed * deltaTime;
	}

	// "Z"  - XY×ª	
	if (keyboardInput.keyStates[GLFW_KEY_Z] != GLFW_RELEASE) {
		std::cout << "Z" << std::endl;
		const glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
		const float angle = -modelRotateSpeed * deltaTime;
		std::cout << angle << std::endl;
		_models[activeModelIndex]->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *_models[activeModelIndex]->rotation;
	}
	// "X" -  XZ×ª	
	if (keyboardInput.keyStates[GLFW_KEY_X] != GLFW_RELEASE) {
		std::cout << "X" << std::endl;
		const glm::vec3 axis = { 1.0f, 0.0f, 0.0f };
		const float angle = -modelRotateSpeed * deltaTime;
		std::cout << angle << std::endl;
		_models[activeModelIndex]->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *_models[activeModelIndex]->rotation;
	}
	// "C" -  YZ×ª
	if (keyboardInput.keyStates[GLFW_KEY_C] != GLFW_RELEASE) {
		std::cout << "C" << std::endl;
		const glm::vec3 axis = { 0.0f, 0.0f, 1.0f };
		const float angle = -modelRotateSpeed * deltaTime;
		std::cout << angle << std::endl;
		_models[activeModelIndex]->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *_models[activeModelIndex]->rotation;
	}

	// "]" - model scale up
	if (keyboardInput.keyStates[GLFW_KEY_LEFT_BRACKET] != GLFW_RELEASE) {
		std::cout << "[" << std::endl;
		const float scale = std::max(0.1f, _models[activeModelIndex]->scale[0] - deltaTime);
		_models[activeModelIndex]->scale = { scale, scale, scale };
	}
	// "[" - model scale down
 	if (keyboardInput.keyStates[GLFW_KEY_RIGHT_BRACKET] != GLFW_RELEASE) {
		std::cout << "]" << std::endl;
		const float scale = _models[activeModelIndex]->scale[0] + deltaTime;
		_models[activeModelIndex]->scale = { scale, scale, scale };
	}


	if (keyboardInput.keyStates[GLFW_KEY_F2] == GLFW_PRESS) {
		//CaptureScreen(_windowWidth, _windowHeight, renderFrame());
		long width = windowWidth, height = windowHeight;
		//Transformation::renderFrame();
		unsigned char* screenPixel = new unsigned char[width * height * 3];
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, screenPixel);
		int l = (width * 3 + 3) / 4 * 4;
		time_t t = time(0);
		static char tmp[64] = "";
		strftime(tmp, sizeof(tmp), "./userdata/%Y_%m_%d_%H_%M_%S.bmp", localtime(&t));
		//std::string name = "";//tmp;
		//name = filePath + "screenshot" + name + ".bmp";
		int bmi[] = { l * height + 544,00,54,40,width,height,1 | 3 * 8 << 16,0,l * height,0,0,100,0 };
		FILE* fp = fopen(tmp, "wb");
		fprintf(fp, "BM");
		fwrite(&bmi, 52, 1, fp);
		fwrite(screenPixel, 1, l * height, fp);
		fclose(fp);
		keyboardInput.keyStates[GLFW_KEY_F2] = GLFW_RELEASE;
	}

	// Mouse Scroll  - Zoom in/out
	if (mouseInput.scroll.y != 0) {
		if (mouseInput.scroll.y > 0)
			std::cout << "Zoom in" << mouseInput.scroll.y << std::endl;
		else
			std::cout << "Zoom out" << mouseInput.scroll.y << std::endl;
		camera->position -= cameraMoveSpeed * camera->getFront() * (float)mouseInput.scroll.y * deltaTime;
		if (mouseInput.scroll.y > 0)
			mouseInput.scroll.y -= std::max(0.1, mouseInput.scroll.y / 20.0f);
		else
			mouseInput.scroll.y += std::max(0.1, -mouseInput.scroll.y / 20.0f);;
		if (std::abs(mouseInput.scroll.y) < 0.1)
			mouseInput.scroll.y = 0;
	}

	
	// Mouse Right Click - Move Direction
	if (mouseInput.move.xCurrent != mouseInput.move.xOld) {
		std::cout << "mouse move in x direction" << std::endl;
		if (mouseInput.click.right == true) {
			const float deltaX = static_cast<float>(mouseInput.move.xCurrent - mouseInput.move.xOld);
			const float angle = -cameraRotateSpeed * deltaTime * deltaX;
			const glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
			camera->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *camera->rotation;
			if (activeCameraIndex == 1) {
				camera->position = -camera->getFront() * glm::length(camera->position);
			}
		}
		mouseInput.move.xOld = mouseInput.move.xCurrent;
	}

	if (mouseInput.move.yCurrent != mouseInput.move.yOld) {
		std::cout << "mouse move in y direction" << std::endl;
		if (mouseInput.click.right == true) {
			const float deltaY = static_cast<float>(mouseInput.move.yCurrent - mouseInput.move.yOld);
			const float angle = -cameraRotateSpeed * deltaTime * deltaY;
			const glm::vec3 axis = camera->getRight();
			camera->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *camera->rotation;
			if (activeCameraIndex == 1) {
				camera->position = -camera->getFront() * glm::length(camera->position);
			}
		}
		mouseInput.move.yOld = mouseInput.move.yCurrent;
	}

	// collision detection
	for (int i = 0; i < _models.size(); i++) {
		if (CollisionDetection(_models[i]->getBoundingBox(), _models[i]->getModelMatrix(), camera->position)) {
			camera->position = oldCameraPosition;
		}
	}
}

void SceneRoaming::renderFrame() {
	// trivial things
//	showFpsInWindowTitle();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 projection = _cameras[activeCameraIndex]->getProjectionMatrix();
	glm::mat4 view = _cameras[activeCameraIndex]->getViewMatrix();
	
	_phongShader->use();
	// 1. transfer mvp matrix to the shader
	_phongShader->setMat4("projection", projection);
	_phongShader->setMat4("view", view);

	// 2. transfer light attributes to the shader
	_phongShader->setVec3("ambientLight.color", _ambientLight->color);
	_phongShader->setFloat("ambientLight.intensity", _ambientLight->intensity);
	_phongShader->setVec3("spotLight.position", _spotLight->position);
	_phongShader->setVec3("spotLight.direction", _spotLight->getFront());
	_phongShader->setFloat("spotLight.intensity", _spotLight->intensity);
	_phongShader->setVec3("spotLight.color", _spotLight->color);
	_phongShader->setFloat("spotLight.angle", _spotLight->angle);
	_phongShader->setFloat("spotLight.kc", _spotLight->kc);
	_phongShader->setFloat("spotLight.kl", _spotLight->kl);
	_phongShader->setFloat("spotLight.kq", _spotLight->kq);
	_phongShader->setVec3("directionalLight.direction", _directionalLight->getFront());
	_phongShader->setFloat("directionalLight.intensity", _directionalLight->intensity);
	_phongShader->setVec3("directionalLight.color", _directionalLight->color);

	// 3. transfer view position to the shader
	_phongShader->setVec3("eye.position", _cameras[activeCameraIndex]->position);

	// draw models
	for (int i = 0; i < _models.size() ; i++) {
		_phongShader->setMat4("model", _models[i]->getModelMatrix());
		_phongShader->setVec3("material.ka", _materials[i]->ka);
		_phongShader->setVec3("material.kd", _materials[i]->kd);
		_phongShader->setVec3("material.ks", _materials[i]->ks);
		_phongShader->setFloat("material.ns", _materials[i]->ns);

		glActiveTexture(GL_TEXTURE0);
		_materials[i]->mapKd->bind();

		_models[i]->draw();
	}

	_skybox->draw(projection, view);

	_ball->draw(projection, view);
	_cone->draw(projection, view);
	_cube->draw(projection, view);
	_cylinder->draw(projection, view);
	_prism->draw(projection, view);

	// draw ui elements
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::Begin("Control Panel", nullptr, flags)) {
		ImGui::End();
	}
	else {
		ImGui::Text("material: ");
		ImGui::Separator();
		ImGui::ColorEdit3("ka", (float*)&_materials[0]->ka);
		ImGui::ColorEdit3("kd", (float*)&_materials[0]->kd);
		ImGui::ColorEdit3("ks", (float*)&_materials[0]->ks);
		ImGui::ColorEdit3("ns", (float*)&_materials[0]->ns);
		ImGui::NewLine();

		ImGui::Text("ambient light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##1", &_ambientLight->intensity, 0.0f, 2.0f);
		ImGui::ColorEdit3("color##1", (float*)&_ambientLight->color);
		ImGui::NewLine();

		ImGui::Text("directional light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##2", &_directionalLight->intensity, 0.0f, 1.5f);
		ImGui::ColorEdit3("color##2", (float*)&_directionalLight->color);
		ImGui::NewLine();

		ImGui::Text("spot light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity##3", &_spotLight->intensity, 0.0f, 1.5f);
		ImGui::ColorEdit3("color##3", (float*)&_spotLight->color);
		ImGui::SliderFloat("angle##3", (float*)&_spotLight->angle, 0.0f, glm::radians(180.0f), "%f rad");
		ImGui::NewLine();

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneRoaming::initPhongShader() {
	const char* vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"

		"out vec3 fPosition;\n"
		"out vec3 fNormal;\n"
		"out vec2 fTexCoord;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"void main() {\n"
		"	fPosition = vec3(model * vec4(aPosition, 1.0f));\n"
		"	fNormal = mat3(transpose(inverse(model))) * aNormal;\n"
		"	fTexCoord = aTexCoord;\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";


	const char* fsCode =
		"#version 330 core\n"
		"in vec3 fPosition;\n"
		"in vec3 fNormal;\n"
		"in vec2 fTexCoord;\n"
		"out vec4 color;\n"

		"// material data structure declaration\n"
		"struct Material {\n"
		"	vec3 ka; \n"
		"	vec3 kd;\n"
		"	vec3 ks;\n"
		"	float ns;\n"
		"};\n"

		"// ambient light data structure declaration\n"
		"struct AmbientLight {\n"
		"	vec3 color;\n"
		"	float intensity;\n"
		"};\n"

		"// directional light data structure declaration\n"
		"struct DirectionalLight {\n"
		"	vec3 direction;\n"
		"	float intensity;\n"
		"	vec3 color;\n"
		"};\n"

		"// spot light data structure declaration\n"
		"struct SpotLight {\n"
		"	vec3 position;\n"
		"	vec3 direction;\n"
		"	float intensity;\n"
		"	vec3 color;\n"
		"	float angle;\n"
		"	float kc;\n"
		"	float kl;\n"
		"	float kq;\n"
		"};\n"

		"// view data structure declaration\n"
		"struct Eye {\n"
		"	vec3 position;\n"
		"};\n"

		"// uniform variables\n"
		"uniform Material material;\n"
		"uniform AmbientLight ambientLight;\n"
		"uniform DirectionalLight directionalLight;\n"
		"uniform SpotLight spotLight;\n"
		"uniform Eye eye;\n"
		"uniform sampler2D mapKd;\n"

		"vec3 calcAmbient() {\n"
		"	vec3 ambient = material.ka * ambientLight.color * ambientLight.intensity;\n"
		"	return ambient;\n "
		"}\n"

		"vec3 calcDiffuse(vec3 normal){\n"
		"	vec3 direlightDir = normalize(-directionalLight.direction);\n"
		"	vec3 dirediffuse = directionalLight.intensity * directionalLight.color * max(dot(direlightDir, normal), 0.0f) * material.kd;\n"

		"	vec3 spotlightDir = normalize(spotLight.position - fPosition);\n"
		"	float theta = acos(-dot(spotlightDir, normalize(spotLight.direction)));\n"
		"	if (theta > spotLight.angle) {\n"
		"		return dirediffuse;\n"
		"	}\n"
		"	else{\n"
		"		vec3 spotdiffuse = spotLight.color * max(dot(spotlightDir, normal), 0.0f) * material.kd;\n"
		"		float distance = length(spotLight.position - fPosition);\n"
		"		float attenuation = 1.0f / (spotLight.kc + spotLight.kl * distance + spotLight.kq * distance * distance);\n"
		"		return (dirediffuse + spotLight.intensity * attenuation * spotdiffuse);\n"
		"	}\n"
		"}\n"

		"vec3 calcSpecular(vec3 normal) {\n"
		"	vec3 direlightDir = normalize(-directionalLight.direction);\n"
		"	vec3 spotlightDir = normalize(spotLight.position - fPosition);\n"
		"	vec3 direreflectDir = reflect(-direlightDir, normal);\n"
		"	vec3 spotreflectDir = reflect(-spotlightDir, normal);\n"
		"	vec3 viewDir = normalize(eye.position - fPosition);\n"

		"	vec3 directionalSpec = directionalLight.intensity * directionalLight.color * pow(max(dot(direreflectDir, viewDir), 0.0f), material.ns) * material.ks; \n"

		"	float theta = acos(-dot(spotlightDir, normalize(spotLight.direction)));\n"
		"	if (theta > spotLight.angle) {\n"
		"		return directionalSpec;\n"
		"	}\n"
		"	else {\n"
		"		vec3 spotSpec = spotLight.color * pow(max(dot(spotreflectDir, viewDir), 0.0f), material.ns) * material.ks;\n"
		"		float distance = length(spotLight.position - fPosition);\n"
		"		float attenuation = 1.0f / (spotLight.kc + spotLight.kl * distance + spotLight.kq * distance * distance);\n"
		"		spotSpec = spotLight.intensity * attenuation * spotSpec;\n"
		"		return (directionalSpec + spotSpec);\n "
		"	}\n"
		"}\n"


		"void main() {\n"
		"	vec3 normal = normalize(fNormal);\n"
		"	vec4 result;\n"
		"	result = vec4(calcAmbient() + calcDiffuse(normal) + calcSpecular(normal), 1.0f);\n"
		"	color = texture(mapKd, fTexCoord) * result;\n"
		"}\n";


	_phongShader.reset(new GLSLProgram);
	_phongShader->attachVertexShader(vsCode);
	_phongShader->attachFragmentShader(fsCode);
	_phongShader->link();
}