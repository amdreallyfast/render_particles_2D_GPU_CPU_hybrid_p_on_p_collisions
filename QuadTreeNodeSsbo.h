#pragma once

#include "SsboBase.h"

struct ParticleQuadTreeNode;

/*-----------------------------------------------------------------------------------------------
Description:
    Sets up the Shader Storage Block Object for quad tree nodes.  The polygon will be used in 
    the compute shader only.
Creator: John Cox, 1/16/2017
-----------------------------------------------------------------------------------------------*/
class QuadTreeNodeSsbo : public SsboBase
{
public:
    QuadTreeNodeSsbo(const ParticleQuadTreeNode *nodeCollection, int numNodes);
    virtual ~QuadTreeNodeSsbo();

    void ConfigureCompute(unsigned int computeProgramId, const std::string &bufferNameInShader) override;
    void ConfigureRender(unsigned int renderProgramId, unsigned int drawStyle) override;

private:
    unsigned int _bufferSizeBytes;
};

