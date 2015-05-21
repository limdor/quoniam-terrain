/// \file HistogramBuilder.h
/// \class HistogramBuilder
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _HISTOGRAM_BUILDER_H_
#define _HISTOGRAM_BUILDER_H_

//Dependency includes
#include "VisibilityChannelHistogram.h"

//Project includes
#include "GLSLProgram.h"
#include "Scene.h"
#include "ViewpointsMesh.h"

/// Static class to create an InformationChannelHistogram
class HistogramBuilder
{
public:
    /// Create an InformationChannelHistogram given the Scene and the ViewpointsMesh
    static VisibilityChannelHistogram* CreateHistogram(Scene* pScene, ViewpointsMesh* pViewpointsMesh, int pWidthResolution, bool pFaceCulling, bool pIgnoreNormals = false);
};

#endif
