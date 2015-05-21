/// \file ObscuranceMap.h
/// \class ObscuranceMap
/// \author Xavier Bonaventura
/// \author Nicolau Sunyer
/// \author Sergi Funtané
/// \author Copyright: (c) Universitat de Girona

#ifndef _OBSCURANCE_MAP_
#define _OBSCURANCE_MAP_

//Dependency includes
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

//Project includes
#include "GLSLProgram.h"
#include "Scene.h"

class ObscuranceMap
{
public:
    // Method to calculate the lightmap.
    ObscuranceMap(Scene *pScene, int pTextureSize);
    void SetScene(Scene* pScene);
    void SetTextureSize(int pTextureSize);
    void ComputeObscurances(int pIterations);
    const float* GetImgRgbBuffer() const;

private:
    //GLSL
    // Handlers
    GLSLProgram *mProjectionProgram;
    GLSLProgram *mTransferProgram;
    GLSLProgram *mNormalizeProgram;

    //Variables to store the OpenGL stats
    bool mPreviousDepthTest, mPreviousCullFace, mPreviousBlend;
    glm::vec4 mPreviousClearColor;
    GLint mPreviousViewport[4];
    GLint mBinded2DTexture;
    GLint mBindedReadFramebuffer;
    GLint mBindedDrawFramebuffer;
    GLint mBindedRenderbuffer;
    GLint mBindedPixelPackBuffer;
    GLint mPreviousProgram;
    GLfloat mPreviousPointSize;

    float* mImgRgbBuffer;
    int mResProjectionX;  //Resolution
    int mResProjectionY;  //Resolution
    int mResLightmapX;    //Resolution
    int mResLightmapY;    //Resolution

    int mCount;

    //Textures and buffers
    GLuint mTexName;
    GLuint mTexProjeccions[2]; //Texture name
    GLuint mTexTransfer;
    GLuint mTexNormalize;
    GLuint mFbProjeccions;           // Framebuffer Projeccions
    GLuint mFbTransfer;              // Framebuffer Transfer
    GLuint mFbLightmapNormalization; // Lightmap Normalization
    GLuint mProjectionsPBO[2];	// PBOs to store the projections.

    GLuint mQueryId;
    float* mBlueBuffer;

    Scene* mScene;						// Mesh Data
    glm::vec3 mEsferaCenter;
    float mEsferaRadius;

    void LoadShaders();
    void SaveOpenGLStats();
    void RestoreOpenGLStats();
    void InitializeProjection();
    void InitializeLightmap();
    float VanDerCorput(int pI, int pBase) const;
    glm::vec3 RandomPoint(float pR, const glm::vec3& pC);
    void UpdateTexture(int pIterations);
    void NormalizeLightmap();
    unsigned int RenderProjeccions(const glm::vec3 &pCameraPosition, int pBuffer, int pFirst);
    void RenderTransfer(int pBuffer, int pFirst);
};

#endif
