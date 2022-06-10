#include <cmath>

#include "prism.h"

Prism::Prism() {
	GLfloat vertices[] = { 
		//-2.5, 5.0f, -4.33013f, 
		//2.5f, 5.0f, -4.33013f, 
		//5.0f, 5.0f, 0.0f, 
		//2.5f, 5.0f, 4.33013f, 
		//-2.5f, 5.0f, 4.33013f, 
		//-5.0f, 5.0f, 0.0f, 
		//-5.0f, -5.0f, 0.0f, 
		//-2.5f, -5.0f, -4.33013f, 
		//2.5f, -5.0f, -4.33013f, 
		//5.0f, -5.0f, 0.0f, 
		//2.5f, -5.0f, 4.33013f, 
		//-2.5f, -5.0f, 4.33013f

		-11.0f, 3.0f, -4.33013f,
		-6.0f, 3.0f, -4.33013f,
		-3.5f, 3.0f, 0.0f,
		-6.0f,3.0f, 4.33013f,
		-11.0f, 3.0f, 4.33013f,
		-13.5f, 3.0f, 0.0f,
		-13.5f, 0.0f, 0.0f,
		-11.0f, 0.0f, -4.33013f,
		-6.0f, 0.0f, -4.33013f,
		-3.5f, 0.0f, 0.0f,
		-6.0f, 0.0f, 4.33013f,
		-11.0f, 0.0f, 4.33013f
	};

	GLint indices[] = {
		0,7,8,8,0,1,
		1,8,9,9,1,2,
		2,9,10,9,10,3,
		4,10,11,10,4,3,
		5,6,11,4,5,11,
		0,7,6,0,5,6,
		0,1,5,2,3,4,
		1,4,5,4,1,2,
		6,7,8,9,10,11,
		8,6,11,8,9,11,
	};

	// create vao and vbo
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color
	// glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	try {

		const char* vsCode =
			"#version 330 core\n"
			"layout(location = 0) in vec3 aPosition;\n"
			"uniform mat4 projection;\n"
			"uniform mat4 view;\n"
			"uniform mat4 model;\n"
			"void main() {\n"
			"mat4 model = mat4(1.0f);"
			"   gl_Position = (projection * view * model * vec4(aPosition, 1.0f));\n"
			"}\n";

		const char* fsCode =
			"#version 330 core\n"
			"out vec4 color;\n"
			"void main() {\n"
			"   color = vec4(0.0, 1.0,0.0,1.0);\n"
			"}\n";

		_shader.reset(new GLSLProgram);
		_shader->attachVertexShader(vsCode);
		_shader->attachFragmentShader(fsCode);
		_shader->link();
	}
	catch (const std::exception&) {
		cleanup();
		throw;
	}

	//error部分

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::stringstream ss;
		ss << "cube creation failure, (code " << error << ")";
		cleanup();
		throw std::runtime_error(ss.str());
	}
}

Prism::Prism(Prism&& rhs) noexcept
	: _vao(rhs._vao),
	_vbo(rhs._vbo),
	_ebo(rhs._ebo),
	_shader(std::move(rhs._shader)) {
	rhs._vao = 0;
	rhs._vbo = 0;
	rhs._ebo = 0;
}


Prism::~Prism() {
	cleanup();
}

void Prism::draw(const glm::mat4& projection, const glm::mat4& view) {
	glDepthFunc(GL_LEQUAL);
	_shader->use();
	_shader->setMat4("view", view);
	_shader->setMat4("projection", projection);
	glBindVertexArray(_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);

}

void Prism::cleanup() {
	if (_vbo != 0) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}

	if (_vao != 0) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	if (_ebo != 0) {
		glDeleteVertexArrays(1, &_ebo);
		_ebo = 0;
	}
}