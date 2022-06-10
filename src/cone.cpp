#include "cone.h"

Cone::Cone() {
    GLint  num_stacks = 10000;
	GLfloat PI = 3.1415926f;
    GLdouble angle = 2 * PI / num_stacks;
    GLfloat radius = 3;
    GLfloat height = 5;

    GLint num_vertices = (num_stacks+1) * 3 + 1;
    GLint num_indices = num_stacks * 2 * 3;

	 GLfloat vertices[90012];
     GLint indices[60000];
    
	 // start of cone
    vertices[0] = 0.0;
    vertices[1] = 5.0;
    vertices[2] = 0.0;

    for (GLint i = 0; i <=num_stacks; i++) {
        vertices[3 * i + 3] = cos(angle * i) * radius;
        vertices[3 * i + 4] = 5.0f;
        vertices[3 * i + 5] = -sin(angle * i) * radius;
        vertices[3 * i + 6 + 3 * num_stacks] = vertices[3 * i + 3];
        vertices[3 * i + 7 + 3 * num_stacks] = vertices[3 * i + 4];
        vertices[3 * i + 8 + 3 * num_stacks] = vertices[3 * i + 5];
        vertices[3 * i + 9 + 6 * num_stacks] = 0.0;
        vertices[3 * i + 10 + 6 * num_stacks] = height+5;
        vertices[3 * i + 11 + 6 * num_stacks] = 0.0;
        
            if (i != num_stacks) {
                indices[i * 6] = 0;
                indices[i * 6 + 1] = i + 1;
                indices[i * 6 + 2] = i + 2;
                indices[i * 6 + 3] = i+1+(num_stacks+1)*2;
                indices[i * 6 + 4] = i + 3 + num_stacks;
                indices[i * 6 + 5] = i + 2 + num_stacks;
            }
    }

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
            "   color = vec4(1.0 , 0.0 , 0.0 , 1.0);\n"
            "}\n";

        _shader.reset(new GLSLProgram);
        _shader->attachVertexShader(vsCode);
        _shader->attachFragmentShader(fsCode);
        _shader->link();
    } catch (const std::exception&) {
        cleanup();
        throw;
    }
    
    //error
	GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        std::stringstream ss;
        ss << "cube creation failure, (code " << error << ")";
        cleanup();
        throw std::runtime_error(ss.str());
    }
}

Cone::Cone(Cone&& rhs) noexcept
    : _vao(rhs._vao),
      _vbo(rhs._vbo),
      _ebo(rhs._ebo),
      _shader(std::move(rhs._shader)) {
    rhs._vao = 0;
    rhs._vbo = 0;
    rhs._ebo = 0;
}


Cone::~Cone() {
    cleanup();
}

void Cone::draw(const glm::mat4& projection, const glm::mat4& view) {
    glDepthFunc(GL_LEQUAL);
    _shader->use();
    _shader->setMat4("view",view);
    _shader->setMat4("projection",projection);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_ebo);
    glDrawElements(GL_TRIANGLES, 60000, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

}

void Cone::cleanup() {
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
