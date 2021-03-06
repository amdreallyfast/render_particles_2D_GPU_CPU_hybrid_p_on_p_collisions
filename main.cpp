// the OpenGL version include also includes all previous versions
// Build note: Due to a minefield of preprocessor build flags, the gl_load.hpp must come after 
// the version include.
// Build note: Do NOT mistakenly include _int_gl_4_4.h.  That one doesn't define OpenGL stuff 
// first.
// Build note: Also need to link opengl32.lib (unknown directory, but VS seems to know where it 
// is, so don't put in an "Additional Library Directories" entry for it).
// Build note: Also need to link glload/lib/glloadD.lib.
#include "glload/include/glload/gl_4_4.h"
#include "glload/include/glload/gl_load.hpp"

// Build note: Must be included after OpenGL code (in this case, glload).
// Build note: Also need to link freeglut/lib/freeglutD.lib.  However, the linker will try to 
// find "freeglut.lib" (note the lack of "D") instead unless the following preprocessor 
// directives are set either here or in the source-building command line (VS has a
// "Preprocessor" section under "C/C++" for preprocessor definitions).
// Build note: Also need to link winmm.lib (VS seems to know where it is, so don't put in an 
// "Additional Library Directories" entry).
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#include "freeglut/include/GL/freeglut.h"

// this linking approach is very useful for portable, crude, barebones demo code, but it is 
// better to link through the project building properties
#pragma comment(lib, "glload/lib/glloadD.lib")
#pragma comment(lib, "opengl32.lib")            // needed for glload::LoadFunctions()
#pragma comment(lib, "freeglut/lib/freeglutD.lib")
#ifdef WIN32
#pragma comment(lib, "winmm.lib")               // Windows-specific; freeglut needs it
#endif

// apparently the FreeType lib also needs a companion file, "freetype261d.pdb"
#pragma comment (lib, "freetype-2.6.1/objs/vc2010/Win32/freetype261d.lib")

// for printf(...)
#include <stdio.h>

// for basic OpenGL stuff
#include "OpenGlErrorHandling.h"
#include "ShaderStorage.h"

// for particles, where they live, and how to update them
#include "glm/vec2.hpp"
#include "ParticleQuadTree.h"
#include "ParticleSsbo.h"
#include "PolygonSsbo.h"
#include "QuadTreeNodeSsbo.h"
#include "ComputeControllerGenerateQuadTreeGeometry.h"
#include "ComputeControllerParticleReset.h"
#include "ComputeControllerParticleUpdate.h"
#include "ComputeControllerParticleCollisions.h"

// for moving the shapes around in window space
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// for the frame rate counter
#include "FreeTypeEncapsulated.h"
#include "Stopwatch.h"

Stopwatch gTimer;
FreeTypeEncapsulated gTextAtlases;

// in a bigger program, uniform locations would probably be stored in the same place as the 
// shader programs
GLint gUnifLocGeometryTransform;

// ??stored in scene??
ParticleSsbo *gpParticleBuffer = 0;
PolygonSsbo *gpParticleBoundingRegionBuffer = 0;
PolygonSsbo *gpQuadTreeGeometryBuffer = 0;
QuadTreeNodeSsbo *gpQuadTreeBuffer = 0;
ParticleQuadTree *gpQuadTree = 0;

// in a bigger program, ??where would particle stuff be stored??
IParticleEmitter *gpParticleEmitterBar1 = 0;
IParticleEmitter *gpParticleEmitterBar2 = 0;
ComputeControllerParticleReset *gpParticleReseter = 0;
ComputeControllerParticleUpdate *gpParticleUpdater = 0;
ComputeControllerGenerateQuadTreeGeometry *gpQuadTreeGeometryGenerator = 0;
ComputeControllerParticleCollisions *gpQuadTreeParticleCollider = 0;




