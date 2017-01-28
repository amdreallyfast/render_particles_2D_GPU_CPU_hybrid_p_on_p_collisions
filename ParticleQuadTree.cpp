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



    //// start with a single subdivision
    //int numInitialRows = 2;
    //int numInitialColumns = 2;

    //// starting at the origin (upper left corner of the 2D particle region)
    //// Note: Unlike most rectangles in graphical programming, OpenGL's begin at the lower left.  
    //// Beginning at the upper left, as this algorithm does, means that X and Y are not (0,0), 
    //// but X and Y are (0, max).
    //float xBegin = particleRegionCenter.x - particleRegionRadius;
    //float yBegin = particleRegionCenter.y + particleRegionRadius;

    //float xIncrementPerNode = 2.0f * particleRegionRadius / numInitialRows;
    //float yIncrementPerNode = 2.0f * particleRegionRadius / numInitialColumns;

    //float y = yBegin;
    //for (int row = 0; row < numInitialRows; row++)
    //{
    //    float x = xBegin;
    //    for (int column = 0; column < numInitialColumns; column++)
    //    {
    //        int nodeIndex = (row * numInitialColumns) + column;
    //        ParticleQuadTreeNode &node = _allNodes[nodeIndex];
    //        node._inUse = true;

    //        // set the borders of the node
    //        node._leftEdge = x;
    //        node._rightEdge = x + xIncrementPerNode;
    //        node._topEdge = y;
    //        node._bottomEdge = y - yIncrementPerNode;

    //        // assign neighbors
    //        // Note: Nodes on the edge of the initial tree have three null neighbors.  There may 
    //        // be other ways to calculate these, but I chose the following because it is spelled 
    //        // out verbosely.
    //        // Ex: a top - row node has null top left, top, and top right neighbors.

    //        // left edge does not have a "left" neighbor
    //        if (column > 0)
    //        {
    //            node._neighborIndexLeft = nodeIndex - 1;
    //        }

    //        // left and top edges do not have a "top left" neighbor
    //        if (row > 0 && column > 0)
    //        {
    //            // "top left" neighbor = current node - 1 row - 1 column
    //            node._neighborIndexTopLeft = nodeIndex - numInitialColumns - 1;
    //        }

    //        // top row does not have a "top" neighbor
    //        if (row > 0)
    //        {
    //            node._neighborIndexTop = nodeIndex - numInitialColumns;
    //        }

    //        // top right edges do not have a "top right" neighbor
    //        if (row > 0 && column < (numInitialColumns - 1))
    //        {
    //            // "top right" neighbor = current node - 1 row + 1 column
    //            node._neighborIndexTopRight = nodeIndex - numInitialColumns + 1;
    //        }

    //        // right edge does not have a "right" neighbor
    //        if (column < (numInitialColumns - 1))
    //        {
    //            node._neighborIndexRight = nodeIndex + 1;
    //        }

    //        // right and bottom edges do not have a "bottom right" neighbor
    //        if (row < (numInitialRows - 1) &&
    //            column < (numInitialColumns - 1))
    //        {
    //            // "bottom right" neighbor = current node + 1 row + 1 column
    //            node._neighborIndexBottomRight = nodeIndex + numInitialColumns + 1;
    //        }

    //        // bottom edge does not have a "bottom" neighbor
    //        if (row < (numInitialRows - 1))
    //        {
    //            node._neighborIndexBottom = nodeIndex + numInitialColumns;
    //        }

    //        // left and bottom edges do not have a "bottom left" neighbor
    //        if (row < (numInitialRows - 1) && column > 0)
    //        {
    //            // bottom left neighbor = current node + 1 row - 1 column
    //            node._neighborIndexBottomLeft = nodeIndex + numInitialColumns - 1;
    //        }

    //        // setup for next node
    //        x += xIncrementPerNode;
    //    }

    //    // end of row, increment to the next one
    //    y -= yIncrementPerNode;
    //}
}

