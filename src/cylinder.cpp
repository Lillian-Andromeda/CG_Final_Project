#include "cylinder.h"

Cylinder::Cylinder() {
	float PI = 3.1415926f;
    //顶点坐标
    GLfloat vertices[60000];
    
    //索引坐标
    GLint indices[30000];
    //设置顶点坐标
    GLfloat p = 0.0f, r = 3.0f;
    for (GLint i = 0; i < 20000; i += 2){
       p = i * PI / 180;
       vertices[i * 3] = cos(p) * r;
       vertices[i * 3 + 1] = 0.0f;
       vertices[i * 3 + 2] = sin(p) * r;

       vertices[(i + 1) * 3] = cos(p) * r;
       vertices[(i + 1) * 3 + 1] = 5.0f;
       vertices[(i + 1) * 3 + 2] = sin(p) * r;
    }
    //设置索引坐标
    int t = 0;
    for (GLint i = 0; i < 10000; i++){
       indices[3*i] = t;
       indices[3 * i + 1] = t + 1;
       indices[3 * i + 2] = t + 2;
       t++;

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
            "   color = vec4(1.0, 1.0, 0.0, 1.0);\n"
            "}\n";

        _shader.reset(new GLSLProgram);
        _shader->attachVertexShader(vsCode);
        _shader->attachFragmentShader(fsCode);
        _shader->link();
    } catch (const std::exception&) {
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

Cylinder ::Cylinder(Cylinder&& rhs) noexcept
    : _vao(rhs._vao),
      _vbo(rhs._vbo),
      _ebo(rhs._ebo),
      _shader(std::move(rhs._shader)) {
    rhs._vao = 0;
    rhs._vbo = 0;
    rhs._ebo = 0;
}


Cylinder::~Cylinder() {
    cleanup();
}

void Cylinder::draw(const glm::mat4& projection, const glm::mat4& view) {
    glDepthFunc(GL_LEQUAL);
    _shader->use();
    _shader->setMat4("view",view);
    _shader->setMat4("projection",projection);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_ebo);
    glDrawElements(GL_TRIANGLES, 10000, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

}

void Cylinder::cleanup() {
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
