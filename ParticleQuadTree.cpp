#include "ParticleQuadTree.h"
#include "Particle.h"

/*-----------------------------------------------------------------------------------------------
Description:
    Sets up the initial tree subdivision with 2 rows and 2 columns.  Boundaries are determined 
    by the particle region center and the particle region radius.  The ParticleUpdater should 
    constrain particles to this region, and the quad tree will subdivide within this region.
Parameters:
    particleRegionCenter    In world space
    particleRegionRadius    In world space
Returns:    None
Exception:  Safe
Creator:    John Cox (12-17-2016)
-----------------------------------------------------------------------------------------------*/
ParticleQuadTree::ParticleQuadTree(const glm::vec4 &particleRegionCenter, float particleRegionRadius) :
    _completedNodePopulations(0),
    _particleRegionCenter(particleRegionCenter),
    _particleRegionRadius(particleRegionRadius)
{
    float particleRegionLeft = _particleRegionCenter.x - _particleRegionRadius;
    float particleRegionRight = _particleRegionCenter.x + _particleRegionRadius;
    float particleRegionTop = _particleRegionCenter.y + _particleRegionRadius;
    float particleRegionBottom = _particleRegionCenter.y - _particleRegionRadius;

    // top left
    // Note: Using brackets because I want to reuse the name "node".
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::TOP_LEFT];
        node._inUse = true;
        node._leftEdge = particleRegionLeft;
        node._rightEdge = particleRegionCenter.x;
        node._topEdge = particleRegionTop;
        node._bottomEdge = particleRegionCenter.y;
        // no left neighbor
        // no top left neighbor
        // no top neighbor
        // no top right neighbor
        node._neighborIndexRight = FIRST_FOUR_NODE_INDEXES::TOP_RIGHT;
        node._neighborIndexBottomRight = FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT;
        node._neighborIndexBottom = FIRST_FOUR_NODE_INDEXES::BOTTOM_LEFT;
        // no bottom left neighbor
    }

    // top right
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::TOP_RIGHT];
        node._inUse = true;
        node._leftEdge = particleRegionCenter.x;
        node._rightEdge = particleRegionRight;
        node._topEdge = particleRegionTop;
        node._bottomEdge = particleRegionCenter.y;
        node._neighborIndexLeft = FIRST_FOUR_NODE_INDEXES::TOP_LEFT;
        // no top left neighbor
        // no top neighbor
        // no top right neighbor
        // no right neighbor
        // no bottom right neighbor
        node._neighborIndexBottom = FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT;
        node._neighborIndexBottomLeft = FIRST_FOUR_NODE_INDEXES::BOTTOM_LEFT;
    }

    // bottom left
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::BOTTOM_LEFT];
        node._inUse = true;
        node._leftEdge = particleRegionLeft;
        node._rightEdge = particleRegionCenter.x;
        node._topEdge = particleRegionCenter.y;
        node._bottomEdge = particleRegionBottom;
        // no left neighbor
        // no top left neighbor
        node._neighborIndexTop = FIRST_FOUR_NODE_INDEXES::TOP_LEFT;
        node._neighborIndexTopRight = FIRST_FOUR_NODE_INDEXES::TOP_RIGHT;
        node._neighborIndexRight = FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT;
        // no bottom right neighbor
        // no bottom neighbor
        // no bottom left neighbor
    }

    // bottom right
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT];
        node._inUse = true;
        node._leftEdge = particleRegionCenter.x;
        node._rightEdge = particleRegionRight;
        node._topEdge = particleRegionCenter.y;
        node._bottomEdge = particleRegionBottom;
        node._neighborIndexLeft;
        node._neighborIndexTopLeft;
        node._neighborIndexTop;
        // no top left neighbor
        // no right neighbor
        // no bottom right neighbor
        // no bottom neighbor
        // no bottom left neighbor
    }

    _numActiveNodes = FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Runs through the node array and resets every single node except those in the initial 
    subdivision.

    Note: Rather than have a loop that runs through every single node and resets them one by 
    one, this method makes use spelling out the first four nodes' resets manually, and then a 
    memset(...) for the rest.
