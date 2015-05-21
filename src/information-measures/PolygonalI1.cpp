//Definition include
#include "PolygonalI1.h"

//Dependency includes
#include <glm/exponential.hpp>

//Project includes
#include "Tools.h"

PolygonalI1::PolygonalI1(const QString &pName): Measure(pName)
{

}

PolygonalI1::~PolygonalI1()
{

}

void PolygonalI1::Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram)
{
    QVector< int > elementsOutOfDomain;

    int numberOfPolygons = pVisibilityChannelHistogram->GetNumberOfPolygons();
    int numberOfViewpoints = pVisibilityChannelHistogram->GetNumberOfViewpoints();

    mValues.fill( 0.0f, numberOfPolygons );

    float maxValue = -FLT_MAX;
    for( int currentPolygon = 0; currentPolygon < numberOfPolygons; currentPolygon++ )
    {
        unsigned int sum_a_z = pVisibilityChannelHistogram->GetSumPerPolygon(currentPolygon);
        if( sum_a_z != 0 )
        {
            for( int currentViewpoint = 0; currentViewpoint < numberOfViewpoints; currentViewpoint++ )
            {
                unsigned int a_z = pVisibilityChannelHistogram->GetValue( currentViewpoint, currentPolygon );
                unsigned int a_t = pVisibilityChannelHistogram->GetSumPerViewpoint(currentViewpoint);
                if( a_z != 0 )
                {
                    unsigned int sum_a_t = pVisibilityChannelHistogram->GetTotalSum();
                    float aux = a_z / (float)a_t;
                    mValues[currentPolygon] += a_z * glm::log2( aux * ( sum_a_t / (float) sum_a_z ) );
                }
            }
            mValues[currentPolygon] /= sum_a_z;
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
