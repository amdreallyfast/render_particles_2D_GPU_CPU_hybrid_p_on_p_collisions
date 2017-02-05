#pragma once

#include <memory>

#include "ParticleQuadTreeNode.h"


// TODO: header
// 2-4-2017
struct ParticleCollisionCandidates
{
    // TODO: header
    // 2-4-2017
    ParticleCollisionCandidates()
    {
        _numCandidates = 0;
        memset(_candidateIndexes, 0, sizeof(int) * MAX_CANDIDATES_PER_PARTICLE);
    }

    // be simple and just use the total number of particles per node for all nine nodes (a particle's "home" node + 8 neighbors)
    static const unsigned int MAX_CANDIDATES_PER_PARTICLE = ParticleQuadTreeNode::MAX_PARTICLES_PER_NODE * 9;

    unsigned int _numCandidates;
    unsigned int _candidateIndexes[MAX_CANDIDATES_PER_PARTICLE];
};