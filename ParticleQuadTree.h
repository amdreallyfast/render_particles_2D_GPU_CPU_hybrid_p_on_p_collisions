#pragma once

#include <vector>
#include "ParticleQuadTreeNode.h"
#include "Particle.h"
#include "glm/vec4.hpp"


// TODO: forward declare ParticleQuadTreeNode

/*-----------------------------------------------------------------------------------------------
Description:
    Starts up the quad tree that will contain the particles.  The constructor initializes the 
    values for all the nodes, in particular calculating each node's edges and neighbors.  After 
    that, it is a dumb structure.  It is meant to be used as setup before uploading the quad 
    tree into an SSBO.
    
    Note: I considered and heavily entertained the idea of starting every frame with one node
    and subdividing as necessary, but I abandoned that idea because there will be many particles
    in all but the initial frames.  It took some extra calculations up front, but the initial
    subdivision should cut down on the subdivisions that are needed on every frame.
Creator:    John Cox (1-10-2017)
-----------------------------------------------------------------------------------------------*/
class ParticleQuadTree
{
public:
    ParticleQuadTree(const glm::vec4 &particleRegionCenter, float particleRegionRadius);

    void ResetTree();
    void AddParticlestoTree(Particle *particleCollection, int numParticles);
    const ParticleQuadTreeNode *CurrentBuffer() const;

    unsigned int NumActiveNodes() const;
    int NumNodePopulations() const;
    void ResetNumNodePopulations();

public:
    static const int MAX_NODES = 256 * 256;

private:
    bool AddParticleToNode(int particleIndex, int nodeIndex);
    bool SubdivideNode(int nodeIndex);

    enum FIRST_FOUR_NODE_INDEXES
    {
        TOP_LEFT = 0,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        NUM_STARTING_NODES
    };

    int _completedNodePopulations;


    int _numActiveNodes;
    ParticleQuadTreeNode _allNodes[MAX_NODES];
    glm::vec4 _particleRegionCenter;
    float _particleRegionRadius;

    Particle _localParticleArray[Particle::MAX_PARTICLES];
};
