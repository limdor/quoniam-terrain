//System includes
#include "time.h"

//Qt includes
#include <QProgressDialog>

//Dependency includes
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\matrix_inverse.hpp"
#include "glew.h"

//Project includes
#include "Debug.h"
#include "GPUGeometry.h"
#include "MainWindow.h"
#include "ObscuranceMap.h"

// To use Offsets in the PBOs.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const unsigned int POSITION  = 0;
const unsigned int TEXTCOORD = 3;

ObscuranceMap::ObscuranceMap(Scene *pScene, int pTextureSize)
{
    //Initialization of projection textures
    mTexName = 0;
    mTexProjeccions[0] = 0;
    //Initialization of projection buffers
    mFbProjeccions = 0;           // Framebuffer Projeccions
    mTexProjeccions[1] = 0;
    mProjectionsPBO[0] = 0;	      // PBOs to store the projections.
    mProjectionsPBO[1] = 0;	      // PBOs to store the projections.
    //Initialization of lightmap textures
    mTexTransfer = 0;
    mTexNormalize = 0;
    //Initialization of lightmap buffers
    mFbTransfer = 0;              // Framebuffer Transfer
    mFbLightmapNormalization = 0; // Lightmap Normalization

    mCount = 0;
    mBlueBuffer = NULL;
    mImgRgbBuffer = NULL;

    LoadShaders();

    SetScene(pScene);
    SetTextureSize(pTextureSize);
}

void ObscuranceMap::SetScene(Scene* pScene)
{
    mScene = pScene;
    mEsferaCenter = mScene->GetBoundingSphere()->GetCenter();
    mEsferaRadius = mScene->GetBoundingSphere()->GetRadius() * (glm::sqrt(12.0f) / 2.0f);

    mResLightmapX = glm::ceil( glm::sqrt( (float)mScene->GetNumberOfPolygons() ) );
    mResLightmapY = mResLightmapX;
    SaveOpenGLStats();
    InitializeLightmap();
    RestoreOpenGLStats();
}

void ObscuranceMap::SetTextureSize(int pTextureSize)
{
    mResProjectionX = pTextureSize * 2;
    mResProjectionY = pTextureSize * 2;

    mResLightmapX = glm::ceil( glm::sqrt( (float)mScene->GetNumberOfPolygons() ) );
    mResLightmapY = mResLightmapX;

    Debug::Log( QString("Plans de projeccio: ResolucioX = %1 , ResolucioY = %2").arg(mResProjectionX).arg(mResProjectionY) );
    Debug::Log( QString("Lightmap: ResolucioX = %1 , ResolucioY = %2").arg(mResLightmapX).arg(mResLightmapY) );
    SaveOpenGLStats();
    InitializeProjection();
    InitializeLightmap();
    RestoreOpenGLStats();
}

void ObscuranceMap::ComputeObscurances(int pIterations)
{
    if(mImgRgbBuffer != NULL)
    {
        delete[] mImgRgbBuffer;
    }
    mImgRgbBuffer = new float[mResLightmapX * mResLightmapY * 4];

    SaveOpenGLStats();
    // Setup GL States
    glEnable(GL_DEPTH_TEST);	// Enable Depth Testing
    glPointSize(1.0);           // Set point size
    //Generate the reflectivity texture.
    UpdateTexture(pIterations);

    glBindFramebuffer(GL_FRAMEBUFFER, mFbLightmapNormalization);
    glReadPixels(0, 0, mResLightmapX, mResLightmapY, GL_RGBA, GL_FLOAT, mImgRgbBuffer);
    RestoreOpenGLStats();
}

const float *ObscuranceMap::GetImgRgbBuffer() const
{
    return mImgRgbBuffer;
}

