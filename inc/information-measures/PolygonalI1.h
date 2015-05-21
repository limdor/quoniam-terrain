/// \file PolygonalI1.h
/// \class PolygonalI1
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _POLYGONAL_I1_H_
#define _POLYGONAL_I1_H_

#include "Measure.h"

class PolygonalI1 : public Measure
{
public:
    PolygonalI1(const QString& pName);
    ~PolygonalI1();

    void Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram);
};

#endif
