#include "ball.h"

Ball::Ball(){
    int nSlices = 50; // 经线
    int nStacks = 50; // 纬线
    int nVerts = (nStacks + 1) * (nSlices + 1);
    int elements = nSlices * nStacks * 6;
    float theta, phi;
    float thetaFac = 3.14f * 2.0f / nSlices;
    float phiFac = 3.14f * 1.0f / nStacks;
    float nx, ny, nz;
    int idx = 0;
    GLfloat vertices[51 * 51 * 3];//顶点
	GLint indices[50 * 50 * 6];//索引
    for (int i = 0; i <= nSlices; i++)
    {
        theta = i * thetaFac;
        for (int j = 0; j <= nStacks; j++)
        {
            phi = j * phiFac;
            nx = sinf(phi) * cosf(theta);
            ny = sinf(phi) * sinf(theta);
            nz = cosf(phi);
            vertices[idx * 3] = 1.0f * nx ;
            vertices[idx * 3 + 1] = 1.0f * ny + 11;
            vertices[idx * 3 + 2] = 1.0f * nz ;
            idx++;
        }
    }
    // 定义球体坐标索引
    int indx = 0;
    for (int i = 0; i < nStacks; i++)
    {
        for (int j = 0; j < nSlices; j++)
        {
            int i0 = i * (nSlices + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (nSlices + 1);
            int i3 = i2 + 1;
            if ((j + i) % 2 == 1) {
                indices[indx] = i0;
                indices[indx + 1] = i2;
                indices[indx + 2] = i1;
                indices[indx + 3] = i1;
                indices[indx + 4] = i2;
                indices[indx + 5] = i3;
                indx += 6;
            }
            else
            {
                indices[indx] = i0;
                indices[indx + 1] = i2;
                indices[indx + 2] = i3;
                indices[indx + 3] = i0;
                indices[indx + 4] = i3;
                indices[indx + 5] = i1;
                indx += 6;
            }
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
 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
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
            "   color = vec4(0.0, 0.0, 1.0,1.0);\n"
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

Ball::Ball(Ball&& rhs) noexcept
    : _vao(rhs._vao),
      _vbo(rhs._vbo),
      _ebo(rhs._ebo),
      _shader(std::move(rhs._shader)) {
    rhs._vao = 0;
    rhs._vbo = 0;
    rhs._ebo = 0;
}


Ball::~Ball() {
    cleanup();
}

void Ball::draw(const glm::mat4& projection, const glm::mat4& view) {
    glDepthFunc(GL_LEQUAL);
    _shader->use();
    _shader->setMat4("view",view);
    _shader->setMat4("projection",projection);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_ebo);
    glDrawElements(GL_TRIANGLES, 15000 , GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

}

void Ball::cleanup() {
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

    

