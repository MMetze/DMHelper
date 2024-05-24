#include "dmh_opengl.h"

#if DMH_DEBUG_OPENGL

DMH_DEBUG_OPENGL_Singleton* DMH_DEBUG_OPENGL_Singleton::_instance = nullptr;

const GLuint INVALID_PROGRAM = 999999;

DMH_DEBUG_OPENGL_Singleton::DMH_DEBUG_OPENGL_Singleton() :
    _uniformHash(),
    _currentProgram(INVALID_PROGRAM)
{
}

DMH_DEBUG_OPENGL_Singleton::~DMH_DEBUG_OPENGL_Singleton()
{
}

void DMH_DEBUG_OPENGL_Singleton::Initialize()
{
    if(!_instance)
        _instance = new DMH_DEBUG_OPENGL_Singleton();
}

void DMH_DEBUG_OPENGL_Singleton::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

void DMH_DEBUG_OPENGL_Singleton::setProgram(GLuint program)
{
    if(!_instance)
        return;

    _instance->_currentProgram = program;
}

void DMH_DEBUG_OPENGL_Singleton::removeProgram(GLuint program)
{
    if(!_instance)
        return;

    if(_instance->_currentProgram == program)
        _instance->_currentProgram = INVALID_PROGRAM;

    _instance->_uniformHash.remove(program);
}

void DMH_DEBUG_OPENGL_Singleton::registerUniform(GLuint program, GLuint uniform, const char* name)
{
    if(!_instance)
        return;

    if(_instance->_currentProgram != program)
    {
        qDebug() << "[DMH_DEBUG_OPENGL] ERROR: Current program not set to " << program << " when using uniform " << uniform;
        return;
    }

    if(!_instance->_uniformHash.contains(program))
        _instance->_uniformHash.insert(program, QList<GLuint>());

    _instance->_uniformHash[program].append(uniform);
    qDebug() << "[DMH_DEBUG_OPENGL] registerUniform " << uniform << " (" << name << ") for " << program;
}

void DMH_DEBUG_OPENGL_Singleton::checkUniform(GLuint uniform)
{
    if(!_instance)
        return;

    if(_instance->_currentProgram == INVALID_PROGRAM)
    {
        qDebug() << "[DMH_DEBUG_OPENGL] ERROR: Invalid program set when using uniform " << uniform;
        return;
    }

    if((!_instance->_uniformHash.contains(_instance->_currentProgram)) ||
        (!_instance->_uniformHash[_instance->_currentProgram].contains(uniform)))
        qDebug() << "[DMH_DEBUG_OPENGL] ERROR: Uniform " << uniform << " not registered for program " << _instance->_currentProgram;
}

#endif
