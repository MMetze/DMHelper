#include "publishglbattlegrid.h"
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QDebug>

#include <QImage>


PublishGLBattleGrid::PublishGLBattleGrid(const GridConfig& config, qreal opacity, const QSizeF& gridSize) :
    PublishGLBattleObject(),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _shaderProgram(0),
    _config(),
    _opacity(opacity),
    _gridSize(gridSize),
    _vertices(),
    _indices()
{
    _config.copyValues(config);

    createGridObjects();
}

PublishGLBattleGrid::~PublishGLBattleGrid()
{
    PublishGLBattleGrid::cleanup();
}

void PublishGLBattleGrid::cleanup()
{
    cleanupGrid();

    PublishGLBattleObject::cleanup();
}

void PublishGLBattleGrid::paintGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    //qDebug() << "[PublishGLBattleGrid]::paintGL context: " << QOpenGLContext::currentContext();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    /*
    GLint previousProgram = 0;
    f->glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);

    f->glUseProgram(_shaderProgram);

    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, getMatrixData());
    e->glBindVertexArray(_VAO);
    f->glDrawElements(GL_LINES, 37, GL_UNSIGNED_INT, 0);

    f->glUseProgram(previousProgram);
    */

    //qDebug() << "[PublishGLBattleGrid]::paintGL UseProgram: " << _shaderProgram << ", context: " << QOpenGLContext::currentContext();
    f->glUseProgram(_shaderProgram);
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, getMatrixData());
    e->glBindVertexArray(_VAO);
    f->glLineWidth(_config.getGridPen().width());
    f->glUniform4f(f->glGetUniformLocation(_shaderProgram, "gridColor"), _config.getGridPen().color().redF(), _config.getGridPen().color().greenF(), _config.getGridPen().color().blueF(), _opacity);

//    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    //f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    f->glDrawElements(GL_LINES, _indices.count(), GL_UNSIGNED_INT, 0);

}

void PublishGLBattleGrid::setPosition(const QPoint& position)
{
    if(_position == position)
        return;

    _position = position;
    updateModelMatrix();
}

void PublishGLBattleGrid::setProjectionMatrix(const GLfloat* projectionMatrix)
{
    if(!_shaderProgram)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    //qDebug() << "[PublishGLBattleGrid]::setProjectionMatrix UseProgram: " << _shaderProgram << ", context: " << QOpenGLContext::currentContext();
    f->glUseProgram(_shaderProgram);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix);
}

void PublishGLBattleGrid::setConfig(const GridConfig& config)
{
    _config.copyValues(config);
    rebuildGrid();
}

void PublishGLBattleGrid::setOpacity(qreal opacity)
{
    if(_opacity == opacity)
        return;

    _opacity = opacity;
}

void PublishGLBattleGrid::addLine(int x0, int y0, int x1, int y1, int zOrder)
{
    Q_UNUSED(zOrder);

    /*
    _vertices.append(x0 - (_gridSize.width() / 2.f));
    _vertices.append((_gridSize.height() / 2.f) - y0);
    _vertices.append(0.0f);

    _vertices.append(x1 - (_gridSize.width() / 2.f));
    _vertices.append((_gridSize.height() / 2.f) - y1);
    _vertices.append(0.0f);
    */

    _vertices.append(x0);
    _vertices.append(-y0);
    _vertices.append(0.0f);

    _vertices.append(x1);
    _vertices.append(-y1);
    _vertices.append(0.0f);

    _indices.append((_vertices.count()/3) - 2);
    _indices.append((_vertices.count()/3) - 1);
}

void PublishGLBattleGrid::createGridObjects()
{
    if(!QOpenGLContext::currentContext())
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;





    const char *vertexShaderSource = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform vec4 gridColor;\n;"
        "out vec4 ourColor; // output a color to the fragment shader\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "   ourColor = gridColor; // set ourColor to the input color we got from the vertex data\n"
        "}\0";

/*
    const char *vertexShaderSource = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 ourColor; // output a color to the fragment shader\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = aColor; // set ourColor to the input color we got from the vertex data\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";
*/

    unsigned int vertexShader;
    vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleGrid] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec4 ourColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = ourColor;\n"
        "}\0";
    /*
    const char *fragmentShaderSource = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
        "}\0";
    */

