
#include "ParticleCollisionCandidateSsbo.h"

#include "glload/include/glload/gl_4_4.h"


/*-----------------------------------------------------------------------------------------------
Description:
    Ensures that the object starts object with initialized values and that the buffer's size is 
    established.
Parameters: 
    candidateBlocksForEachParticle  A collection of particle candidates, one object for each 
        particle.
    numBlocks   Number of ParticleCollisionCandidates objects in the collection
Returns:    None
Creator:    John Cox, 2-4-2017
-----------------------------------------------------------------------------------------------*/
ParticleCollisionCandidateSsbo::ParticleCollisionCandidateSsbo(const ParticleCollisionCandidates *candidateBlocksForEachParticle, unsigned int numBlocks) :
    SsboBase()  // generate buffers
{
    // ignore _numVertices because this SSBO does not draw

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    GLuint bufferSizeBytes = sizeof(ParticleCollisionCandidates) * numBlocks;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeBytes, candidateBlocksForEachParticle, GL_STATIC_DRAW);

    _bufferSizeBytes = bufferSizeBytes;

    // cleanup
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Binds the SSBO object (a CPU-side thing) to its corresponding buffer in the shader (GPU).
Parameters: 
    computeProgramId    Self-explanatory
Returns:    None
Creator:    John Cox, 2-4-2017
-----------------------------------------------------------------------------------------------*/
void ParticleCollisionCandidateSsbo::ConfigureCompute(unsigned int computeProgramId, const std::string &bufferNameInShader)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _bufferId);
    GLuint storageBlockIndex = glGetProgramResourceIndex(computeProgramId, GL_SHADER_STORAGE_BLOCK, bufferNameInShader.c_str());
    glShaderStorageBlockBinding(computeProgramId, storageBlockIndex, _ssboBindingPointIndex);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, _ssboBindingPointIndex, _bufferId);

    // cleanup
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

/*-----------------------------------------------------------------------------------------------
Description:
    The SSBO for particle collision candidate blocks does not draw.  It is used for particle 
    collision detection only.
Parameters:
    irrelevant
Returns:    None
Creator:    John Cox, 2-4-2017
-----------------------------------------------------------------------------------------------*/
void ParticleCollisionCandidateSsbo::ConfigureRender(unsigned int, unsigned int )
{

}

