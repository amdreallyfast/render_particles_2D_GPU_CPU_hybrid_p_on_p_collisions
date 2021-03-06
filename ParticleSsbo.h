#pragma once

#include "SsboBase.h"
#include "Particle.h"
#include <vector>

/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates the SSBO that stores Particles.  It generates a chunk of space on the GPU that 
    is big enough to store the requested number of particles, and since this buffer will be used 
    in a drawing shader as well as a compute shader, this class will also up the VAO and the 
    vertex attributes.
Creator:    John Cox (9-3-2016)
-----------------------------------------------------------------------------------------------*/
class ParticleSsbo : public SsboBase
{
public:
    ParticleSsbo(const std::vector<Particle> &allParticles);
    virtual ~ParticleSsbo() override = default; // empty override of base destructor
    
    void ConfigureCompute(unsigned int computeProgramId, const std::string &bufferNameInShader) override;
    void ConfigureRender(unsigned int renderProgramId, unsigned int drawStyle) override;
};

