#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "./base/glsl_program.h"
#include "./base/bounding_box.h"

class Cube {
public:
    Cube();

    Cube(Cube&& rhs) noexcept;

    ~Cube();
	//GLuint getBoundingBoxVao() const;

	//BoundingBox getBoundingBox() const;

	//virtual void drawBoundingBox() const;

    void draw(const glm::mat4& projection, const glm::mat4& view);

private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
	GLuint _boxVao = 0;
	GLuint _boxVbo = 0;
	GLuint _boxEbo = 0;
	//BoundingBox _boundingBox;
	//void computeBoundingBox();
	//void initBoxGLResources();

    std::unique_ptr<GLSLProgram> _shader;

    void cleanup();
};
