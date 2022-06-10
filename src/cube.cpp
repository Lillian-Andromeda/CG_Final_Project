#include <cmath>

#include "cube.h"

Cube::Cube() {
    GLfloat vertices[] = {
        6.0f,5.0f,5.0f, 
        6.0f,0.0f,5.0f, 
        11.0f,0.0f,5.0f,
        11.0f, 5.0f,5.0f,
        6.0f,5.0f,0.0f,
        6.0f,0.0f,0.0f,
        11.0f,0.0f,0.0f,
        11.0f,5.0f,0.0f
    };
        
    // indices
    GLint indices[] ={
        0, 1, 3, 1, 2, 3, // down  ABCD
        1, 2, 5, 2, 6, 5, // right BCGF
        4, 5, 7, 5, 6, 7, // up    EFGH
        0, 3, 4, 3, 7, 4, // left  ADHE
        0, 1, 5, 5, 4, 0, // front ABFE
        3, 2, 6, 6, 7, 3, // back  DCGH
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
           // "layout(location = 1) in vec3 aColor;\n"
           // "out vec3 ourColor;\n"
            "uniform mat4 projection;\n"
            "uniform mat4 model;\n"
            "uniform mat4 view;\n"
            "void main() {\n"
            "mat4 model = mat4(1.0f);"
           //" ourColor = aColor;\n"
            "   gl_Position = (projection * view * model * vec4(aPosition, 1.0f));\n"
            "}\n";

        const char* fsCode =
            "#version 330 core\n"
            "out vec4 color;\n"
            //"in vec3 ourColor;\n"
            "void main() {\n"
            "   color = vec4(1.0, 0.0,1.0,1.0);\n"
            "}\n";

        _shader.reset(new GLSLProgram);
        _shader->attachVertexShader(vsCode);
        _shader->attachFragmentShader(fsCode);
        _shader->link();
    } catch (const std::exception&) {
        cleanup();
        throw;
    }
    

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::stringstream ss;
        ss << "cube creation failure, (code " << error << ")";
        cleanup();
        throw std::runtime_error(ss.str());
    }
}

Cube::Cube(Cube&& rhs) noexcept
    : _vao(rhs._vao),
      _vbo(rhs._vbo),
      _ebo(rhs._ebo),
      _shader(std::move(rhs._shader)) {
    rhs._vao = 0;
    rhs._vbo = 0;
    rhs._ebo = 0;
}


Cube::~Cube() {
    cleanup();
}

void Cube::draw(const glm::mat4& projection, const glm::mat4& view) {
    glDepthFunc(GL_LEQUAL);
    _shader->use();
  //  _shader->setMat4("view",glm::mat4(glm::mat3(view)));
    _shader->setMat4("view",view);
    _shader->setMat4("projection",projection);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_ebo);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

}

void Cube::cleanup() {
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
