//Definition include
#include "PolygonalI3.h"

//System includes
#include <float.h>

//Dependency includes
#include "glm/exponential.hpp"

//Project includes
#include "Tools.h"

PolygonalI3::PolygonalI3(const QString &pName): Measure(pName)
{

}

PolygonalI3::~PolygonalI3()
{

}

void PolygonalI3::Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram)
{
    QVector< int > elementsOutOfDomain;

    int numberOfPolygons = pVisibilityChannelHistogram->GetNumberOfPolygons();
    int numberOfViewpoints = pVisibilityChannelHistogram->GetNumberOfViewpoints();

    mValues.fill( 0.0f, numberOfPolygons );

    //The viewpoint I2 is computed first
    QVector< float > viewpointI2(numberOfViewpoints, 0.0f);
    unsigned int sum_a_t = pVisibilityChannelHistogram->GetTotalSum();
    for( int currentViewpoint = 0; currentViewpoint < numberOfViewpoints; currentViewpoint++ )
    {
        unsigned int a_t = pVisibilityChannelHistogram->GetSumPerViewpoint(currentViewpoint);
        float sumAux1 = 0.0f;
        float sumAux2 = 0.0f;
        for( int currentPolygon = 0; currentPolygon < numberOfPolygons; currentPolygon++ )
        {
            unsigned int sum_a_z = pVisibilityChannelHistogram->GetSumPerPolygon(currentPolygon);
            unsigned int a_z = pVisibilityChannelHistogram->GetValue(currentViewpoint, currentPolygon);
            if( sum_a_z != 0)
            {
                if( a_z != 0 )
                {
                    float aux1 = a_z / (float)a_t;
                    sumAux1 += aux1 * glm::log2(aux1);
                }
                float aux2 = sum_a_z / (float)sum_a_t;
                sumAux2 += aux2 * glm::log2(aux2);
            }
        }
        viewpointI2[currentViewpoint] = (sumAux1 - sumAux2);
    }

    //Now I3 is computed using viewpoint I2
    float minValue = FLT_MAX;
    for( int currentPolygon = 0; currentPolygon < numberOfPolygons; currentPolygon++ )
    {
        unsigned int sum_a_z = pVisibilityChannelHistogram->GetSumPerPolygon(currentPolygon);
        if( sum_a_z != 0 )
        {
            for( int currentViewpoint = 0; currentViewpoint < numberOfViewpoints; currentViewpoint++ )
            {
                unsigned int a_z = pVisibilityChannelHistogram->GetValue(currentViewpoint, currentPolygon);
                if( a_z != 0 )
                {
                    float aux = a_z / (float)sum_a_z;
                    mValues[currentPolygon] += aux * viewpointI2.at(currentViewpoint);
                }
            }
            if( mValues.at(currentPolygon) < minValue )
            {
                minValue = mValues.at(currentPolygon);
            }
        }
        else
        {
            elementsOutOfDomain.push_back(currentPolygon);
        }
    }
    //Els elements fora del domini se'ls hi assigna un valor per defecte que correspon al valor mínim
    for( int currentPolygon = 0; currentPolygon < elementsOutOfDomain.size(); currentPolygon++ )
    {
        mValues[elementsOutOfDomain.at(currentPolygon)] = minValue;
    }
    mScaledValues = Tools::ScaleValues( mValues, 0.0f, 1.0f, 0.1f );

    mSort = Tools::GetOrderedIndexes(mValues);
    mPositions = Tools::GetPositions(mSort);
    mComputed = true;
}