/*-----------------------------------------------------------------------------------------------
Description:
    Creates a 32-point wireframe circle.

    Note: I could have used sinf(...) and cosf(...) to create the points, but where's the fun in
    that if I have a faster and obtuse algorithm :) ?  Algorithm courtesy of 
    http://slabode.exofire.net/circle_draw.shtml .
Parameters:
    putDataHere     Self-explanatory.
    radius          Values in window coordinates (X and Y on range [-1,+1]).
Returns:    None
Exception:  Safe
Creator:    John Cox (6-12-2016)
            Adapted for this program 1/7/2017
-----------------------------------------------------------------------------------------------*/
void GenerateCircle(const glm::vec4 &center, const float radius, std::vector<PolygonFace> *putDataHere)
{
    // a 32-point, 0.25 radius (window dimensions) circle will suffice for this demo
    unsigned int arcSegments = 32;
    float x = radius;
    float y = 0.0f;
    float theta = 2 * 3.1415926f / float(arcSegments);
    float tangetialFactor = tanf(theta);
    float radialFactor = cosf(theta);

    // duplicate vertices are expected in this approach
    // Note: The polygon SSBOs were made in anticipation of particle-polygon collision detection 
    // in a compute shader, in which each face should be a self-contained unit.  I decided that 
    // duplicate vertices would be ok for this project since it is particle-heavy, not vertex 
    // heavy.  Besides, graphical memory is absurdly cheap these days.
    // Also Note: This algorithm assumes (I think) that the origin is at (0,0).  Account for 
    // non-0 centers by adding the argument "center" to each vertex after it is calculated.
    for (unsigned int segmentCount = 0; segmentCount < 32; segmentCount++)
    {
        glm::vec2 pos1(x, y);
        glm::vec2 normal1(glm::normalize(glm::vec2(x, y)));
        MyVertex v1(pos1, normal1);

        float tx = (-y) * tangetialFactor;
        float ty = x * tangetialFactor;

        // add the tangential factor
        x += tx;
        y += ty;

        // correct using the radial factor
        x *= radialFactor;
        y *= radialFactor;

        glm::vec2 pos2(x, y);
        glm::vec2 normal2(glm::normalize(glm::vec2(x, y)));
        MyVertex v2(pos2, normal2);

        // Note: The same X and Y that were used for the second vertex will be used for the 
        // first vertex of the next face.

        // account for non-0 circle centers
        v1._position += center;
        v2._position += center;

        putDataHere->push_back(PolygonFace(v1, v2));
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    Governs window creation, the initial OpenGL configuration (face culling, depth mask, even
    though this is a 2D demo and that stuff won't be of concern), the creation of geometry, and
    the creation of a texture.
Parameters: None
Returns:    None
Creator:    John Cox (3-7-2016)
-----------------------------------------------------------------------------------------------*/
void Init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    // inactive particle Z = -0.6   alpha = 0
    // active particle Z = -0.7     alpha = 1
    // polygon fragment Z = -0.8    alpha = 1
    // Note: The blend function is done via glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA).  
    // The first argument is the scale factor for the source color (presumably the existing 
    // fragment color in the frame buffer) and the second argument is the scale factor for the 
    // destination color (presumably the color of the fragment that is being added).  The second 
    // argument ("one minus source alpha") means that, when any color is being added, the 
    // resulting color will be "(existingFragmentAlpha * existingFragmentColor) - 
    // (addedFragmentAlpha * addedFragmentColor)".  
    // Also Note: If the color furthest from the camera is black (vec4(0,0,0,0)), then any 
    // color on top of it will end up as (using the equation) "vec4(0,0,0,0) - whatever", which 
    // is clamped at 0.  So put the opaque (alpha=1) furthest from the camera (this demo is 2D, 
    // so make it a lower Z).  The depth range is 0-1, so the lower Z limit is -1.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();

    // FreeType initialization
    std::string freeTypeShaderKey = "freetype";
    shaderStorageRef.NewShader(freeTypeShaderKey);
    shaderStorageRef.AddShaderFile(freeTypeShaderKey, "FreeType.vert", GL_VERTEX_SHADER);
    shaderStorageRef.AddShaderFile(freeTypeShaderKey, "FreeType.frag", GL_FRAGMENT_SHADER);
    shaderStorageRef.LinkShader(freeTypeShaderKey);
    GLuint freeTypeProgramId = shaderStorageRef.GetShaderProgram(freeTypeShaderKey);
    gTextAtlases.Init("FreeSans.ttf", freeTypeProgramId);

    // for the particle compute shader stuff
    std::string computeShaderUpdateKey = "compute particle update";
    shaderStorageRef.NewShader(computeShaderUpdateKey);
    shaderStorageRef.AddShaderFile(computeShaderUpdateKey, "ParticleUpdate.comp", GL_COMPUTE_SHADER);
    shaderStorageRef.LinkShader(computeShaderUpdateKey);

    std::string computeShaderResetKey = "compute particle reset";
    shaderStorageRef.NewShader(computeShaderResetKey);
    shaderStorageRef.AddShaderFile(computeShaderResetKey, "ParticleReset.comp", GL_COMPUTE_SHADER);
    shaderStorageRef.LinkShader(computeShaderResetKey);

    std::string computeQuadTreeParticleColliderKey = "compute quad tree collider";
    shaderStorageRef.NewShader(computeQuadTreeParticleColliderKey);
    shaderStorageRef.AddShaderFile(computeQuadTreeParticleColliderKey, "ParticleCollisions.comp", GL_COMPUTE_SHADER);
    shaderStorageRef.LinkShader(computeQuadTreeParticleColliderKey);

    std::string ComputeControllerGenerateQuadTreeGeometryKey = "compute quad tree generate geometry";
    shaderStorageRef.NewShader(ComputeControllerGenerateQuadTreeGeometryKey);
    shaderStorageRef.AddShaderFile(ComputeControllerGenerateQuadTreeGeometryKey, "GenerateQuadTreeGeometry.comp", GL_COMPUTE_SHADER);
    shaderStorageRef.LinkShader(ComputeControllerGenerateQuadTreeGeometryKey);



    // a render shader specifically for the particles (particle color may change depending on 
    // particle state, so it isn't the same as the geometry's render shader)
    std::string renderParticlesShaderKey = "render particles";
    shaderStorageRef.NewShader(renderParticlesShaderKey);
    shaderStorageRef.AddShaderFile(renderParticlesShaderKey, "ParticleRender.vert", GL_VERTEX_SHADER);
    shaderStorageRef.AddShaderFile(renderParticlesShaderKey, "ParticleRender.frag", GL_FRAGMENT_SHADER);
    shaderStorageRef.LinkShader(renderParticlesShaderKey);

    // a render shader specifically for the geometry (nothing special; just a transform, color 
    // white, pass through to frag shader)
    std::string renderGeometryShaderKey = "render geometry";
    shaderStorageRef.NewShader(renderGeometryShaderKey);
    shaderStorageRef.AddShaderFile(renderGeometryShaderKey, "Geometry.vert", GL_VERTEX_SHADER);
    shaderStorageRef.AddShaderFile(renderGeometryShaderKey, "Geometry.frag", GL_FRAGMENT_SHADER);
    shaderStorageRef.LinkShader(renderGeometryShaderKey);
    GLuint renderGeometryProgramId = shaderStorageRef.GetShaderProgram(renderGeometryShaderKey);
    //gUnifLocGeometryTransform = shaderStorageRef.GetUniformLocation(renderGeometryShaderKey, "transformMatrixWindowSpace");

    // set up the particle region 
    //glm::mat4 windowSpaceTransform = glm::rotate(glm::mat4(), 45.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    //windowSpaceTransform *= glm::translate(glm::mat4(), glm::vec3(-0.1f, -0.05f, 0.0f));
    glm::mat4 windowSpaceTransform = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    windowSpaceTransform *= glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));

    glm::vec4 particleRegionCenter = windowSpaceTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float particleRegionRadius = 0.8f;

    std::vector<PolygonFace> particleRegionPolygonFaces;
    GenerateCircle(particleRegionCenter, particleRegionRadius, &particleRegionPolygonFaces);
    gpParticleBoundingRegionBuffer = new PolygonSsbo(particleRegionPolygonFaces);
    gpParticleBoundingRegionBuffer->ConfigureRender(renderGeometryProgramId, GL_LINES);

    // set up the particle SSBO for computing and rendering
    std::vector<Particle> allParticles(Particle::MAX_PARTICLES);
    gpParticleBuffer = new ParticleSsbo(allParticles);
    gpParticleBuffer->ConfigureCompute(shaderStorageRef.GetShaderProgram(computeShaderResetKey), "ParticleBuffer");
    gpParticleBuffer->ConfigureCompute(shaderStorageRef.GetShaderProgram(computeShaderUpdateKey), "ParticleBuffer");
    gpParticleBuffer->ConfigureCompute(shaderStorageRef.GetShaderProgram(computeQuadTreeParticleColliderKey), "ParticleBuffer");
    gpParticleBuffer->ConfigureRender(shaderStorageRef.GetShaderProgram(renderParticlesShaderKey), GL_POINTS);

    // set up the quad tree for computation
    gpQuadTree = new ParticleQuadTree(particleRegionCenter, particleRegionRadius);
    gpQuadTreeBuffer = new QuadTreeNodeSsbo(gpQuadTree->QuadTreeBuffer(), ParticleQuadTree::MAX_NODES);
    gpQuadTreeBuffer->ConfigureCompute(shaderStorageRef.GetShaderProgram(computeQuadTreeParticleColliderKey), "QuadTreeNodeBuffer");
    gpQuadTreeBuffer->ConfigureCompute(shaderStorageRef.GetShaderProgram(ComputeControllerGenerateQuadTreeGeometryKey), "QuadTreeNodeBuffer");

    // set up the quad tree's nodes for rendering
    unsigned int allPolygonFaces = ParticleQuadTree::MAX_NODES * 4;
    std::vector<PolygonFace> quadTreePolygonFaces(allPolygonFaces);
    gpQuadTreeGeometryBuffer = new PolygonSsbo(quadTreePolygonFaces);
    gpQuadTreeGeometryBuffer->ConfigureCompute(shaderStorageRef.GetShaderProgram(ComputeControllerGenerateQuadTreeGeometryKey), "QuadTreeFaceBuffer");
    gpQuadTreeGeometryBuffer->ConfigureRender(renderGeometryProgramId, GL_LINES);


    // put the bar emitters across from each and spraying particles toward each other and up so 
    // that the particles collide near the middle with a slight upward velocity

    // bar on the left and emitting up and right
    glm::vec2 bar1P1(-0.5f, +0.5f);
    glm::vec2 bar1P2(-0.5f, -0.0f);
    glm::vec2 emitDir1(+1.0f, +0.0f);
    float minVel = 0.1f;
    float maxVel = 0.5f;
    gpParticleEmitterBar1 = new ParticleEmitterBar(bar1P1, bar1P2, emitDir1, minVel, maxVel);
    gpParticleEmitterBar1->SetTransform(windowSpaceTransform);

    // bar on the right and emitting up and left
    glm::vec2 bar2P1 = glm::vec2(-0.1f, -0.5f);
    glm::vec2 bar2P2 = glm::vec2(+0.2f, -0.5f);
    glm::vec2 emitDir2 = glm::vec2(-0.0f, +0.5f);
    gpParticleEmitterBar2 = new ParticleEmitterBar(bar2P1, bar2P2, emitDir2, minVel, maxVel);
    gpParticleEmitterBar2->SetTransform(windowSpaceTransform);

    // start up the encapsulation of the CPU side of the computer shader
    gpParticleReseter = new ComputeControllerParticleReset(Particle::MAX_PARTICLES, computeShaderResetKey);
    gpParticleReseter->AddEmitter(gpParticleEmitterBar1);
    gpParticleReseter->AddEmitter(gpParticleEmitterBar2);

    gpParticleUpdater = new ComputeControllerParticleUpdate(Particle::MAX_PARTICLES, particleRegionCenter, particleRegionRadius, computeShaderUpdateKey);

    gpQuadTreeGeometryGenerator = new ComputeControllerGenerateQuadTreeGeometry(ParticleQuadTree::MAX_NODES, allPolygonFaces, ComputeControllerGenerateQuadTreeGeometryKey);

    gpQuadTreeParticleCollider = new ComputeControllerParticleCollisions(Particle::MAX_PARTICLES, ParticleQuadTree::MAX_NODES, particleRegionCenter, computeQuadTreeParticleColliderKey);

    // the timer will be used for framerate calculations
    gTimer.Init();
    gTimer.Start();
}

