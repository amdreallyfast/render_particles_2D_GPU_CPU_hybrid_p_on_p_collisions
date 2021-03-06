#pragma once

#include <string>
#include "glm/vec4.hpp"


/*-----------------------------------------------------------------------------------------------
Description:
    Controls the compute shader that checks for particle collisions within nodes and their 
    neighbors.  There is one shader dispatched for every particle.

Creator:    John Cox (1-21-2017)
-----------------------------------------------------------------------------------------------*/
class ComputeControllerParticleCollisions
{
public:
    ComputeControllerParticleCollisions(unsigned int maxParticles, unsigned int maxNodes, const glm::vec4 &particleRegionCenter, const std::string computeShaderKey);

    // no destructor because there are no buffers that need to be destroyed

    void Update(float deltaTimeSec);

private:
    unsigned int _computeProgramId;
    unsigned int _totalParticles;

    int _unifLocMaxParticles;
    int _unifLocMaxNodes;
    int _unifLocInverseDeltaTimeSec;
    int _unifLoctParticleRegionCenter;
};

