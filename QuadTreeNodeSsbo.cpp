#include "QuadTreeNodeSsbo.h"

#include "ParticleQuadTreeNode.h"
#include "glload/include/glload/gl_4_4.h"



/*-----------------------------------------------------------------------------------------------
Description:
    Calls the base class to give members initial values (zeros).

    Allocates space for the SSBO and dumps the given collection of node data into it.
Parameters:
faceCollection  Self-explanatory
Returns:    None
Creator: John Cox, 1-16-2017
-----------------------------------------------------------------------------------------------*/
QuadTreeNodeSsbo::QuadTreeNodeSsbo(const ParticleQuadTreeNode *nodeCollection, int numNodes) :
    SsboBase()  // generate buffers
{
    // ignore _numVertices because this SSBO does not draw

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    GLuint bufferSizeBytes = sizeof(ParticleQuadTreeNode) * numNodes;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeBytes, nodeCollection, GL_STATIC_DRAW);

    _bufferSizeBytes = bufferSizeBytes;

    // cleanup
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

/*-----------------------------------------------------------------------------------------------
Description:
Binds the SSBO object (a CPU-side thing) to its corresponding buffer in the shader (GPU).

Note: It is ok to call this function for multiple compute shaders so that the same SSBO can
be used in each shader.  No member variables are altered in this function.
Parameters:
computeProgramId    Self-explanatory
Returns:    None
Creator: John Cox, 1-16-2017
-----------------------------------------------------------------------------------------------*/
void QuadTreeNodeSsbo::ConfigureCompute(unsigned int computeProgramId, const std::string &bufferNameInShader)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);

    // see the corresponding area in ParticleSsbo::Init(...) for explanation
    // Note: MUST use the same binding point 

    //GLuint ssboBindingPointIndex = 13;   // or 1, or 5, or 17, or wherever IS UNUSED
    GLuint storageBlockIndex = glGetProgramResourceIndex(computeProgramId, GL_SHADER_STORAGE_BLOCK, bufferNameInShader.c_str());
    glShaderStorageBlockBinding(computeProgramId, storageBlockIndex, _ssboBindingPointIndex);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _ssboBindingPointIndex, _bufferId);

    // cleanup
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

/*-----------------------------------------------------------------------------------------------
Description:
    The SSBO for quad tree nodes does not draw.  The geometry for the tree's nodes will be put 
    into a PolygonSsbo.
Parameters:
    irrelevant
Returns:    None
Creator: John Cox, 1-16-2017
-----------------------------------------------------------------------------------------------*/
void QuadTreeNodeSsbo::ConfigureRender(unsigned int, unsigned int)
{
}