Parameters: None
Returns:    None
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleQuadTree::ResetTree()
{
    _numActiveNodes = FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES;

    // top left
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::TOP_LEFT];
        node._numCurrentParticles = 0;
        node._isSubdivided = 0;
        node._childNodeIndexTopLeft = -1;
        node._childNodeIndexTopRight = -1;
        node._childNodeIndexBottomLeft = -1;
        node._childNodeIndexBottomRight = -1;
    }

    // top right
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::TOP_RIGHT];
        node._numCurrentParticles = 0;
        node._isSubdivided = 0;
        node._childNodeIndexTopLeft = -1;
        node._childNodeIndexTopRight = -1;
        node._childNodeIndexBottomLeft = -1;
        node._childNodeIndexBottomRight = -1;
    }

    // bottom left
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::BOTTOM_LEFT];
        node._numCurrentParticles = 0;
        node._isSubdivided = 0;
        node._childNodeIndexTopLeft = -1;
        node._childNodeIndexTopRight = -1;
        node._childNodeIndexBottomLeft = -1;
        node._childNodeIndexBottomRight = -1;
    }

    // bottom right
    {
        ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT];
        node._numCurrentParticles = 0;
        node._isSubdivided = 0;
        node._childNodeIndexTopLeft = -1;
        node._childNodeIndexTopRight = -1;
        node._childNodeIndexBottomLeft = -1;
        node._childNodeIndexBottomRight = -1;
    }

    // wipe out the rest of the nodes
    ParticleQuadTreeNode *startHere = &_allNodes[FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES];
    size_t numBytes = sizeof(ParticleQuadTreeNode) * (MAX_NODES - FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES);
    memset(startHere, 0, numBytes);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Governs the addition of particles to the quad tree.  AddParticleToNode(...) will 
    handle subdivision and addition of particles to child nodes.
Parameters: 
    particleCollection  An updated particle array.
