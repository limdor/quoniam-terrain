/// \file SphereOfViewpoints.h
/// \class SphereOfViewpoints
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _SPHERE_OF_VIEWPOINTS_H_
#define _SPHERE_OF_VIEWPOINTS_H_

//Project includes
#include "Camera.h"
#include "SpherePointCloud.h"

class SphereOfViewpoints : public SpherePointCloud
{
public:
    /// Constructor
    SphereOfViewpoints(float pAngle, float pAspectRatio);
    /// Destructor
    ~SphereOfViewpoints();

    /// Generate 10*4^depth+2 viewpoints quasi-uniformly distributed using the recursive
    /// subdivision of faces, starting from an icosahedron
    void SetToQuasiUniform(unsigned char pDepth);

protected:
    /// Set a camera for every point
    void SetCameras();
    /// Field of view
    float mAngle;
    /// Aspect ratio
    float mAspectRatio;
    /// Array of viewpoints
    QVector< Camera * > mCameras;
};

#endif