void ObscuranceMap::LoadShaders()
{
    //Projection Shaders
    GLSLShader* projectionVS = new GLSLShader("shaders/Projection.vert", GL_VERTEX_SHADER);
    if( projectionVS->HasErrors() )
    {
        Debug::Error( QString("shaders/Projection.vert: %1").arg(projectionVS->GetLog()) );
    }
    GLSLShader* projectionFS = new GLSLShader("shaders/Projection.frag", GL_FRAGMENT_SHADER);
    if( projectionFS->HasErrors() )
    {
        Debug::Error( QString("shaders/Projection.frag: %1").arg(projectionFS->GetLog()) );
    }
    mProjectionProgram = new GLSLProgram("Projection");
    mProjectionProgram->AttachShader(projectionVS);
    mProjectionProgram->AttachShader(projectionFS);
    mProjectionProgram->LinkProgram();
    delete projectionVS;
    delete projectionFS;

    //Lightmap Normalization Shaders
    GLSLShader* normalizeVS = new GLSLShader("shaders/Normalize.vert", GL_VERTEX_SHADER);
    if( normalizeVS->HasErrors() )
    {
        Debug::Error( QString("shaders/Normalize.vert: %1").arg(normalizeVS->GetLog()) );
    }
    GLSLShader* normalizeFS = new GLSLShader("shaders/Normalize.frag", GL_FRAGMENT_SHADER);
    if( normalizeFS->HasErrors() )
    {
        Debug::Error( QString("shaders/Normalize.frag: %1").arg(normalizeFS->GetLog()) );
    }
    mNormalizeProgram = new GLSLProgram("Lightmap Normalization");
    mNormalizeProgram->AttachShader(normalizeVS);
    mNormalizeProgram->AttachShader(normalizeFS);
    mNormalizeProgram->LinkProgram();
    delete normalizeVS;
    delete normalizeFS;

    //Energy Transfer Shaders
    GLSLShader* transferVS = new GLSLShader("shaders/Transfer.vert", GL_VERTEX_SHADER);
    if( transferVS->HasErrors() )
    {
        Debug::Error( QString("shaders/Transfer.vert: %1").arg(transferVS->GetLog()) );
    }
    GLSLShader* transferFS = new GLSLShader("shaders/Transfer.frag", GL_FRAGMENT_SHADER);
    if( transferFS->HasErrors() )
    {
        Debug::Error( QString("shaders/Transfer.frag: %1").arg(transferFS->GetLog()) );
    }
    mTransferProgram = new GLSLProgram("Energy Transfer");
    mTransferProgram->AttachShader(transferVS);
    mTransferProgram->AttachShader(transferFS);
    mTransferProgram->LinkProgram();
    delete transferVS;
    delete transferFS;
}

void ObscuranceMap::SaveOpenGLStats()
{
    mPreviousDepthTest = glIsEnabled(GL_DEPTH_TEST);
    mPreviousCullFace = glIsEnabled(GL_CULL_FACE);
    mPreviousBlend = glIsEnabled(GL_BLEND);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &mPreviousClearColor[0]);
    glGetIntegerv(GL_VIEWPORT, &mPreviousViewport[0]);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &mBinded2DTexture);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &mBindedReadFramebuffer);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &mBindedDrawFramebuffer);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &mBindedRenderbuffer);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &mBindedPixelPackBuffer);
    glGetIntegerv(GL_CURRENT_PROGRAM, &mPreviousProgram);
    glGetFloatv(GL_POINT_SIZE, &mPreviousPointSize);
}

void ObscuranceMap::RestoreOpenGLStats()
{
    if(mPreviousDepthTest)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    if(mPreviousCullFace)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    if(mPreviousBlend)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glClearColor(mPreviousClearColor.r, mPreviousClearColor.g, mPreviousClearColor.b, mPreviousClearColor.a);
    glViewport(mPreviousViewport[0], mPreviousViewport[1], mPreviousViewport[2], mPreviousViewport[3]);
    glBindTexture(GL_TEXTURE_2D, mBinded2DTexture);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mBindedReadFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBindedDrawFramebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mBindedRenderbuffer);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mBindedPixelPackBuffer);
    glUseProgram(mPreviousProgram);
    glPointSize(mPreviousPointSize);
}

