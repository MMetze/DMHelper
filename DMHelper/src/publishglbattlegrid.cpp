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
    _indices(),
    _recreateGrid(false)
{
    _config.copyValues(config);

    createGridObjectsGL();
}

PublishGLBattleGrid::~PublishGLBattleGrid()
{
    PublishGLBattleGrid::cleanup();
}

void PublishGLBattleGrid::cleanup()
{
    cleanupGridGL();

    PublishGLBattleObject::cleanup();
}

void PublishGLBattleGrid::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if((!QOpenGLContext::currentContext()) || (!functions))
        return;

#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid]::paintGL context: " << QOpenGLContext::currentContext();
#endif

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    if(_recreateGrid)
    {
        _recreateGrid = false;
        rebuildGridGL();
    }

#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid]::paintGL UseProgram: " << _shaderProgram << ", context: " << QOpenGLContext::currentContext();
#endif
    functions->glUseProgram(_shaderProgram);
    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, getMatrixData());
    e->glBindVertexArray(_VAO);
    functions->glLineWidth(_config.getGridPen().width());
    functions->glUniform4f(functions->glGetUniformLocation(_shaderProgram, "gridColor"), _config.getGridPen().color().redF(), _config.getGridPen().color().greenF(), _config.getGridPen().color().blueF(), _opacity);

    functions->glDrawElements(GL_LINES, _indices.count(), GL_UNSIGNED_INT, 0);
}

void PublishGLBattleGrid::setPosition(const QPoint& position)
{
    if(_position == position)
        return;

    _position = position;
    updateModelMatrix();
}

void PublishGLBattleGrid::setSize(const QSize& size)
{
    if(_gridSize == size)
        return;

    _gridSize = size;
    rebuildGrid();
}

void PublishGLBattleGrid::setProjectionMatrix(const GLfloat* projectionMatrix)
{
    if(!_shaderProgram)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid]::setProjectionMatrix UseProgram: " << _shaderProgram << ", context: " << QOpenGLContext::currentContext();
#endif
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

    _vertices.append(x0);
    _vertices.append(-y0);
    _vertices.append(0.0f);

    _vertices.append(x1);
    _vertices.append(-y1);
    _vertices.append(0.0f);

    _indices.append((_vertices.count()/3) - 2);
    _indices.append((_vertices.count()/3) - 1);
}

void PublishGLBattleGrid::createGridObjectsGL()
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

#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid]::createGridObjects Program: " << _shaderProgram << ", context: " << QOpenGLContext::currentContext();
#endif
    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");
#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid] Program: " << _shaderProgram << ", model matrix: " << _shaderModelMatrix;
#endif

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());

    rebuildGrid();
}

void PublishGLBattleGrid::rebuildGridGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    cleanupGridGL();

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    Grid* tempGrid = new Grid(nullptr, QRect(QPoint(0, 0), _gridSize.toSize()));
    tempGrid->rebuildGrid(_config, 0, this);
    delete tempGrid;

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid] vertices: " << sizeof(_vertices) << ", _vertices: " << _vertices.count() << ", " << sizeof(_vertices.data()) << ", indices: " << sizeof(_indices) << ", _indices: " << _indices.count() << ", " << sizeof(_indices.data());
    qDebug() << "[PublishGLBattleGrid] glError before buffers: " << f->glGetError();
#endif

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, _vertices.count() * sizeof(float), _vertices.data(), GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.count() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);

#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid] glError after buffers: " << f->glGetError();
#endif

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
}

void PublishGLBattleGrid::cleanupGridGL()
{
#ifdef DEBUG_BATTLE_GRID
    qDebug() << "[PublishGLBattleGrid] Cleaning up image object. VAO: " << _VAO << ", VBO: " << _VBO << ", EBO: " << _EBO << ", texture: " << _textureID;
#endif

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

void PublishGLBattleGrid::rebuildGrid()
{
    _recreateGrid = true;
    emit changed();
}
