/// \file Tools.h
/// \class Tools
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _TOOLS_H_
#define _TOOLS_H_

//Qt includes
#include <QPair>

//Dependency includes
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

//Project includes
#include "Geometry.h"

class Tools
{
public:
    static QVector< int > GetOrderedIndexesByDimension(QVector<QPair<QPair<Geometry *, int>, glm::vec3> > &pValues, int pDimension);
    static QVector< int > GetOrderedIndexes(const QVector< float >& pValues);
    static QVector< int > GetPositions(const QVector< int >& pValues);
    static QVector< glm::vec4 > ConvertFloatsToColors(const QVector< float >& pValues, bool pInverted);
    static QVector< glm::vec4 > ConvertNormalizedFloatsToColors(const QVector< float >& pValues, bool pInverted);
    static QVector< float > ScaleValues(const QVector< float >& pValues, float pLowerBound, float pUpperBound, float pPercentOfClipping = 0.0f);
    static float Mean(const QVector< float >& pValues, const QVector<float>& pWeights = QVector<float>(), float pPower = 1.0f);

    static float TriangleArea(const glm::vec3& pA, const glm::vec3& pB, const glm::vec3& pC);

    static QString GetProgramPath();
private:    
    static glm::vec4 ConvertNormalizedFloatToColor(float pValue, bool pInverted);
};

#endif