void ObscuranceMap::InitializeProjection()
{
    if(mBlueBuffer != NULL)
    {
        delete mBlueBuffer;
    }
    mBlueBuffer = new float[mResProjectionX * mResProjectionY * 4];
    for( int i = 0; i < mResProjectionX; i++ )
    {
        for( int j = 0; j < mResProjectionY; j++ )
        {
            mBlueBuffer[4 * (i * mResProjectionY + j) + 0] = 0.0f;
            mBlueBuffer[4 * (i * mResProjectionY + j) + 1] = 0.0f;
            mBlueBuffer[4 * (i * mResProjectionY + j) + 2] = 1.0f;
            mBlueBuffer[4 * (i * mResProjectionY + j) + 3] = 1.0f;
        }
    }

    glDeleteTextures(1, &mTexName);
    glGenTextures(1, &mTexName);
    glBindTexture(GL_TEXTURE_2D, mTexName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResProjectionX, mResProjectionY, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDeleteTextures(1, &mTexProjeccions[0]);
    glGenTextures(1, &mTexProjeccions[0]);
    glBindTexture(GL_TEXTURE_2D, mTexProjeccions[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResProjectionX, mResProjectionY, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDeleteFramebuffers(1, &mFbProjeccions);
    glGenFramebuffers(1, &mFbProjeccions);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbProjeccions);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexProjeccions[0],0);

    glDeleteRenderbuffers(1, &mTexProjeccions[1]);
    glGenRenderbuffers(1, &mTexProjeccions[1]);
    glBindRenderbuffer(GL_RENDERBUFFER, mTexProjeccions[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mResProjectionX, mResProjectionY);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mTexProjeccions[1]);

    glDeleteBuffers(2, &mProjectionsPBO[0]);
    glGenBuffers(2, &mProjectionsPBO[0]);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mProjectionsPBO[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, mResProjectionX*mResProjectionY*4*sizeof(float), mBlueBuffer, GL_STREAM_COPY);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mProjectionsPBO[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, mResProjectionX*mResProjectionY*4*sizeof(float), NULL, GL_STREAM_COPY);
}

void ObscuranceMap::InitializeLightmap()
{
    glDeleteTextures(1, &mTexTransfer);
    glGenTextures(1,&mTexTransfer);
    glBindTexture(GL_TEXTURE_2D, mTexTransfer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResLightmapX, mResLightmapY, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDeleteTextures(1, &mTexNormalize);
    glGenTextures(1,&mTexNormalize);
    glBindTexture(GL_TEXTURE_2D, mTexNormalize);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mResLightmapX, mResLightmapY, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDeleteFramebuffers(1, &mFbTransfer);
    glGenFramebuffers(1, &mFbTransfer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbTransfer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexTransfer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDeleteFramebuffers(1, &mFbLightmapNormalization);
    glGenFramebuffers(1, &mFbLightmapNormalization);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbLightmapNormalization);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexNormalize, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
}

float ObscuranceMap::VanDerCorput(int pI, int pBase) const
{
    float prec = 1.0f / (float)pBase;
    float f = 0.0f;
    while(pI != 0)
    {
        f += (float)(prec * (pI % pBase));
        pI /= pBase;
        prec /= pBase;
    }
    return f;
}

// Returns a random point in the surface of the sphere
glm::vec3 ObscuranceMap::RandomPoint(float pR, const glm::vec3& pC)
{
    mCount++;
    float ale1 = VanDerCorput(mCount, 2);
    float ale2 = VanDerCorput(mCount, 3);
    //float ale1= (float)rand()/(float)RAND_MAX;
    //float ale2= (float)rand()/(float)RAND_MAX;

    float alfa = 1.0 - 2.0 * ale1;	/* real between -1 and 1 */
    alfa = glm::acos(alfa);		/* angle between 0 and PI */

    float beta = 2.0f * 3.14159265358979f * ale2;	/* angle between 0 and 2*PI */

    glm::vec3 p;
    p.x = pC.x + pR * glm::sin(alfa) * glm::cos(beta);
    p.y = pC.y + pR * glm::sin(alfa) * glm::sin(beta);
    p.z = pC.z + pR * glm::cos(alfa);

    return p;
}

// Function that updates the lightmap.
// The number of directions used to calculate the lightmap is iterations * steps
void ObscuranceMap::UpdateTexture(int pIterations)
{
    mCount = 0;

    glGenQueries(1, &mQueryId);
    QProgressDialog progress(MainWindow::GetInstance());
    progress.setLabelText("Computing obscurances per polygon");
    progress.setCancelButton(0);
    progress.setRange(0, pIterations);
    progress.show();
    qApp->processEvents();
    unsigned int bufferSize = mResProjectionX*mResProjectionY*4*sizeof(float);
    for(int i = 0; i < pIterations; i++)
    {
        progress.setValue(i);
        qApp->processEvents();
        // Find a random point over the sphere
        glm::vec3 cameraPosition = RandomPoint(mEsferaRadius, mEsferaCenter);
        glBindFramebuffer(GL_FRAMEBUFFER, mFbProjeccions);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer( GL_PIXEL_PACK_BUFFER, mProjectionsPBO[0] );			// Bind The Buffer
        glBufferData( GL_PIXEL_PACK_BUFFER, bufferSize, mBlueBuffer, GL_STREAM_COPY );
        glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
        RenderProjeccions(cameraPosition, 1,1);
        RenderTransfer(1,1);
        int l = 0;
        bool somethingRendered = true;
        while(somethingRendered)
        {
            int pixelsRendered = RenderProjeccions(cameraPosition, l,0);
            somethingRendered = (pixelsRendered != 0);
            l = (l == 0)? 1:0;
            RenderTransfer(l,0);
        }
        glBindBuffer( GL_PIXEL_PACK_BUFFER, mProjectionsPBO[l] );			// Bind The Buffer
        glBufferData( GL_PIXEL_PACK_BUFFER, bufferSize, mBlueBuffer, GL_STREAM_COPY );
        glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
        l = (l = 0)? 1:0;
        RenderTransfer(l,1);
    }
    NormalizeLightmap();
    glDeleteQueries(1, &mQueryId);
}

// Function that copies the 16-bit fp RGBA buffer of the lightmap to a 32-bit RGBA fb buffer.
void ObscuranceMap::NormalizeLightmap()
{
    mNormalizeProgram->UseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, mFbLightmapNormalization);

    glEnable(GL_CULL_FACE);

    glViewport(0, 0, mResLightmapX, mResLightmapY);

    glm::mat4 projection = glm::ortho(-(float)mResLightmapX, (float)mResLightmapY, -(float)mResLightmapX, (float)mResLightmapY);
    glm::mat4 modelView = glm::mat4(1.0f);

    mNormalizeProgram->BindTexture(GL_TEXTURE_2D, "texture", mTexTransfer, 0);
    mNormalizeProgram->SetUniform("res", (float)mResLightmapX);
    mNormalizeProgram->SetUniform("modelViewProjection", projection * modelView);

    //Draw the geometry.
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0, 0);  glVertex3f(-mResLightmapX, -mResLightmapY, -0.5f);
        glTexCoord2f(1, 0);  glVertex3f( mResLightmapX, -mResLightmapY, -0.5f);
        glTexCoord2f(1, 1);  glVertex3f( mResLightmapX,  mResLightmapY, -0.5f);
        glTexCoord2f(0, 1);  glVertex3f(-mResLightmapX,  mResLightmapY, -0.5f);
    }
    glEnd();
}