// TODO: header
void ParticleQuadTree::ResetTree()
{
    _numActiveNodes = FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES;

    for (int nodeIndex = 0; nodeIndex < _MAX_NODES; nodeIndex++)
    {
        ParticleQuadTreeNode &node = _allNodes[nodeIndex];

        node._numCurrentParticles = 0;
        node._isSubdivided = 0;
        node._childNodeIndexTopLeft = -1;
        node._childNodeIndexTopRight = -1;
        node._childNodeIndexBottomLeft = -1;
        node._childNodeIndexBottomRight = -1;

        // all excess nodes are turned off
        if (nodeIndex >= FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES)
        {
            node._inUse = 0;
        }
    }

    // TODO: try a memset to 0 from _allNodes[FIRST_FOUR_NODE_INDEXES::NUM_STARTING_NODES] to _allNodes[_MAX_NODES - 1]

    //// top left
    //ParticleQuadTreeNode &node = _allNodes[FIRST_FOUR_NODE_INDEXES::TOP_LEFT];

    //// top right
    //node = _allNodes[FIRST_FOUR_NODE_INDEXES::TOP_RIGHT];

    //// bottom left
    //node = _allNodes[FIRST_FOUR_NODE_INDEXES::BOTTOM_LEFT];

    //// bottom right
    //node = _allNodes[FIRST_FOUR_NODE_INDEXES::BOTTOM_RIGHT];

}

/*-----------------------------------------------------------------------------------------------
Description:
    Governs the addition of particles to the quad tree.  AddParticleToNode(...) will 
    handle subdivision and addition of particles to child nodes.
Parameters: 
    particleCollection  A container for all particles in use by this program.
Returns:    None
Exception:  Safe
Creator:    John Cox (12-17-2016)
-----------------------------------------------------------------------------------------------*/
void ParticleQuadTree::AddParticlestoTree(Particle *particleCollection, int numParticles)
{
    for (size_t particleIndex = 0; particleIndex < numParticles; particleIndex++)
    {
        Particle &p = particleCollection[particleIndex];
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

        AddParticleToNode(particleIndex, nodeIndex, particleCollection);
    }

    _completedNodePopulations++;
}

// TODO: header
const ParticleQuadTreeNode *ParticleQuadTree::CurrentBuffer() const
{
    return _allNodes;
}

// TODO: header
unsigned int ParticleQuadTree::NumActiveNodes() const
{
    return _numActiveNodes;
}

// TODO: header
unsigned int ParticleQuadTree::SizeOfAllNodesBytes() const
{
    return _MAX_NODES * sizeof(ParticleQuadTreeNode);
}

// TODO: header
int ParticleQuadTree::NumNodePopulations() const
{
    return _completedNodePopulations;
}

// TODO: header
bool ParticleQuadTree::AddParticleToNode(int particleIndex, int nodeIndex, Particle *particleCollection)
{
    // don't bother checking the pointer; if a null pointer was passed, just crash
    ParticleQuadTreeNode &node = _allNodes[nodeIndex];

    if (node._numCurrentParticles == ParticleQuadTreeNode::MAX_PARTICLES_PER_QUAD_TREE_NODE)
    {
        if (!SubdivideNode(nodeIndex, particleCollection))
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
        Particle &p = particleCollection[particleIndex];

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
        return AddParticleToNode(particleIndex, childNodeIndex, particleCollection);
    }
    else
    {
        // not subdivided, so add the particle to the collection
        node._indicesForContainedParticles[node._numCurrentParticles++] = particleIndex;

        return true;
    }
}

// TODO: header
bool ParticleQuadTree::SubdivideNode(int nodeIndex, Particle *particleCollection)
{
    // don't bother checking the pointer; if a null pointer was passed, just crash

    if (_numActiveNodes > (_MAX_NODES - 4))
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
        Particle &p = particleCollection[particleIndex];

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
}