//    "    FragColor = texture(texture1, TexCoord);\n"
//    "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"

    unsigned int fragmentShader;
    fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleGrid] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleGrid] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    //qDebug() << "[PublishGLBattleGrid]::createGridObjects Program: " << _shaderProgram << ", context: " << QOpenGLContext::currentContext();
    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");
    //qDebug() << "[PublishGLBattleGrid] Program: " << _shaderProgram << ", model matrix: " << _shaderModelMatrix;

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());

    rebuildGrid();

    /*
    if(!QOpenGLContext::currentContext())
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    float vertices[] = {
        // positions              // colors
         0.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       100.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       200.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       300.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       400.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       500.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       600.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       700.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       800.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       900.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
      1000.0f,    0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       100.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       200.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       300.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       400.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       500.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       600.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       700.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       800.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       900.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
      1000.0f,  100.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       100.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       200.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       300.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       400.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       500.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       600.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       700.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       800.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       900.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
      1000.0f,  200.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       100.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       200.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       300.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       400.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       500.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       600.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       700.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       800.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
       900.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f,
      1000.0f,  300.0f, 0.0f,      1.0f, 0.0f, 0.0f
    };

    unsigned int indices[] =
    {   // note that we start from 0!
         0,  1,
         1,  2,
         2,  3,
         3,  4,
         4,  5,
         5,  6,
         6,  7,
         7,  8,
         8,  9,
         9, 10,
        11, 12,
        12, 13,
        13, 14,
        14, 15,
        15, 16,
        16, 17,
        17, 18,
        18, 19,
        19, 20,
        20, 21,
        22, 23,
        23, 24,
        24, 25,
        25, 26,
        26, 27,
        27, 28,
        28, 29,
        29, 30,
        30, 31,
        31, 32,
        33, 34,
        34, 35,
        35, 36,
        36, 37,
        37, 38,
        38, 39,
        39, 40
    };

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);




    const char *vertexShaderSource = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 ourColor; // output a color to the fragment shader\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = aColor; // set ourColor to the input color we got from the vertex data\n"
        "}\0";

    unsigned int vertexShader;
    vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleGrid] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShader;
    fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleGrid] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleGrid] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());

    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    */
}