// Function that renders the depth peeling geometry planes and stores them in PBOs.
unsigned int ObscuranceMap::RenderProjeccions(const glm::vec3& pCameraPosition, int pBuffer, int pFirst)
{
    mProjectionProgram->UseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, mFbProjeccions);

    glEnable(GL_DEPTH_TEST);									// Enable Depth Testing
    glDepthFunc(GL_LESS);

    glDisable(GL_CULL_FACE);

    glViewport(0, 0, mResProjectionX, mResProjectionY);

    glm::mat4 projection = glm::ortho(-mEsferaRadius, mEsferaRadius, -mEsferaRadius, mEsferaRadius, 0.0f, mEsferaRadius * 2.0f);
    glm::mat4 modelView = glm::lookAt( pCameraPosition, mEsferaCenter, glm::vec3(0.0f, 1.0f, 0.0f) );

    mProjectionProgram->SetUniform("modelViewProjection", projection * modelView);
    mProjectionProgram->SetUniform("modelViewInverseTranspose", glm::inverseTranspose(modelView));
    mProjectionProgram->SetUniform("res", (float)mResProjectionX);
    mProjectionProgram->SetUniform("first", (float)pFirst);
    mProjectionProgram->SetUniform("polygonalTexturesSize", mResLightmapX);

    int other = ((pBuffer == 1)? 0:1);
    if(pFirst == 0)
    {
        glBindBuffer( GL_PIXEL_PACK_BUFFER, mProjectionsPBO[other] );
        glBindTexture(GL_TEXTURE_2D, mTexName);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, mResProjectionX, mResProjectionY);
        glBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mProjectionProgram->BindTexture(GL_TEXTURE_2D, "ztex", mTexName, 0);
    }

    glBeginQuery(GL_SAMPLES_PASSED, mQueryId);
    int processedPolygons = 0;
    for( int i = 0; i < mScene->GetNumberOfMeshes(); i++ )
    {
        Geometry* currentMesh = mScene->GetMesh(i);

        mProjectionProgram->SetUniform("offset", processedPolygons);
        currentMesh->Draw();
        processedPolygons += currentMesh->GetNumFaces();
    }
    glEndQuery(GL_SAMPLES_PASSED);
    unsigned int sampleCount;
    glGetQueryObjectuiv(mQueryId, GL_QUERY_RESULT, &sampleCount);

    //Copy the color buffer to PBO.
    glBindBuffer( GL_PIXEL_PACK_BUFFER, mProjectionsPBO[pBuffer] );			// Bind The Buffer
    glReadPixels(0, 0, mResProjectionX, mResProjectionY, GL_RGBA, GL_FLOAT, BUFFER_OFFSET(0));
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    return sampleCount;
}

