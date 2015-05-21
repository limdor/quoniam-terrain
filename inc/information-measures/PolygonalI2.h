/// \file PolygonalI2.h
/// \class PolygonalI2
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _POLYGONAL_I2_H_
#define _POLYGONAL_I2_H_

#include "Measure.h"

class PolygonalI2 : public Measure
{
public:
    PolygonalI2(const QString &pName);
    ~PolygonalI2();

    void Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram);
};

#endif
