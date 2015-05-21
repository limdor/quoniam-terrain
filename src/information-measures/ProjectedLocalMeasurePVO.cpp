#include "ProjectedLocalMeasurePVO.h"
#include "Tools.h"
#include "Debug.h"
#include "PolygonalI2.h"

ProjectedLocalMeasurePVO::ProjectedLocalMeasurePVO(const QString &pName): Measure(pName)
{
    mLocalMeasure = NULL;
    mLocalMeasureCreated = false;
    mScaleLocalMeasure = false;
    mScaleLocalMeasureLowerBound = 0.0f;
    mScaleLocalMeasureUpperBound = 1.0f;
}

ProjectedLocalMeasurePVO::~ProjectedLocalMeasurePVO()
{
    if(mLocalMeasureCreated)
    {
        delete mLocalMeasure;
    }
}

void ProjectedLocalMeasurePVO::Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram)
{
    QVector< float > scaledPolygonalMeasure;

    int numberOfViewpoints = pVisibilityChannelHistogram->GetNumberOfViewpoints();
    int numberOfPolygons = pVisibilityChannelHistogram->GetNumberOfPolygons();

    mValues.fill( 0.0f, numberOfViewpoints );

    if( mLocalMeasure == NULL )
    {
        mLocalMeasure = new PolygonalI2("I2");
        mLocalMeasureCreated = true;
        Debug::Warning("ProjectedLocalMeasurePVO::Dependencies have been set automatically!");
    }

    if(!mLocalMeasure->Computed())
    {
        mLocalMeasure->Compute(pVisibilityChannelHistogram);
    }
    if(mScaleLocalMeasure)
    {
        scaledPolygonalMeasure = Tools::ScaleValues(mLocalMeasure->GetValues(), mScaleLocalMeasureLowerBound, mScaleLocalMeasureUpperBound );
    }
    else
    {
        scaledPolygonalMeasure = mLocalMeasure->GetValues();
    }
    for( int currentViewpoint = 0; currentViewpoint < numberOfViewpoints; currentViewpoint++ )
    {
        for( int currentPolygon = 0; currentPolygon < numberOfPolygons; currentPolygon++ )
        {
            unsigned int a_z = pVisibilityChannelHistogram->GetValue(currentViewpoint, currentPolygon);
            unsigned int sum_a_z = pVisibilityChannelHistogram->GetSumPerPolygon(currentPolygon);

            if( a_z != 0 )
            {
                float aux = a_z / (float)sum_a_z;
                mValues[currentViewpoint] += aux * scaledPolygonalMeasure.at(currentPolygon);
            }
        }
    }
    mScaledValues = Tools::ScaleValues( mValues, 0.0f, 1.0f );

    mSort = Tools::GetOrderedIndexes(mValues);
    mPositions = Tools::GetPositions(mSort);
    mComputed = true;
}

void ProjectedLocalMeasurePVO::AddDpendencyLocalMeasure(Measure *pLocalMeasure)
{
    if(mLocalMeasureCreated)
    {
        delete mLocalMeasure;
        mLocalMeasureCreated = false;
    }
    mLocalMeasure = pLocalMeasure;
}

void ProjectedLocalMeasurePVO::SetScaleDependencyLocalMeasure(float pLowerBound, float pUpperBound)
{
    mScaleLocalMeasureLowerBound = pLowerBound;
    mScaleLocalMeasureUpperBound = pUpperBound;
    mScaleLocalMeasure = true;
}

bool ProjectedLocalMeasurePVO::IsLocalMeasureScaled() const
{
    return mScaleLocalMeasure;
}

float ProjectedLocalMeasurePVO::GetScaleLowerBound() const
{
    return mScaleLocalMeasureLowerBound;
}

float ProjectedLocalMeasurePVO::GetScaleUpperBound() const
{
    return mScaleLocalMeasureUpperBound;
}
