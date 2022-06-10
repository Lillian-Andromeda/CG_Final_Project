#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "./base/glsl_program.h"


class Cylinder {
public:
    Cylinder();

    Cylinder(Cylinder&& rhs) noexcept;

    ~Cylinder();

    void draw(const glm::mat4& projection, const glm::mat4& view);

private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    std::unique_ptr<GLSLProgram> _shader;

    void cleanup();
};
