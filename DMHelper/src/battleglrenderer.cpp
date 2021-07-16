#include "battleglrenderer.h"
#include "battledialogmodel.h"
#include "map.h"
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QDebug>

BattleGLRenderer::BattleGLRenderer(BattleDialogModel* model) :
    _model(model),
    _backgroundTexture(0),
    _fowTexture(0),
    _shaderProgram(0),
    _VBO(0),
    _EBO(0)
{
}

void BattleGLRenderer::cleanup()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    if(_VBO > 0)
        f->glDeleteBuffers(1, &_VBO);

    if(_EBO > 0)
        f->glDeleteBuffers(1, &_EBO);
}

/*
static const char *vertexShaderSource =
    "#version 150\n"
    "in vec4 vertex;\n"
    "in vec3 normal;\n"
    "out vec3 vert;\n"
    "out vec3 vertNormal;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "#version 150\n"
    "in highp vec3 vert;\n"
    "in highp vec3 vertNormal;\n"
    "out highp vec4 fragColor;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    "   fragColor = vec4(col, 1.0);\n"
    "}\n";
*/

void BattleGLRenderer::initializeGL()
{
    if((!_model) || (_model->getBackgroundImage().isNull()))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &BattleGLRenderer::cleanup);

    f->glEnable(GL_TEXTURE_2D); // Enable texturing
    f->glEnable(GL_BLEND);// you enable blending function
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    f->glEnable(GL_DEPTH_TEST);
    f->glDepthFunc(GL_GREATER);

    const char *vertexShaderSource = "#version 330 core\n"
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
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShader;
    fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    /*
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    */

    /*
    QSizeF imageSize = _model->getBackgroundImage().size();
    imageSize.scale(_targetSize, Qt::KeepAspectRatio);
    if(imageSize.width() == _targetSize.width())
    {
        imageSize.rwidth() /= _targetSize.width();
        imageSize.rheight() /= _targetSize.width();
    }
    else
    {
        imageSize.rwidth() /= _targetSize.height();
        imageSize.rheight() /= _targetSize.height();
    }
    */

    QSizeF imageSize = _model->getBackgroundImage().size();
    float vertices[] = {
        // positions    // colors           // texture coords
         (float)imageSize.width() / 2,  (float)imageSize.height() / 2, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)imageSize.width() / 2, -(float)imageSize.height() / 2, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)imageSize.width() / 2, -(float)imageSize.height() / 2, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)imageSize.width() / 2,  (float)imageSize.height() / 2, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);
    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //f->glEnableVertexAttribArray(0);
    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);

    // Matrices
    // Model
    int modelLoc = f->glGetUniformLocation(_shaderProgram, "model");
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix.constData());
    // View
    int viewLoc = f->glGetUniformLocation(_shaderProgram, "view");
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix.constData());
    // Projection
    int projectionLoc = f->glGetUniformLocation(_shaderProgram, "projection");
    QMatrix4x4 projectionMatrix;
    imageSize = _model->getBackgroundImage().size();
    imageSize.scale(_targetSize, Qt::KeepAspectRatio);
    projectionMatrix.ortho(-imageSize.width() * 2, imageSize.width() * 2, -imageSize.height() * 2, imageSize.height() * 2, 0.1f, 1000.f);
    f->glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMatrix.constData());

    // Texture
    f->glGenTextures(1, &_backgroundTexture);
    f->glBindTexture(GL_TEXTURE_2D, _backgroundTexture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//_MIPMAP_NEAREST); //GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR);

    // load and generate the background texture
    QImage glBackgroundImage = _model->getBackgroundImage().convertToFormat(QImage::Format_RGBA8888).mirrored();//.scaled(512, 512);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    f->glGenTextures(1, &_fowTexture);
    f->glBindTexture(GL_TEXTURE_2D, _fowTexture);
    Map* map = _model->getMap();
    QImage glFowImage = map->getBWFoWImage().convertToFormat(QImage::Format_RGBA8888).mirrored();//.scaled(512, 512);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glFowImage.width(), glFowImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glFowImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    f->glUseProgram(_shaderProgram);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually


//    f->glMatrixMode(GL_PROJECTION); // Switch to projection matrix
//    f->glLoadIdentity(); // Clear any existing matrix
//    f->glOrtho(0,width(),0,HEIGHT,-1000.0,1000.0);
}

void BattleGLRenderer::resizeGL(int w, int h)
{
    // Update projection matrix and other size related settings:
    //m_projection.setToIdentity();
    //m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);
    _targetSize = QSize(w, h);
    qDebug() << "[BattleGLRenderer] Resize w: " << w << ", h: " << h;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    QSizeF imageSize = _targetSize.scaled(_model->getBackgroundImage().size(), Qt::KeepAspectRatioByExpanding);
    //imageSize.scale(_targetSize, Qt::KeepAspectRatio);
    //float dim = qMax(imageSize.width() / 2.0, imageSize.height() / 2.0);

    // Projection
    int projectionLoc = f->glGetUniformLocation(_shaderProgram, "projection");
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-imageSize.width() / 2, imageSize.width() / 2, -imageSize.height() / 2, imageSize.height() / 2, 0.1f, 1000.f);
    //projectionMatrix.ortho(-imageSize.width(), imageSize.width(), -imageSize.height(), imageSize.height(), 0.1f, 1000.f);
    //projectionMatrix.ortho(-imageSize.width()*2, imageSize.width()*2, -imageSize.height()*2, imageSize.height()*2, 0.1f, 1000.f);
    //projectionMatrix.ortho(-dim, dim, -dim, dim, 0.1f, 1000.f);
    f->glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMatrix.constData());

    /*
    imageSize.rwidth() /= _targetSize.width();
    imageSize.rheight() /= _targetSize.height();

    float vertices[] = {
        // positions    // colors           // texture coords
         (float)imageSize.width(),  (float)imageSize.height(), 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)imageSize.width(), -(float)imageSize.height(), 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)imageSize.width(), -(float)imageSize.height(), 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)imageSize.width(),  (float)imageSize.height(), 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };

    f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    */

}

void BattleGLRenderer::paintGL()
{
    if(!_model)
        return;

    // Draw the scene:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    f->glClearColor(_model->getBackgroundColor().redF(), _model->getBackgroundColor().greenF(), _model->getBackgroundColor().blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);

    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    f->glBindTexture(GL_TEXTURE_2D, _backgroundTexture);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    f->glBindTexture(GL_TEXTURE_2D, _fowTexture);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}


/*
// 0. copy our vertices array in a buffer for OpenGL to use
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
// 1. then set the vertex attributes pointers
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
// 2. use our shader program when we want to render an object
glUseProgram(shaderProgram);
// 3. now draw the object
someOpenGLFunctionThatDrawsOurTriangle();
*/
