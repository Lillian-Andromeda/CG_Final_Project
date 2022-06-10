#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "whack_moles.h"

const std::string modelPath = "./media/gopher.obj";
const std::string holePath = "./media/hole.obj";

const std::string gopherTexturePath = "./media/gopher.jpeg";
const std::string stoneTexturePath = "./media/stone.jpeg";


const std::vector<std::string> skyboxPaths = {
	"./media/field/posx.jpg",
	"./media/field/negx.jpg",
	"./media/field/posy.jpg",
	"./media/field/negy.jpg",
	"./media/field/posz.jpg",
	"./media/field/negz.jpg"
};

void WhackMoles::init(Window& window, MouseInput& mouseInput) {
	int windowWidth = window.getWidth();
	int windowHeight = window.getHeight();

	// set input mode
	mouseInput.move.xOld = mouseInput.move.xCurrent = 0.5 * windowWidth;
	mouseInput.move.yOld = mouseInput.move.yCurrent = 0.5 * windowHeight;
	glfwSetCursorPos(window.getHandle(), mouseInput.move.xCurrent, mouseInput.move.yCurrent);

	// init cameras
	_cameras.resize(2);

	const float aspect = 1.0f * windowWidth / windowHeight;
	constexpr float znear = 0.1f;
	constexpr float zfar = 10000.0f;

	// perspective camera
	_cameras[0].reset(new PerspectiveCamera(
		glm::radians(60.0f), aspect, 0.1f, 10000.0f));
	_cameras[0]->position = glm::vec3(0.0f, 3.0f, 15.0f);
//	_cameras[0]->rotation = glm::quat(cosf(5.0), sinf(5.0)*glm::vec3{-1.0f, 0.0f, 0.0f})*_cameras[0]->rotation;

	//// orthographic camera
	_cameras[1].reset(new PerspectiveCamera(
		glm::radians(60.0f), aspect, 0.1f, 10000.0f));
	_cameras[1]->position = glm::vec3(0.0f, 3.0f, 15.0f);

	_skybox.reset(new SkyBox(skyboxPaths));

	// init model
	_models.resize(10);
	for (int i = 0; i < 9; i++) {
		_models[i].reset(new Model(modelPath));
		_models[i]->scale = glm::vec3(0.9f, 0.9f, 0.9f);
		_models[i]->position = glm::vec3(i / 3 * 5.0f - 5.0f, -4.0f, i % 3 * 5.0f - 5.0f);
	}
	_models[9].reset(new Model(holePath));
	_models[9]->scale = glm::vec3(0.5f, 0.5f, 0.5f);
	_models[9]->position = glm::vec3(0.0f, 0.0f, 0.0f);


	// init textures
	std::shared_ptr<Texture2D> gopherTexture = std::make_shared<Texture2D>(gopherTexturePath);
	std::shared_ptr<Texture2D> stoneTexture = std::make_shared<Texture2D>(stoneTexturePath);


	// init materials
	_materials.resize(2);
	for (int i = 0; i < _materials.size(); i++) {
		_materials[i].reset(new PhongMaterial);
		_materials[i]->ka = glm::vec3(0.03f, 0.03f, 0.03f);
		_materials[i]->kd = glm::vec3(1.f, 1.0f, 1.0f);
		_materials[i]->ks = glm::vec3(1.0f, 1.0f, 1.0f);
		_materials[i]->ns = 10.0f;
	}
	_materials[0]->mapKd = gopherTexture;
	_materials[1]->mapKd = stoneTexture;
	
	// init lights
	_ambientLight.reset(new AmbientLight);
	_ambientLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
	_ambientLight->intensity =2.0f;
	_ambientLight->position = glm::vec3(10.0f, 10.f, 10.0f);

	_directionalLight.reset(new DirectionalLight);
	_directionalLight->position = glm::vec3(0.0f, 10.0f, 10.0f);
	_directionalLight->rotation = glm::angleAxis(glm::radians(45.0f), -glm::vec3(1.0f, 1.0f, 1.0f));
	_directionalLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
	_directionalLight->intensity = 1.0f;

	_spotLight.reset(new SpotLight);
	_spotLight->position = glm::vec3(0.0f, 5.0f, 5.0f);
	_spotLight->rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	// init shader
	initPhongShader();

}

void WhackMoles::deinit() {

}

