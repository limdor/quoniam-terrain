//Definition include
#include "HistogramBuilder.h"

//GLEW has to be included before any OpenGL include
#include "glew.h"

//Qt includes
#include <QApplication>
#include <QProgressDialog>
#include <QTime>
#include <QVector>

//Dependency includes
#include "glm/common.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/type_ptr.hpp"

//Project includes
#include "Debug.h"
#include "GLSLProgram.h"
#include "GLSLShader.h"
#include "GPUGeometry.h"
#include "MainWindow.h"

VisibilityChannelHistogram* HistogramBuilder::CreateHistogram(Scene* pScene, ViewpointsMesh* pViewpointsMesh, int pWidthResolution, bool pFaceCulling, bool pIgnoreNormals)
{
    int windowHeight;
    unsigned int j, totalNumberOfPixels;
    GLuint renderBuffer, frameBuffer, depthRenderBuffer;
    QVector< unsigned int > facesAreas;
    glm::vec4 previousClearColor;
    GLint previousViewport[4];
    Camera* currentViewpoint;

    int windowWidth = pWidthResolution;
    int numberOfPolygons = pScene->GetNumberOfPolygons();
    int numberOfViewpoints = pViewpointsMesh->GetNumberOfViewpoints();

    VisibilityChannelHistogram* histogram = new VisibilityChannelHistogram(numberOfViewpoints, numberOfPolygons);
    GLSLShader* basicVS = new GLSLShader("shaders/Basic.vert", GL_VERTEX_SHADER);
    if( basicVS->HasErrors() )
    {
        Debug::Error( QString("shaders/Basic.vert: %1").arg(basicVS->GetLog()) );
    }
    GLSLShader* colorPerFaceFS = new GLSLShader("shaders/ColorPerFace.frag", GL_FRAGMENT_SHADER);
    if( colorPerFaceFS->HasErrors() )
    {
        Debug::Error( QString("shaders/ColorPerFace.frag: %1").arg(colorPerFaceFS->GetLog()) );
    }

    GLSLProgram* shaderColorPerFace = new GLSLProgram("ShaderColorPerFace");
    shaderColorPerFace->AttachShader(basicVS);
    shaderColorPerFace->AttachShader(colorPerFaceFS);
    shaderColorPerFace->LinkProgram();

    QApplication::setOverrideCursor( Qt::WaitCursor );
    QTime t;
    t.start();
    QProgressDialog progress(MainWindow::GetInstance());
    progress.setLabelText("Projecting scene to viewpoint sphere...");
    progress.setCancelButton(0);
    progress.setRange(0, numberOfViewpoints);
    progress.show();

    //Guardem els estats abans del mètode
    bool previousDepthTest = glIsEnabled(GL_DEPTH_TEST);
    bool previousCullFace = glIsEnabled(GL_CULL_FACE);
    bool previousBlend = glIsEnabled(GL_BLEND);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &previousClearColor[0]);
    glGetIntegerv(GL_VIEWPORT, &previousViewport[0]);

    glEnable(GL_DEPTH_TEST);
    if(pFaceCulling && !pIgnoreNormals)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    glDisable(GL_BLEND);

    //Creació del RenderBuffer
    glGenRenderbuffers( 1, &renderBuffer );
    glGenRenderbuffers( 1, &depthRenderBuffer );
    //Creació del FrameBuffer
    glGenFramebuffers( 1, &frameBuffer );
    //Creació del FrameBuffer
    glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
    CHECK_GL_ERROR();

    //Inicialització per pintar
    shaderColorPerFace->UseProgram();
    shaderColorPerFace->SetUniform("ignoreNormals", pIgnoreNormals);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    int previousHeight = -1;
    float* storedPixels = NULL;
    //Recorrem els viewpoints de l'esfera
    for( int i = 0; i < numberOfViewpoints; i++ )
    {
        currentViewpoint = pViewpointsMesh->GetViewpoint(i);
        windowHeight = (int)(windowWidth / currentViewpoint->GetAspectRatio());

        if(windowHeight != previousHeight)
        {
            previousHeight = windowHeight;

            //Configuració del RenderBuffer
            glBindRenderbuffer( GL_RENDERBUFFER, renderBuffer );
            glRenderbufferStorage( GL_RENDERBUFFER, GL_R32F, windowWidth, windowHeight );
            CHECK_GL_ERROR();

            glBindRenderbuffer( GL_RENDERBUFFER, depthRenderBuffer );
            glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight );
            CHECK_GL_ERROR();

            glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);
            CHECK_GL_ERROR();
            glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
            CHECK_GL_ERROR();
#ifdef QT_DEBUG
            if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                CHECK_GL_ERROR();
#endif

            //Calculem el nombre total de píxels
            totalNumberOfPixels = ((unsigned int)windowWidth * (unsigned int)windowHeight);

            if(storedPixels != NULL)
            {
                delete[] storedPixels;
            }
            //Espai on guardarem els pixels de cada draw
            storedPixels = new float [windowWidth * windowHeight];

            glViewport( 0, 0, windowWidth, windowHeight );
        }

        progress.setValue(i);

        //Inicialitzacio de l'area projectada de cada objecte
        facesAreas.fill( 0, numberOfPolygons );

        //Netegem el buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        //Calculem la matriu de la càmera
        glm::mat4 viewCamera = currentViewpoint->GetViewMatrix();
        glm::mat4 projectionCamera = currentViewpoint->GetProjectionMatrix();
        glUniformMatrix4fv( shaderColorPerFace->GetUniformLocation("modelViewProjection"), 1, GL_FALSE, glm::value_ptr(projectionCamera * viewCamera));
        CHECK_GL_ERROR();

        //Inicialitzem el nombre de poligons processats
        int processedPolygons = 0;

        //Recorrem els meshos
        for( int k = 0; k < pScene->GetNumberOfMeshes(); k++ )
        {
            glUniform1i( shaderColorPerFace->GetUniformLocation("offset"), processedPolygons);
            pScene->GetMesh(k)->Draw();
            processedPolygons += pScene->GetMesh(k)->GetNumFaces();
        }
        glFlush();

        //Obtenim els pixels que s'han pintat en el FrameBuffer
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, windowWidth, windowHeight, GL_RED, GL_FLOAT, storedPixels);
        CHECK_GL_ERROR();

        //SaveScreenshot( QString("Projection_%1.jpg").arg(i) );
        for(j = 0; j < totalNumberOfPixels; j++ )
        {
            int pixelActual = glm::round(storedPixels[j]);

            if(pixelActual > 0)
            {
                Q_ASSERT(pixelActual <= numberOfPolygons);
                facesAreas[pixelActual - 1]++;
            }
        }
        histogram->SetValues(i, facesAreas);
    }
    if(storedPixels != NULL)
    {
        delete[] storedPixels;
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //Eliminacio del frambuffer i els 2 renderbuffer
    glDeleteFramebuffers( 1, &frameBuffer);
    glDeleteRenderbuffers( 1, &depthRenderBuffer);
    glDeleteRenderbuffers( 1, &renderBuffer );

    histogram->Compute();
    Debug::Log( QString("GLCanvas::ComputeViewpointsProbabilities %1x%2 - Time elapsed: %3 ms").arg(windowWidth).arg(windowHeight).arg(t.elapsed()) );

    //Restaurem els estats abans del mètode
    if(previousDepthTest)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    if(previousCullFace)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    if(previousBlend)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glClearColor(previousClearColor.r, previousClearColor.g, previousClearColor.b, previousClearColor.a);
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);

    progress.hide();

    QApplication::restoreOverrideCursor();

    return histogram;
}
