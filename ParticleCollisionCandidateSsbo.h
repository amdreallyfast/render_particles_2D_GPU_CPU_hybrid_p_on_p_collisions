#pragma once

#include "SsboBase.h"
#include "ParticleCollisionCandidates.h"
#include <vector>

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates the SSBO that stores Particles.  It generates a chunk of space on the GPU that 
    is big enough to store the requested number of particles, and since this buffer will be used 
    in a drawing shader as well as a compute shader, this class will also up the VAO and the 
    vertex attributes.
Creator:    John Cox, 2-4-2017
-----------------------------------------------------------------------------------------------*/
class ParticleCollisionCandidateSsbo : public SsboBase
{
public:
    ParticleCollisionCandidateSsbo(const ParticleCollisionCandidates *candidateBlocksForEachParticle, unsigned int numBlocks);
    virtual ~ParticleCollisionCandidateSsbo() override = default; // empty override of base destructor
    
    void ConfigureCompute(unsigned int computeProgramId, const std::string &bufferNameInShader) override;
    void ConfigureRender(unsigned int renderProgramId, unsigned int drawStyle) override;
};

