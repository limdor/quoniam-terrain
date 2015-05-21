//Definition include
#include "SphereOfViewpoints.h"

//Project includes
#include "PerspectiveCamera.h"

SphereOfViewpoints::SphereOfViewpoints(float pAngle, float pAspectRatio):
    mAngle(pAngle), mAspectRatio(pAspectRatio)
{
    mMesh->SetName("Sphere of viewpoints");
}

SphereOfViewpoints::~SphereOfViewpoints()
{
    for( int i = 0; i < mNumberOfPoints; i++ )
    {
        delete mCameras[i];
    }
}

void SphereOfViewpoints::SetToQuasiUniform(unsigned char pDepth)
{
    SpherePointCloud::SetToQuasiUniform(pDepth);
    SetCameras();
}

void SphereOfViewpoints::SetCameras()
{
    mCameras.resize(mNumberOfPoints);
    for( int i = 0; i < mNumberOfPoints; i++ )
    {
        mCameras[i] = new PerspectiveCamera( 0.05f, 3.0f, glm::vec3(0.0f), SpherePointCloud::Up(mVertices.at(i)), mVertices.at(i), mAngle, mAspectRatio );
        mCameras[i]->mName = QString("%1").arg(i);
    }
}