void WhackMoles::handleInput(
	Window& window,
	KeyboardInput& keyboardInput, MouseInput& mouseInput,
	float deltaTime) {
	int windowWidth = window.getWidth();
	int windowHeight = window.getHeight();

	constexpr float cameraMoveSpeed = 5.0f;
	constexpr float modelMoveSpeed = 5.0f;
	constexpr float cameraRotateSpeed = 0.1f;
	constexpr float modelRotateSpeed = 5.0f;
	static float modelUpSpeed[10] = { 0.0f };
	static float dY[10] = { 0.0f };
	static int dtime1[10], dtime2[10];
	static bool first = true;
	static int flyBunny = 1;
	static int ran = 17;
	srand(time(NULL));

	if (first) {
		for (int i = 0; i < 9; i++) {
			dY[i] = 0.001f;
			dtime1[i] = 100;
			dtime2[i] = 50;
			modelUpSpeed[i] = 0.0f;
		}
		modelUpSpeed[rand() % 9] = 0.05f;
		first = false;
	}

	if (rand() % ran == 0 && flyBunny <= 3) {
		int r = rand() % 9;
		if (modelUpSpeed[r] == 0.0f)
			flyBunny++,
			modelUpSpeed[r] = 0.05f;
		ran = rand() % 15 + 7;
	}
	
	if (keyboardInput.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		window.close();
		return;
	}

	if (keyboardInput.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS) {
		std::cout << "switch camera" << std::endl;
		// switch camera
		activeCameraIndex = (activeCameraIndex + 1) % _cameras.size();
		keyboardInput.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
		return;
	}

	Camera* camera = _cameras[activeCameraIndex].get();

	if (mouseInput.click.middle == true) {
		std::cout << "Zoom To Fit" << std::endl;
		const float aspect = 1.0f * windowWidth / windowHeight;
		camera->rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		camera->position = glm::vec3(0.0f, 0.0f, 15.0f);
		mouseInput.scroll.y = 0;
		mouseInput.click.middle = false;
		return;
	}

	//Mouse Scroll Zoom in/out
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

	//Mouse Left Click Move Direction
	if (mouseInput.move.xCurrent != mouseInput.move.xOld) {
		std::cout << "mouse move in x direction" << std::endl;
		if (mouseInput.click.left == true) {
			const float deltaX = static_cast<float>(mouseInput.move.xCurrent - mouseInput.move.xOld);
			const float angle = -cameraRotateSpeed * deltaTime * deltaX;
			const glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
			camera->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *camera->rotation;
			if (activeCameraIndex == 1) {
				const float Length = glm::length(camera->position);
				camera->position = -camera->getFront() * Length;
			}
		}
		mouseInput.move.xOld = mouseInput.move.xCurrent;
	}

	if (mouseInput.move.yCurrent != mouseInput.move.yOld) {
		std::cout << "mouse move in y direction" << std::endl;
		if (mouseInput.click.left == true) {
			const float deltaY = static_cast<float>(mouseInput.move.yCurrent - mouseInput.move.yOld);
			const float angle = -cameraRotateSpeed * deltaTime * deltaY;
			const glm::vec3 axis = camera->getRight();
			camera->rotation = glm::quat{ cos(angle / 2.0f), axis * sin(angle / 2.0f) } *camera->rotation;
			if (activeCameraIndex == 1) {
				const float Length = glm::length(camera->position);
				camera->position = -camera->getFront() * Length;
			}
		}
		mouseInput.move.yOld = mouseInput.move.yCurrent;
	}

	for (int i = 0; i < 9; i++) {
		if (dY[i] == 5.0f) {
			dtime1[i]--;
		} else if (dY[i] == 0.0f) {
			dtime2[i]--;
		} else {
			dY[i] += modelUpSpeed[i];
		}

		if (dY[i] > 5.0f) {
			dY[i] = 5.0f;
		} else if (dY[i] < 0.0f) {
			dY[i] = 0.0f;
		}

		_models[i]->position = glm::vec3(_models[i]->position.x, dY[i], _models[i]->position.z);
		if (dtime1[i] == 0) {
			dY[i] += (modelUpSpeed[i] = -modelUpSpeed[i]), dtime1[i] = 100;
		}

		if (dtime2[i] == 0) {
			flyBunny--;
			modelUpSpeed[i] = 0;
			dtime2[i] = 50;
			dY[i] = 0.001f;
			int r = rand() % 9;
			if (modelUpSpeed[r] == 0.0f)
				flyBunny++,
				modelUpSpeed[r] = 0.05f;
		}
	}
}


void WhackMoles::renderFrame() {
	//	showFpsInWindowTitle();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 projection = _cameras[activeCameraIndex]->getProjectionMatrix();
	glm::mat4 view = _cameras[activeCameraIndex]->getViewMatrix();

	_phongShader->use();
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

	_phongShader->setVec3("material.ka", _materials[0]->ka);
	_phongShader->setVec3("material.kd", _materials[0]->kd);
	_phongShader->setVec3("material.ks", _materials[0]->ks);
	_phongShader->setFloat("material.ns", _materials[0]->ns);

	glActiveTexture(GL_TEXTURE0);
	_materials[0]->mapKd->bind();

	for (int i = 0; i < 9; i++) {
		_phongShader->setMat4("model", _models[i]->getModelMatrix());
		_models[i]->draw();
	}

	glActiveTexture(GL_TEXTURE0);
	_materials[1]->mapKd->bind();
	_phongShader->setMat4("model", _models[9]->getModelMatrix());
	_models[9]->draw();


	_skybox->draw(projection, view);

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

void WhackMoles::initPhongShader() {
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