Returns:    None
Exception:  Safe
Creator:    John Cox (12-17-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleQuadTree::AddParticlestoTree(Particle *particleCollection, int numParticles)
{
    // numParticles should be no larger that Particle::MAX_PARTICLES; if it is; the crash is 
    // deserved :)
    memcpy(_localParticleArray, particleCollection, sizeof(Particle) * numParticles);

    for (size_t particleIndex = 0; particleIndex < numParticles; particleIndex++)
    {
        Particle &p = _localParticleArray[particleIndex];
        if (p._isActive == 0)
        {
            // only add active particles
            continue;
        }

        // TODO: try replacing with index calculation and trinary operators

        // if the particle is outside the particle region, then it would be inactive
        int nodeIndex = -1;
        if (p._position.y < _particleRegionCenter.y)
        {
            // bottom
            if (p._position.x < _particleRegionCenter.x)
            {
                // left
                nodeIndex = FIRST_FOUR_NODE_INDEXES::BOTTOM_LEFT;
            }
            else
            {
                // right
                nodeIndex = FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT;
            }
        }
        else
        {
            // top
            if (p._position.x < _particleRegionCenter.x)
            {
                // left
                nodeIndex = FIRST_FOUR_NODE_INDEXES::TOP_LEFT;
            }
            else
            {
                // right
                nodeIndex = FIRST_FOUR_NODE_INDEXES::TOP_RIGHT;
            }
        }

        AddParticleToNode(particleIndex, nodeIndex);
    }

    _completedNodePopulations++;

}

/*-----------------------------------------------------------------------------------------------
Description:
    Returns a pointer to the most-recently completed quad tree buffer.  This is used when 
    uploading the the results back to the GPU.
Parameters: None
Returns:    
    A pointer to said buffer.
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
const ParticleQuadTreeNode *ParticleQuadTree::CurrentBuffer() const
{
    return _allNodes;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Returns the number of nodes that were in the use during the most-recently completed quad 
    tree.  This is used when uploading the the results back to the GPU.
Parameters: None
Returns:    
    A pointer to said buffer.
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
unsigned int ParticleQuadTree::NumActiveNodes() const
{
    return _numActiveNodes;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Returns the number of quad trees that have been completed since the last call to 
    ResetNumNodePopulations().  Helpful for calculating the rate that the quad tree is 
    generated.
Parameters: None
Returns:    
    See description.
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
int ParticleQuadTree::NumNodePopulations() const
{
    return _completedNodePopulations;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Resets the count for how many times the quad tree has been generated.  
Parameters: None
Returns:    None
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleQuadTree::ResetNumNodePopulations()
{
    _completedNodePopulations = 0;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Adds a single particle to a single node.  The particle collection has to come along for the 
    ride because of the risk of particle addition causing the node to subdivide.
Parameters: 
    particleIndex   Self-explanatory.
    nodeIndex       Self-explanatory
Returns:    
    A pointer to said buffer.
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
bool ParticleQuadTree::AddParticleToNode(int particleIndex, int nodeIndex)
{
    // don't bother checking the pointer; if a null pointer was passed, just crash
    ParticleQuadTreeNode &node = _allNodes[nodeIndex];

    if (node._numCurrentParticles == ParticleQuadTreeNode::MAX_PARTICLES_PER_NODE)
    {
        if (!SubdivideNode(nodeIndex))
        {
            // subdivision failed (ran out of nodes) and cannot add particle
            return false;
        }
    }

    // TODO: attempt performance boost by checking if (!subdivided) to see if the CPU assumes that the condition is true
    if (node._isSubdivided)
    {
        // don't need to check >left && < right && >bottom && <top because the first calculations already determined that the particle was (1) within bounds (read, "active") and (2) within one of the first nodes, so every following quadrant check just needs to check against the center values
        float nodeCenterX = (node._leftEdge + node._rightEdge) * 0.5f;
        float nodeCenterY = (node._bottomEdge + node._topEdge) * 0.5f;

        int childNodeIndex = 0;
        Particle &p = _localParticleArray[particleIndex];

        if (isnan(p._position.y))
        {
            printf("");
        }

        // TODO: try replacing conditions with index calculation via ternary operator
        if (p._position.y < nodeCenterY)
        {
            // bottom
            if (p._position.x < nodeCenterX)
            {
                // left
                childNodeIndex = node._childNodeIndexBottomLeft;
            }
            else
            {
                // right
                childNodeIndex = node._childNodeIndexBottomRight;
            }
        }
        else
        {
            // top
            if (p._position.x < nodeCenterX)
            {
                // left
                childNodeIndex = node._childNodeIndexTopLeft;
            }
            else
            {
                // right
                childNodeIndex = node._childNodeIndexTopRight;
            }
        }

        // go deeper
        return AddParticleToNode(particleIndex, childNodeIndex);
    }
    else
    {
        // not subdivided, so add the particle to the collection
        node._indicesForContainedParticles[node._numCurrentParticles++] = particleIndex;

        return true;
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    Acquires four new nodes and sets them as children to the provided node.  Sets their 
    dimensions and neighbors based on the parent.  Then takes the node's particles and divides 
    them amongst the child nodes.
Parameters: 
    nodeIndex       Self-explanatory
Returns:    
    True if the subdivision was successful, false if there weren't enough nodes for the 
    subdivision.
Creator:    John Cox (1-28-2016)
-----------------------------------------------------------------------------------------------*/
bool ParticleQuadTree::SubdivideNode(int nodeIndex)
{
    // don't bother checking the pointer; if a null pointer was passed, just crash

    if (_numActiveNodes > (MAX_NODES - 4))
    {
        // not enough to nodes to subdivide again
        return false;
    }

    int childNodeIndexTopLeft = _numActiveNodes++;
    int childNodeIndexTopRight = _numActiveNodes++;
    int childNodeIndexBottomRight = _numActiveNodes++;
    int childNodeIndexBottomLeft = _numActiveNodes++;

    ParticleQuadTreeNode &node = _allNodes[nodeIndex];
    node._isSubdivided = 1;
    node._childNodeIndexTopLeft = childNodeIndexTopLeft;
    node._childNodeIndexTopRight = childNodeIndexTopRight;
    node._childNodeIndexBottomRight = childNodeIndexBottomRight;
    node._childNodeIndexBottomLeft = childNodeIndexBottomLeft;

    float nodeCenterX = (node._leftEdge + node._rightEdge) * 0.5f;
    float nodeCenterY = (node._bottomEdge + node._topEdge) * 0.5f;

    // assign neighbors
    // Note: This is going to get really messy when I move to 3D and have to use octrees, each 
    // with 26 neighbors ((3 * 3 * 3) - 1 center node).

    ParticleQuadTreeNode &childTopLeft = _allNodes[childNodeIndexTopLeft];
    childTopLeft._inUse = 1;
    childTopLeft._neighborIndexLeft = node._neighborIndexLeft;
    childTopLeft._neighborIndexTopLeft = node._neighborIndexTopLeft;
    childTopLeft._neighborIndexTop = node._neighborIndexTop;
    childTopLeft._neighborIndexTopRight = node._neighborIndexTop;
    childTopLeft._neighborIndexRight = childNodeIndexTopRight;
    childTopLeft._neighborIndexBottomRight = childNodeIndexBottomRight;
    childTopLeft._neighborIndexBottom = childNodeIndexBottomLeft;
    childTopLeft._neighborIndexBottomLeft = node._neighborIndexLeft;
    childTopLeft._leftEdge = node._leftEdge;
    childTopLeft._topEdge = node._topEdge;
    childTopLeft._rightEdge = nodeCenterX;
    childTopLeft._bottomEdge = nodeCenterY;

    ParticleQuadTreeNode &childTopRight = _allNodes[childNodeIndexTopRight];
    childTopRight._inUse = 1;
    childTopRight._neighborIndexLeft = childNodeIndexTopLeft;
    childTopRight._neighborIndexTopLeft = node._neighborIndexTop;
    childTopRight._neighborIndexTop = node._neighborIndexTop;
    childTopRight._neighborIndexTopRight = node._neighborIndexTopRight;
    childTopRight._neighborIndexRight = node._neighborIndexRight;
    childTopRight._neighborIndexBottomRight = node._neighborIndexRight;
    childTopRight._neighborIndexBottom = childNodeIndexBottomRight;
    childTopRight._neighborIndexBottomLeft = childNodeIndexBottomLeft;
    childTopRight._leftEdge = nodeCenterX;
    childTopRight._topEdge = node._topEdge;
    childTopRight._rightEdge = node._rightEdge;
    childTopRight._bottomEdge = nodeCenterY;

    ParticleQuadTreeNode &childBottomRight = _allNodes[childNodeIndexBottomRight];
    childBottomRight._inUse = 1;
    childBottomRight._neighborIndexLeft = childNodeIndexBottomLeft;
    childBottomRight._neighborIndexTopLeft = childNodeIndexTopLeft;
    childBottomRight._neighborIndexTop = childNodeIndexTopRight;
    childBottomRight._neighborIndexTopRight = node._neighborIndexRight;
    childBottomRight._neighborIndexRight = node._neighborIndexRight;
    childBottomRight._neighborIndexBottomRight = node._neighborIndexBottomRight;
    childBottomRight._neighborIndexBottom = node._neighborIndexBottom;
    childBottomRight._neighborIndexBottomLeft = node._neighborIndexBottom;
    childBottomRight._leftEdge = nodeCenterX;
    childBottomRight._topEdge = nodeCenterY;
    childBottomRight._rightEdge = node._rightEdge;
    childBottomRight._bottomEdge = node._bottomEdge;

    ParticleQuadTreeNode &childBottomLeft = _allNodes[childNodeIndexBottomLeft];
    childBottomLeft._inUse = 1;
    childBottomLeft._neighborIndexLeft = node._neighborIndexLeft;
    childBottomLeft._neighborIndexTopLeft = node._neighborIndexLeft;
    childBottomLeft._neighborIndexTop = childNodeIndexTopLeft;
    childBottomLeft._neighborIndexTopRight = childNodeIndexTopRight;
    childBottomLeft._neighborIndexRight = childNodeIndexBottomRight;
    childBottomLeft._neighborIndexBottomRight = node._neighborIndexBottom;
    childBottomLeft._neighborIndexBottom = node._neighborIndexBottom;
    childBottomLeft._neighborIndexBottomLeft = node._neighborIndexBottomLeft;
    childBottomLeft._leftEdge = node._leftEdge;
    childBottomLeft._topEdge = nodeCenterY;
    childBottomLeft._rightEdge = nodeCenterX;
    childBottomLeft._bottomEdge = node._bottomEdge;

    // redistribute the particles amongst the children
    for (int particleCount = 0; particleCount < node._numCurrentParticles; particleCount++)
    {
        int particleIndex = node._indicesForContainedParticles[particleCount];
        Particle &p = _localParticleArray[particleIndex];

        // the node is subdivided, so add the particle to the child nodes
        int childNodeIndex = -1;

        // TODO: replace conditions with ternary operators to calculate indices (an inline private helper function would work)
        if (p._position.y < nodeCenterY)
        {
            // bottom
            if (p._position.x < nodeCenterX)
            {
                // left
                childNodeIndex = node._childNodeIndexBottomLeft;
            }
            else
            {
                // right
                childNodeIndex = node._childNodeIndexBottomRight;
            }
        }
        else
        {
            // top
            if (p._position.x < nodeCenterX)
            {
                // left
                childNodeIndex = node._childNodeIndexTopLeft;
            }
            else
            {
                // right
                childNodeIndex = node._childNodeIndexTopRight;
            }
        }

        ParticleQuadTreeNode &childNode = _allNodes[childNodeIndex];
        childNode._indicesForContainedParticles[childNode._numCurrentParticles++] = particleIndex;

        // not actually necessary because the array will be run over on the next update, but I 
        // still like to clean up after myself in case of debugging
        node._indicesForContainedParticles[particleCount] = -1;
    }

    // reset the subdivided node's particle count so that it doesn't try to subdivide again
    node._numCurrentParticles = 0;
}


