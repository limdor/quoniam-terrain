//Definition include
#include "PolygonalI2.h"

//System includes
#include <float.h>

//Dependency includes
#include "glm/exponential.hpp"

//Project includes
#include "Tools.h"

PolygonalI2::PolygonalI2(const QString &pName): Measure(pName)
{

}

PolygonalI2::~PolygonalI2()
{

}

void PolygonalI2::Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram)
{
    QVector< int > elementsOutOfDomain;

    int numberOfPolygons = pVisibilityChannelHistogram->GetNumberOfPolygons();
    int numberOfViewpoints = pVisibilityChannelHistogram->GetNumberOfViewpoints();

    mValues.fill( 0.0f, numberOfPolygons );

    float maxValue = -FLT_MAX;
    unsigned int sum_a_t = pVisibilityChannelHistogram->GetTotalSum();
    for( int currentPolygon = 0; currentPolygon < numberOfPolygons; currentPolygon++ )
    {
        unsigned int sum_a_z = pVisibilityChannelHistogram->GetSumPerPolygon(currentPolygon);
        if( sum_a_z != 0 )
        {
            float sumAux1 = 0.0f;
            float sumAux2 = 0.0f;
            for( int currentViewpoint = 0; currentViewpoint < numberOfViewpoints; currentViewpoint++ )
            {
                unsigned int a_t = pVisibilityChannelHistogram->GetSumPerViewpoint(currentViewpoint);
                unsigned int a_z = pVisibilityChannelHistogram->GetValue(currentViewpoint, currentPolygon);
                if( a_t != 0 )
                {
                    float aux1 = a_t / (float)sum_a_t;
                    sumAux1 += aux1 * glm::log2(aux1);
                    if( a_z != 0 )
                    {
                        float aux2 = a_z / (float)sum_a_z;
                        sumAux2 += aux2 * glm::log2(aux2);
                    }
                }
            }
            mValues[currentPolygon] = - sumAux1 + sumAux2;
            if( mValues.at(currentPolygon) > maxValue )
            {
                maxValue = mValues.at(currentPolygon);
            }
        }
        else
        {
            elementsOutOfDomain.push_back(currentPolygon);
        }
    }
    //Els elements fora del domini se'ls hi assigna un valor per defecte que correspon al valor màxim
    for( int currentPolygon = 0; currentPolygon < elementsOutOfDomain.size(); currentPolygon++ )
    {
        mValues[elementsOutOfDomain.at(currentPolygon)] = maxValue;
    }
    mScaledValues = Tools::ScaleValues( mValues, 0.0f, 1.0f, 0.1f );

    mSort = Tools::GetOrderedIndexes(mValues);
    mPositions = Tools::GetPositions(mSort);
    mComputed = true;
}
