#include "SsboBase.h"

#include "glload/include/glload/gl_4_4.h"

/*-----------------------------------------------------------------------------------------------
Description:
    A helper function so that I don't have to hard-code a unique binding point index for each 
    SSBO.
Parameters: None
Returns:    None
Creator: John Cox, 1-16-2017
-----------------------------------------------------------------------------------------------*/
static unsigned int GetNewStorageBlockBindingPointIndex()
{
    static GLuint ssboBindingPointIndex = 0;

    return ssboBindingPointIndex++;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Gives members default values.

    Note: As part of the Resource Acquisition is Initialization (RAII) approach that I am trying 
    to use in this program, the SSBOs are generated here.  This means that the OpenGL context 
    MUST be started up prior to initialization.  Prior to OpenGL context instantiation, any 
    gl*(...) function calls will throw an exception.

    The SSBO is linked up with the compute shader in ConfigureCompute(...).
    The VAO is initialized in ConfigureRender(...).

Parameters: None
Returns:    None
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
SsboBase::SsboBase() :
    _vaoId(0),
    _bufferId(0),
    _drawStyle(0),
    _numVertices(0),
    _ssboBindingPointIndex(GetNewStorageBlockBindingPointIndex())
{
    glGenBuffers(1, &_bufferId);
    glGenVertexArrays(1, &_vaoId);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Cleans up the buffer and VAO.  If either is 0, then the glDelete*(...) call silently does 
    nothing.
Parameters: None
Returns:    None
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
SsboBase::~SsboBase()
{
    glDeleteBuffers(1, &_bufferId);
    glDeleteVertexArrays(1, &_vaoId);
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the vertex array object ID.
Parameters: None
Returns:    
    A copy of the VAO's ID.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::VaoId() const
{
    return _vaoId;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the vertex buffer object's ID.
Parameters: None
Returns:
    A copy of the VBO's ID.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::BufferId() const
{
    return _bufferId;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the draw style for the contained object (GL_TRIANGLES, GL_LINES, 
    GL_POINTS, etc.)
Parameters: None
Returns:
    A copy of the draw style GLenum.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::DrawStyle() const
{
    return _drawStyle;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the number of vertices in the SSBO.  Used in the glDrawArrays(...) call.
Parameters: None
Returns:
    See description.
Creator: John Cox, 9-20-2016
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::NumVertices() const
{
    return _numVertices;
}

/*-----------------------------------------------------------------------------------------------
Description:
    A simple getter for the buffer size in bytes.  Used when downloading data from the GPU or 
    uploading data to it via glBufferSubData(...) or a mapping.
Parameters: None
Returns:
    See description.
Creator: John Cox, 1-28-2017
-----------------------------------------------------------------------------------------------*/
unsigned int SsboBase::BufferSizeBytes() const
{
    return _bufferSizeBytes;
}