// Function that calculates the lightmap using the neighbouring projections.
void ObscuranceMap::RenderTransfer(int pBuffer, int pFirst)
{
    mTransferProgram->UseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, mFbTransfer);

    int other = ((pBuffer == 1)? 0:1);

    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_BLEND);

    glViewport(0, 0, mResLightmapX, mResLightmapY);

    mTransferProgram->SetUniform("dmax", 0.3f);

    glEnableVertexAttribArray(POSITION);
    glEnableVertexAttribArray(TEXTCOORD);
    if(pFirst == 1)
    {
        //First Render
        mTransferProgram->SetUniform("direction", (float)pBuffer);

        glBindBuffer( GL_ARRAY_BUFFER, mProjectionsPBO[pBuffer] );
        glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer( GL_ARRAY_BUFFER, mProjectionsPBO[other] );
        glVertexAttribPointer(TEXTCOORD, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_POINTS, 0, mResProjectionX*mResProjectionY );
    }
    else
    {
        mTransferProgram->SetUniform("direction", 0.0f);

        glBindBuffer( GL_ARRAY_BUFFER, mProjectionsPBO[pBuffer] );
        glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer( GL_ARRAY_BUFFER, mProjectionsPBO[other] );
        glVertexAttribPointer(TEXTCOORD, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_POINTS, 0, mResProjectionX*mResProjectionY );

        mTransferProgram->SetUniform("direction", 1.0f);

        glBindBuffer( GL_ARRAY_BUFFER, mProjectionsPBO[other] );
        glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer( GL_ARRAY_BUFFER, mProjectionsPBO[pBuffer] );
        glVertexAttribPointer(TEXTCOORD, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_POINTS, 0, mResProjectionX*mResProjectionY );
    }
    glDisableVertexAttribArray(POSITION);
    glDisableVertexAttribArray(TEXTCOORD);

    glDisable(GL_BLEND);
}