void PublishGLBattleGrid::rebuildGrid()
{
    if(!QOpenGLContext::currentContext())
        return;

    cleanupGrid();

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    //QImage image("C:/Users/turne/Documents/DnD/DM Helper/testdata/Phandalin.png");
    //QSize imageSize = image.size();

    /*
    float vertices[] = {
        // positions                                                   // colors           // texture coords
         (float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)image.width() / 2, -(float)image.height() / 2, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)image.width() / 2, -(float)image.height() / 2, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    */
    /*
    float vertices[] = {
        // positions                                                   // colors           // texture coords
         static_cast<float>(_gridSize.width() / 2.0),  static_cast<float>(_gridSize.height() / 2.0), 0.0f,   //1.0f, 0.0f, 0.0f,   // top right
         static_cast<float>(_gridSize.width() / 2.0), -static_cast<float>(_gridSize.height() / 2.0), 0.0f,   //0.0f, 1.0f, 0.0f,   // bottom right
        -static_cast<float>(_gridSize.width() / 2.0), -static_cast<float>(_gridSize.height() / 2.0), 0.0f,   //0.0f, 0.0f, 1.0f,   // bottom left
        -static_cast<float>(_gridSize.width() / 2.0),  static_cast<float>(_gridSize.height() / 2.0), 0.0f//,   1.0f, 1.0f, 0.0f    // top left
    };
    */


    /*
    float vertices[] = {
        // positions
         0.0f,    0.0f, 0.0f,
       100.0f,    0.0f, 0.0f,
       200.0f,    0.0f, 0.0f,
       300.0f,    0.0f, 0.0f,
       400.0f,    0.0f, 0.0f,
       500.0f,    0.0f, 0.0f,
       600.0f,    0.0f, 0.0f,
       700.0f,    0.0f, 0.0f,
       800.0f,    0.0f, 0.0f,
       900.0f,    0.0f, 0.0f,
      1000.0f,    0.0f, 0.0f,
         0.0f,  100.0f, 0.0f,
       100.0f,  100.0f, 0.0f,
       200.0f,  100.0f, 0.0f,
       300.0f,  100.0f, 0.0f,
       400.0f,  100.0f, 0.0f,
       500.0f,  100.0f, 0.0f,
       600.0f,  100.0f, 0.0f,
       700.0f,  100.0f, 0.0f,
       800.0f,  100.0f, 0.0f,
       900.0f,  100.0f, 0.0f,
      1000.0f,  100.0f, 0.0f,
         0.0f,  200.0f, 0.0f,
       100.0f,  200.0f, 0.0f,
       200.0f,  200.0f, 0.0f,
       300.0f,  200.0f, 0.0f,
       400.0f,  200.0f, 0.0f,
       500.0f,  200.0f, 0.0f,
       600.0f,  200.0f, 0.0f,
       700.0f,  200.0f, 0.0f,
       800.0f,  200.0f, 0.0f,
       900.0f,  200.0f, 0.0f,
      1000.0f,  200.0f, 0.0f,
         0.0f,  300.0f, 0.0f,
       100.0f,  300.0f, 0.0f,
       200.0f,  300.0f, 0.0f,
       300.0f,  300.0f, 0.0f,
       400.0f,  300.0f, 0.0f,
       500.0f,  300.0f, 0.0f,
       600.0f,  300.0f, 0.0f,
       700.0f,  300.0f, 0.0f,
       800.0f,  300.0f, 0.0f,
       900.0f,  300.0f, 0.0f,
      1000.0f,  300.0f, 0.0f,
    };
    */


    /*
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    */
    /*
    unsigned int indices[] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        0, 2,
        1, 3
    };
    */

    /*
    unsigned int indices[] =
    {   // note that we start from 0!
         0,  1,
         1,  2,
         2,  3,
         3,  4,
         4,  5,
         5,  6,
         6,  7,
         7,  8,
         8,  9,
         9, 10,
        11, 12,
        12, 13,
        13, 14,
        14, 15,
        15, 16,
        16, 17,
        17, 18,
        18, 19,
        19, 20,
        20, 21,
        22, 23,
        23, 24,
        24, 25,
        25, 26,
        26, 27,
        27, 28,
        28, 29,
        29, 30,
        30, 31,
        31, 32,
        33, 34,
        34, 35,
        35, 36,
        36, 37,
        37, 38,
        38, 39,
        39, 40
    };
    */

    Grid* tempGrid = new Grid(nullptr, QRect(QPoint(0, 0), _gridSize.toSize()));
    tempGrid->rebuildGrid(_config, 0, this);
    delete tempGrid;

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

//    qDebug() << "[PublishGLBattleGrid] vertices: " << sizeof(vertices) << ", _vertices: " << _vertices.count() << ", " << sizeof(_vertices.data()) << ", indices: " << sizeof(indices) << ", _indices: " << _indices.count() << ", " << sizeof(_indices.data());
//    qDebug() << "[PublishGLBattleGrid] glError before buffers: " << f->glGetError();

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, _vertices.count() * sizeof(float), _vertices.data(), GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.count() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);

//    qDebug() << "[PublishGLBattleGrid] glError after buffers: " << f->glGetError();

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);

    // color attribute
    //f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    //f->glEnableVertexAttribArray(1);
    // texture attribute
    //f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //f->glEnableVertexAttribArray(2);

    // Texture
    //f->glGenTextures(1, &_textureID);
    //f->glBindTexture(GL_TEXTURE_2D, _textureID);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    //f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load and generate the background texture
    //QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    //f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    //f->glGenerateMipmap(GL_TEXTURE_2D);
}

void PublishGLBattleGrid::cleanupGrid()
{
    //qDebug() << "[PublishGLBattleGrid] Cleaning up image object. VAO: " << _VAO << ", VBO: " << _VBO << ", EBO: " << _EBO << ", texture: " << _textureID;

    if(QOpenGLContext::currentContext())
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
        if(_VAO > 0)
        {
            if(e)
                e->glDeleteVertexArrays(1, &_VAO);
            _VAO = 0;
        }

        if(_VBO > 0)
        {
            if(f)
                f->glDeleteBuffers(1, &_VBO);
            _VBO = 0;
        }

        if(_EBO > 0)
        {
            if(f)
                f->glDeleteBuffers(1, &_EBO);
            _EBO = 0;
        }
    }

    _vertices.clear();
    _indices.clear();
}

void PublishGLBattleGrid::updateModelMatrix()
{
    _modelMatrix.setToIdentity();
    _modelMatrix.translate(_position.x(),
                           _position.y());
}
