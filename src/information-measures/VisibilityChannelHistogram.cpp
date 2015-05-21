#include "VisibilityChannelHistogram.h"

VisibilityChannelHistogram::VisibilityChannelHistogram(int pNumberOfViewpoints, int pNumberOfPolygons):
    mNumberOfViewpoints(pNumberOfViewpoints), mNumberOfPolygons(pNumberOfPolygons), mTotalSum(0)
{
    mValues.resize(pNumberOfViewpoints);
    for( int currentViewpoint = 0; currentViewpoint < pNumberOfViewpoints; currentViewpoint++ )
    {
        mValues[currentViewpoint].fill(0, pNumberOfPolygons);
    }
    mSumPerPolygon.fill( 0, mNumberOfPolygons );
    mMeanProjectedArea.fill( 0.0f, mNumberOfPolygons );
    mSumPerViewpoint.fill( 0, mNumberOfViewpoints );
}

VisibilityChannelHistogram::VisibilityChannelHistogram(const VisibilityChannelHistogram *pVisibilityChannelHistogram):
    mValues(pVisibilityChannelHistogram->mValues),
    mNumberOfViewpoints(pVisibilityChannelHistogram->mNumberOfViewpoints), mNumberOfPolygons(pVisibilityChannelHistogram->mNumberOfPolygons),
    mSumPerViewpoint(pVisibilityChannelHistogram->mSumPerViewpoint), mSumPerPolygon(pVisibilityChannelHistogram->mSumPerPolygon),
    mMeanProjectedArea(pVisibilityChannelHistogram->mMeanProjectedArea),
    mTotalSum(pVisibilityChannelHistogram->mTotalSum)
{

}

int VisibilityChannelHistogram::GetNumberOfViewpoints() const
{
    return mNumberOfViewpoints;
}

int VisibilityChannelHistogram::GetNumberOfPolygons() const
{
    return mNumberOfPolygons;
}

unsigned int VisibilityChannelHistogram::GetSumPerViewpoint(int pViewpoint) const
{
    return mSumPerViewpoint.at(pViewpoint);
}

unsigned int VisibilityChannelHistogram::GetSumPerPolygon(int pPolygon) const
{
    return mSumPerPolygon.at(pPolygon);
}

QVector<float> VisibilityChannelHistogram::GetMeanProjectedArea() const
{
    return mMeanProjectedArea;
}

unsigned int VisibilityChannelHistogram::GetTotalSum() const
{
    return mTotalSum;
}

void VisibilityChannelHistogram::SetValues(int pViewpoint, const QVector< unsigned int > &pValues)
{
    mValues[pViewpoint] = pValues;
}

unsigned int VisibilityChannelHistogram::GetValue(int pViewpoint, int pPolygon) const
{
    return mValues.at(pViewpoint).at(pPolygon);
}

void VisibilityChannelHistogram::Compute()
{
    mSumPerPolygon.fill( 0, mNumberOfPolygons );
    mMeanProjectedArea.fill( 0.0f, mNumberOfPolygons );
    mSumPerViewpoint.fill( 0, mNumberOfViewpoints );
    mTotalSum = 0;

    for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
    {
        for( int currentViewpoint = 0; currentViewpoint < mNumberOfViewpoints; currentViewpoint++ )
        {
            unsigned int value = mValues.at(currentViewpoint).at(currentPolygon);
            mTotalSum += value;
            mSumPerPolygon[currentPolygon] += value;
            mSumPerViewpoint[currentViewpoint] += value;
        }
        mMeanProjectedArea[currentPolygon] = mSumPerPolygon.at(currentPolygon) / (float)mNumberOfPolygons;
    }
}