/*-----------------------------------------------------------------------------------------------
Description:
    Updates particle positions, generates the quad tree for the particles' new positions, and 
    commands a new draw.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (1-2-2017)
-----------------------------------------------------------------------------------------------*/
void UpdateAllTheThings()
{
    // just hard-code it for this demo
    float deltaTimeSec = 0.01f;

    // reset inactive particles and update active particles (the MAGIC happens here)
    // Note: 20 particles per emitter per frame * 8 emitters * 60 frames per second stabilizes 
    // (for this particle region and the emitters' min-max spawn velocities) at ~45,000 active 
    // particles in one moment.
    // Also Note: 50 easily maxes out the maximuum 100,000 total particles active at one time.
    gpParticleReseter->ResetParticles(10);
    gpParticleUpdater->Update(deltaTimeSec);

    // grap updated particle data from the GPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpParticleBuffer->BufferId());
    void *bufferPtr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    Particle *particleObjPtr = static_cast<Particle *>(bufferPtr);

    // populate the quad tree (the CPU is great for this job)
    gpQuadTree->ResetTree();
    gpQuadTree->AddParticlestoTree(particleObjPtr, Particle::MAX_PARTICLES);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // and upload the resulting quad tree
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpQuadTreeBuffer->BufferId());
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gpQuadTreeBuffer->BufferSizeBytes(), gpQuadTree->QuadTreeBuffer());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    gpQuadTreeParticleCollider->Update(deltaTimeSec);
    //gpQuadTreeGeometryGenerator->GenerateGeometry();


    // tell glut to call this display() function again on the next iteration of the main loop
    // Note: https://www.opengl.org/discussion_boards/showthread.php/168717-I-dont-understand-what-glutPostRedisplay()-does
    // Also Note: This display() function will also be registered to run if the window is moved
    // or if the viewport is resized.  If glutPostRedisplay() is not called, then as long as the
    // window stays put and doesn't resize, display() won't be called again (tested with 
    // debugging).
    // Also Also Note: It doesn't matter where this is called in this function.  It sets a flag
    // for glut's main loop and doesn't actually call the registered display function, but I 
    // got into the habbit of calling it at the end.
    glutPostRedisplay();
}

