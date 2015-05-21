/// \file PolygonalI3.h
/// \class PolygonalI3
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _POLYGONAL_I3_H_
#define _POLYGONAL_I3_H_

#include "Measure.h"

class PolygonalI3 : public Measure
{
public:
    PolygonalI3(const QString &pName);
    ~PolygonalI3();

    void Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram);
};

#endif
