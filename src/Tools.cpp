//Definition include
#include "Tools.h"

//System includes
#include <math.h>

//Dependency includes
#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "glm/geometric.hpp"

//Qt includes
#include <QDir>
#include <QPair>
#include <QtAlgorithms>

//Project includes
#include "Debug.h"

bool pairCompareX (QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > pI, QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > pJ)
{
    return ( pI.second.second.x < pJ.second.second.x );
}

bool pairCompareY (QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > pI, QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > pJ)
{
    return ( pI.second.second.y < pJ.second.second.y );
}

bool pairCompareZ (QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > pI, QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > pJ)
{
    return ( pI.second.second.z < pJ.second.second.z );
}

QVector< int > Tools::GetOrderedIndexesByDimension(QVector< QPair< QPair< Geometry*, int >, glm::vec3 > >& pValues, int pDimension)
{
    int size = pValues.size();
    QVector< QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > > toSort(size);

    for( int i = 0; i < size; i++ )
    {
        toSort[i] = QPair< int, QPair< QPair< Geometry*, int >, glm::vec3 > > ( i, pValues.at(i) );
    }

    switch (pDimension)
    {
    case 0:
        qSort(toSort.begin(), toSort.end(),pairCompareX);
        break;
    case 1:
        qSort(toSort.begin(), toSort.end(),pairCompareY);
        break;
    case 2:
        qSort(toSort.begin(), toSort.end(),pairCompareZ);
        break;
    }

    QVector< int > result(size);
    for( int i = 0; i < size; i++ )
    {
        result[i] = toSort.at(i).first;
        pValues[i] = toSort.at(i).second;
    }

    return result;
}

template <class T>
bool pairCompare (QPair<int, T> i,QPair<int, T> j)
{
    return ( i.second < j.second );
}

QVector< int > Tools::GetOrderedIndexes(const QVector< float >& pValues)
{
    unsigned int size = pValues.size();

    QVector< QPair<int, float> > toSort(size);
    for( unsigned int i = 0; i < size; i++ )
    {
        toSort[i] = QPair<int, float>( i, pValues.at(i) );
    }

    qSort( toSort.begin(), toSort.end(), pairCompare<float> );

    QVector< int > result(size);
    for( unsigned int i = 0; i < size; i++ )
    {
        result[i] = toSort.at(i).first;
    }

    return result;
}

QVector< int > Tools::GetPositions(const QVector< int >& pValues)
{
    int size = pValues.size();

    QVector< QPair< int, int> > toSort(size);
    for( int i = 0; i < size; i++ )
    {
        toSort[i] = QPair< int, int >( i, pValues.at(i) );
    }

    qSort( toSort.begin(), toSort.end(), pairCompare<int> );

    QVector< int > result(size);
    for( int i = 0; i < size; i++ )
    {
        result[i] = toSort.at(i).first;
    }

    return result;
}

QVector< glm::vec4 > Tools::ConvertFloatsToColors(const QVector< float >& pValues, bool pInverted)
{
    return ConvertNormalizedFloatsToColors( ScaleValues(pValues, 0.0f, 1.0f), pInverted );
}

QVector< float > Tools::ScaleValues(const QVector< float >& pValues, float pLowerBound, float pUpperBound, float pPercentOfClipping)
{
    float min, max;
    unsigned int size = pValues.size();
    if(pPercentOfClipping == 0.0f)
    {
        min = FLT_MAX;
        max = -FLT_MAX;
        for( unsigned int i = 0; i < size; i++ )
        {
            if(pValues.at(i) > max)
            {
                max = pValues.at(i);
            }
            if(pValues.at(i) < min)
            {
                min = pValues.at(i);
            }
        }
    }
    else
    {
        int offset = glm::round(size * (pPercentOfClipping / 200.0f));
        QVector< float > orderedValues = pValues;
        qSort(orderedValues);
        min = orderedValues.at(offset);
        max = orderedValues.at(size - 1 - offset);
    }
    float scale = (pUpperBound - pLowerBound) / (max - min);
    QVector< float > results(size);
    for( unsigned int i = 0; i < size; i++ )
    {
        results[i] = ( glm::clamp(pValues.at(i), min, max) - min ) * scale + pLowerBound;
    }
    return results;
}

float Tools::Mean(const QVector< float >& pValues, const QVector< float >& pWeights, float pPower)
{
    double value = 0.0f;

    if( pValues.size() == pWeights.size() )
    {
        float sumWeights = 0.0f;
        for( int i = 0; i < pValues.size(); i++ )
        {
            value += glm::pow(1.0f - pValues.at(i), pPower) * (double)pWeights.at(i);
            sumWeights += pWeights.at(i);
        }
        value /= sumWeights;
    }
    else
    {
        for( int i = 0; i < pValues.size(); i++ )
        {
            value += glm::pow(pValues.at(i), pPower);
        }
        value /= pValues.size();
    }
    return value;
}

QVector< glm::vec4 > Tools::ConvertNormalizedFloatsToColors(const QVector< float >& pValues, bool pInverted)
{
    unsigned int size = pValues.size();

    QVector< glm::vec4 > results(size);
    for( unsigned int i = 0; i < size; i++ )
    {
        results[i] = ConvertNormalizedFloatToColor( pValues.at(i), pInverted );
    }

    return results;
}

glm::vec4 Tools::ConvertNormalizedFloatToColor(float pValue, bool pInverted)
{
    float percent;

    if(pInverted)
    {
        percent = 1.0f - pValue;
    }
    else
    {
        percent = pValue;
    }
    float value = percent;
    if ( percent < 0.25f )
    {
        percent = percent * 4;// / 0.25;
        return glm::vec4( 0.0, percent, 1.0, value );
    }
    else if ( percent < 0.5f )
    {
        percent = (percent - 0.25f) * 4;// / 0.25;
        return glm::vec4( 0.0 , 1.0 , 1.0 - percent, value );
    }
    else if ( percent < 0.75f )
    {
        percent = (percent - 0.5f) * 4;// / 0.25;
        return glm::vec4( percent , 1.0 , 0.0, value );
    }
    else
    {
        percent = (percent - 0.75f) * 4;// / 0.25;
        return glm::vec4( 1.0 , 1.0 - percent , 0.0, value );
    }
}

float Tools::TriangleArea(const glm::vec3& pA, const glm::vec3& pB, const glm::vec3& pC)
{
    glm::vec3 v = glm::cross( pB - pA, pC - pA );

    return glm::length(v) / 2.0f;
}

QString Tools::GetProgramPath()
{
    QString picturesPath, completePath;

    QString programFolder = QString("Quoniam");
#if defined(Q_OS_WIN32)
    picturesPath = QString( qgetenv("USERPROFILE") ) + QString("\\Pictures");
    completePath = picturesPath + "\\" + programFolder + "\\";
#elif defined(Q_OS_MAC)
    picturesPath = QString( qgetenv("HOME") ) + QString("/Pictures");
    completePath = picturesPath + "/" + programFolder + "/";
#else
    Debug::Warning("I'm on a unidentified operating system!");
#endif
    if(!QDir(completePath).exists())
    {
        if(!QDir(picturesPath).mkdir(programFolder))
        {
            Debug::Warning( QString("Impossible to create the %1 folder!").arg(programFolder) );
        }
    }
    return completePath;
}