/*-----------------------------------------------------------------------------------------------
Description:
    This is the rendering function.  It tells OpenGL to clear out some color and depth buffers,
    to set up the data to draw, to draw than stuff, and to report any errors that it came across.
    This is not a user-called function.

    This function is registered with glutDisplayFunc(...) during glut's initialization.
Parameters: None
Returns:    None
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Display()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw the particle region borders
    glUseProgram(ShaderStorage::GetInstance().GetShaderProgram("render geometry"));
    //glUniformMatrix4fv(gUnifLocGeometryTransform, 1, GL_FALSE, glm::value_ptr(windowSpaceTransform));
    GLuint vaoId = gpParticleBoundingRegionBuffer->VaoId();
    GLenum drawStyle = gpParticleBoundingRegionBuffer->DrawStyle();
    GLuint numVertices = gpParticleBoundingRegionBuffer->NumVertices();
    glBindVertexArray(vaoId);
    glDrawArrays(drawStyle, 0, numVertices);

    //// draw the nodes of the quad tree
    //// Note: Keep using the "render geometry" shader.
    //vaoId = gpQuadTreeGeometryBuffer->VaoId();
    //drawStyle = gpQuadTreeGeometryBuffer->DrawStyle();
    //numVertices = gpQuadTreeGeometryGenerator->NumActiveFaces() * 2;
    //glBindVertexArray(vaoId);
    //glDrawArrays(drawStyle, 0, numVertices);

    // draw the particles
    glUseProgram(ShaderStorage::GetInstance().GetShaderProgram("render particles"));
    glBindVertexArray(gpParticleBuffer->VaoId());
    glDrawArrays(gpParticleBuffer->DrawStyle(), 0, gpParticleBuffer->NumVertices());

    // draw text on top of the rendered items
    glUseProgram(ShaderStorage::GetInstance().GetShaderProgram("freetype"));
    //GLfloat color[4] = { 0.5f, 0.5f, 0.0f, 1.0f };
    GLfloat color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    char str[32];
    int pointSize = 32;
    float scaleXY[2] = { 1.0f, 1.0f };

    // calulate frame rate
    static int elapsedFramesPerSecond = 0;
    static double elapsedTime = 0.0;
    static double frameRate = 0.0;
    static int quadTreePopulationsPerSecond = 0;
    elapsedFramesPerSecond++;
    elapsedTime += gTimer.Lap();
    if (elapsedTime > 1.0)
    {
        frameRate = (double)elapsedFramesPerSecond / elapsedTime;
        elapsedFramesPerSecond = 0;

        quadTreePopulationsPerSecond = (int)((gpQuadTree->NumNodePopulations()) / elapsedTime);
        gpQuadTree->ResetNumNodePopulations();

        elapsedTime -= 1.0f;
    }

    // draw the frame rate once per second in the lower left corner
    // Note: The font textures' orgin is their lower left corner, so the "lower left" in screen 
    // space is just above [-1.0f, -1.0f].
    // Also Note: The first time that "get shader program" runs, it will load the atlas.
    sprintf(str, "%.2lf", frameRate);
    float frameRateXY[2] = { -0.99f, -0.99f };
    gTextAtlases.GetAtlas(pointSize)->RenderText(str, frameRateXY, scaleXY, color);

    // show the number of quad tree populatins per second (it may be a bottleneck)
    sprintf(str, "tree fill rate: %d", quadTreePopulationsPerSecond);
    float quadTreePopulationRateXY[2] = { -0.99f, +0.25f };
    gTextAtlases.GetAtlas(pointSize)->RenderText(str, quadTreePopulationRateXY, scaleXY, color);

    // now show number of active particles
    // Note: For some reason, lower case "i" seems to appear too close to the other letters.
    sprintf(str, "particles: %d", gpParticleUpdater->NumActiveParticles());
    float numActiveParticlesXY[2] = { -0.99f, +0.7f };
    gTextAtlases.GetAtlas(pointSize)->RenderText(str, numActiveParticlesXY, scaleXY, color);

    // now draw the number of active quad tree nodes
    sprintf(str, "nodes: %d", gpQuadTree->NumActiveNodes());
    float numActiveNodesXY[2] = { -0.99f, +0.5f };
    gTextAtlases.GetAtlas(pointSize)->RenderText(str, numActiveNodesXY, scaleXY, color);



    //GLuint copyBufferId1;
    //glGenBuffers(1, &copyBufferId1);
    //glBindBuffer(GL_COPY_WRITE_BUFFER, copyBufferId1);
    //glBufferData(GL_COPY_WRITE_BUFFER, gpQuadTreeBuffer->BufferSizeBytes(), 0, GL_DYNAMIC_COPY);
    //glBindBuffer(GL_COPY_READ_BUFFER, gpQuadTreeBuffer->BufferId());
    //glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, gpQuadTreeBuffer->BufferSizeBytes());
    //void *bufferPtr = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_READ_ONLY);
    //ParticleQuadTreeNode *nodeObjPtr = static_cast<ParticleQuadTreeNode *>(bufferPtr);
    //for (size_t i = 0; i < ParticleQuadTree::MAX_NODES; i++)
    //{
    //    ParticleQuadTreeNode &node = nodeObjPtr[i];
    //    if (node._inUse && node._isSubdivided == 0)
    //    {
    //        //sprintf(str, "%d", i);  // index number
    //        sprintf(str, "%d", node._numCurrentParticles);
    //        float frameRateXY[2] = { node._leftEdge + 0.01f, node._bottomEdge + 0.01f };
    //        gTextAtlases.GetAtlas(24)->RenderText(str, frameRateXY, scaleXY, color);
    //    }
    //}
    //glUnmapBuffer(GL_COPY_WRITE_BUFFER);
    //glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    //glBindBuffer(GL_COPY_READ_BUFFER, 0);
    //glDeleteBuffers(1, &copyBufferId1);







    // clean up bindings
    glUseProgram(0);
    glBindVertexArray(0);       // unbind this BEFORE the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // tell the GPU to swap out the displayed buffer with the one that was just rendered
    glutSwapBuffers();
}

/*-----------------------------------------------------------------------------------------------
Description:
    Tell's OpenGL to resize the viewport based on the arguments provided.  This is an 
    opportunity to call glViewport or glScissor to keep up with the change in size.
    
    This is not a user-called function.  It is registered with glutReshapeFunc(...) during 
    glut's initialization.
Parameters:
    w   The width of the window in pixels.
    h   The height of the window in pixels.
Returns:    None
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Executes when the user presses a key on the keyboard.

    This is not a user-called function.  It is registered with glutKeyboardFunc(...) during
    glut's initialization.

    Note: Although the x and y arguments are for the mouse's current position, this function does
    not respond to mouse presses.
Parameters:
    key     The ASCII code of the key that was pressed (ex: ESC key is 27)
    x       The horizontal viewport coordinates of the mouse's current position.
    y       The vertical window coordinates of the mouse's current position
Returns:    None
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Keyboard(unsigned char key, int x, int y)
{
    // this statement is mostly to get ride of an "unreferenced parameter" warning
    printf("keyboard: x = %d, y = %d\n", x, y);
    switch (key)
    {
    case 27:
    {
        // ESC key
        glutLeaveMainLoop();
        return;
    }
    default:
        break;
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    I don't know what this does, but I've kept it around since early times, and this was the 
    comment given with it:
    
    "Called before FreeGLUT is initialized. It should return the FreeGLUT display mode flags 
    that you want to use. The initial value are the standard ones used by the framework. You can 
    modify it or just return you own set.  This function can also set the width/height of the 
    window. The initial value of these variables is the default, but you can change it."
Parameters:
    displayMode     ??
    width           ??
    height          ??
Returns:
    ??what??
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
unsigned int Defaults(unsigned int displayMode, int &width, int &height) 
{
    // this statement is mostly to get ride of an "unreferenced parameter" warning
    printf("Defaults: width = %d, height = %d\n", width, height);
    return displayMode; 
}

/*-----------------------------------------------------------------------------------------------
Description:
    Cleans up GPU memory.  This might happen when the processes die, but be a good memory steward
    and clean up properly.

    Note: A big program would have the textures, program IDs, buffers, and other things 
    encapsulated somewhere, and each other those would do the cleanup, but in this barebones 
    demo, I can just clean up everything here.
Parameters: None
Returns:    None
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void CleanupAll()
{
    delete gpParticleBuffer;
    delete gpParticleBoundingRegionBuffer;
    delete gpQuadTreeGeometryBuffer;
    delete gpQuadTreeBuffer;
    delete gpParticleEmitterBar1;
    delete gpParticleEmitterBar2;
    delete gpParticleReseter;
    delete gpParticleUpdater;
    delete gpQuadTreeGeometryGenerator;
}

/*-----------------------------------------------------------------------------------------------
Description:
    Program start and end.
Parameters:
    argc    The number of strings in argv.
    argv    A pointer to an array of null-terminated, C-style strings.
Returns:
    0 if program ended well, which it always does or it crashes outright, so returning 0 is fine
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    int width = 500;
    int height = 500;
    unsigned int displayMode = GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL;
    displayMode = Defaults(displayMode, width, height);

    glutInitDisplayMode(displayMode);
    glutInitContextVersion(4, 4);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    // enable this for automatic message reporting (see OpenGlErrorHandling.cpp)
//#define DEBUG
#ifdef DEBUG
    glutInitContextFlags(GLUT_DEBUG);
#endif

    glutInitWindowSize(width, height);
    glutInitWindowPosition(300, 200);
    int window = glutCreateWindow(argv[0]);

    glload::LoadTest glLoadGood = glload::LoadFunctions();
    // ??check return value??

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    if (!glload::IsVersionGEQ(3, 3))
    {
        printf("Your OpenGL version is %i, %i. You must have at least OpenGL 3.3 to run this tutorial.\n",
            glload::GetMajorVersion(), glload::GetMinorVersion());
        glutDestroyWindow(window);
        return 0;
    }

    if (glext_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(DebugFunc, (void*)15);
    }

    Init();

    glutIdleFunc(UpdateAllTheThings);
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();

    CleanupAll();

    return 0;
}