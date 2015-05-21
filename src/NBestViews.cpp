//Definition include
#include "NBestViews.h"

//Project includes
#include "Debug.h"
#include "Tools.h"

NBestViews::NBestViews(const VisibilityChannelHistogram *pVisibilityChannelHistogram):
    mHistogram(pVisibilityChannelHistogram), mNumberOfViewpoints(pVisibilityChannelHistogram->GetNumberOfViewpoints()),
    mNumberOfPolygons(pVisibilityChannelHistogram->GetNumberOfPolygons()),
    mProjectedI1(NULL), mPolygonalI1(NULL), mProjectedI2(NULL), mPolygonalI2(NULL), mProjectedI3(NULL), mPolygonalI3(NULL)
{
    mSumMaxArea = 0.0f;
    mMaxAreaPolygon.fill( 0, mNumberOfPolygons );
    for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
    {
        for( int currentViewpoint = 0; currentViewpoint < mNumberOfViewpoints; currentViewpoint++ )
        {
            unsigned int value = mHistogram->GetValue(currentViewpoint, currentPolygon);
            if( mMaxAreaPolygon.at(currentPolygon) < value )
            {
                mMaxAreaPolygon[currentPolygon] = value;
            }
        }
        mSumMaxArea += mMaxAreaPolygon.at(currentPolygon);
    }
}

NBestViews::~NBestViews()
{

}

void NBestViews::SetDependencyProjectedI1(ProjectedLocalMeasurePVO* pProjectedI1)
{
    mProjectedI1 = pProjectedI1;
}

void NBestViews::SetDependencyPolygonalI1(PolygonalI1 *pPolygonalI1)
{
    mPolygonalI1 = pPolygonalI1;
}

void NBestViews::SetDependencyProjectedI2(ProjectedLocalMeasurePVO* pProjectedI2)
{
    mProjectedI2 = pProjectedI2;
}

void NBestViews::SetDependencyPolygonalI2(PolygonalI2* pPolygonalI2)
{
    mPolygonalI2 = pPolygonalI2;
}

void NBestViews::SetDependencyProjectedI3(ProjectedLocalMeasurePVO* pProjectedI3)
{
    mProjectedI3 = pProjectedI3;
}

void NBestViews::SetDependencyPolygonalI3(PolygonalI3 *pPolygonalI3)
{
    mPolygonalI3 = pPolygonalI3;
}

QVector< int > NBestViews::GetBestNViewsProjectedI1DiscardingPolygons(int pNumberOfViews, float pPercent, int pDiscardingCriteria ) const
{
    QVector< bool > selectedViews( mNumberOfViewpoints, false );
    QVector< bool > selectedPolygons( mNumberOfPolygons, false );
    QVector< int > bestViews;

    QVector<float> scaledPolygonalMeasure;
    if(mProjectedI1->IsLocalMeasureScaled())
    {
        scaledPolygonalMeasure = Tools::ScaleValues(mPolygonalI1->GetValues(), mProjectedI1->GetScaleLowerBound(), mProjectedI1->GetScaleUpperBound() );
    }
    else
    {
        scaledPolygonalMeasure = mPolygonalI1->GetValues();
    }

    int viewpointToAdd = mProjectedI1->GetNth(mNumberOfViewpoints - 1);
    bool viewpointFound = true;

    int i = 1;
    float lastVQ = mProjectedI1->GetValue(viewpointToAdd);
    unsigned int covered = 0;
    while( i < pNumberOfViews && ( covered / (float)mSumMaxArea ) < pPercent )
    {
        bestViews.push_back(viewpointToAdd);
        selectedViews[viewpointToAdd] = true;
        for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
        {
            int value = mHistogram->GetValue(viewpointToAdd, currentPolygon);
            if(!selectedPolygons.at(currentPolygon) && value != 0 )
            {
                bool discard = (pDiscardingCriteria == 0) || (value > mMaxAreaPolygon.at(currentPolygon) * 0.50f);
                selectedPolygons[currentPolygon] = discard;
                if(discard)
                {
                    covered += mMaxAreaPolygon.at(currentPolygon);
                }
            }
        }
        Debug::Log(QString("%1 views selected, %2 covered, last VQ %3").arg(bestViews.size()).arg(100.0f*(covered / (float)mSumMaxArea)).arg(lastVQ));

        float max = -FLT_MAX;
        viewpointFound = false;
        for( int currentViewpoint = 0; currentViewpoint < mNumberOfViewpoints; currentViewpoint++ )
        {
            if( !selectedViews.at(currentViewpoint) )
            {
                float currentProjectedI1 = 0.0f;
                bool seePolygons = false;
                for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
                {
                    unsigned int a_z = mHistogram->GetValue(currentViewpoint, currentPolygon);

                    if( a_z != 0 && !selectedPolygons.at(currentPolygon) )
                    {
                        seePolygons = true;

                        float aux = a_z / (float)mHistogram->GetSumPerPolygon(currentPolygon);

                        currentProjectedI1 += aux * scaledPolygonalMeasure.at(currentPolygon);
                    }
                }
                if( currentProjectedI1 > max && seePolygons )
                {
                    max = currentProjectedI1;
                    lastVQ = max;
                    viewpointToAdd = currentViewpoint;
                    viewpointFound = true;
                }
            }
        }
        if(!viewpointFound)
        {
            break;
        }
        i++;
    }
    if(viewpointFound)
    {
        bestViews.push_back(viewpointToAdd);
        selectedViews[viewpointToAdd] = true;
        for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
        {
            int value = mHistogram->GetValue(viewpointToAdd, currentPolygon);
            if(!selectedPolygons.at(currentPolygon) && value != 0 )
            {
                bool discard = (pDiscardingCriteria == 0) || (value > mMaxAreaPolygon.at(currentPolygon) * 0.50f);
                selectedPolygons[currentPolygon] = discard;
                if(discard)
                {
                    covered += mMaxAreaPolygon.at(currentPolygon);
                }
            }
        }
        Debug::Log(QString("%1 views selected, %2 covered, last VQ %3").arg(bestViews.size()).arg(100.0f*(covered / (float)mSumMaxArea)).arg(lastVQ));
    }

    return bestViews;
}

QVector< int > NBestViews::GetBestNViewsProjectedI2DiscardingPolygons(int pNumberOfViews, float pPercent, int pDiscardingCriteria) const
{
    QVector< bool > selectedViews( mNumberOfViewpoints, false );
    QVector< bool > selectedPolygons( mNumberOfPolygons, false );
    QVector< int > bestViews;

    QVector<float> scaledPolygonalMeasure;
    if(mProjectedI2->IsLocalMeasureScaled())
    {
        scaledPolygonalMeasure = Tools::ScaleValues(mPolygonalI2->GetValues(), mProjectedI2->GetScaleLowerBound(), mProjectedI2->GetScaleUpperBound() );
    }
    else
    {
        scaledPolygonalMeasure = mPolygonalI2->GetValues();
    }

    int viewpointToAdd = mProjectedI2->GetNth(mNumberOfViewpoints - 1);
    bool viewpointFound = true;

    int i = 1;
    float lastVQ = mProjectedI2->GetValue(viewpointToAdd);
    unsigned int covered = 0;
    while( i < pNumberOfViews && ( covered / (float)mSumMaxArea ) < pPercent )
    {
        bestViews.push_back(viewpointToAdd);
        selectedViews[viewpointToAdd] = true;
        for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
        {
            int value = mHistogram->GetValue(viewpointToAdd, currentPolygon);
            if(!selectedPolygons.at(currentPolygon) && value != 0 )
            {
                bool discard = (pDiscardingCriteria == 0) || (value > mMaxAreaPolygon.at(currentPolygon) * 0.50f);
                selectedPolygons[currentPolygon] = discard;
                if(discard)
                {
                    covered += mMaxAreaPolygon.at(currentPolygon);
                }
            }
        }
        Debug::Log(QString("%1 views selected, %2 covered, last VQ %3").arg(bestViews.size()).arg(100.0f*(covered / (float)mSumMaxArea)).arg(lastVQ));

        float max = -FLT_MAX;
        viewpointFound = false;
        for( int currentViewpoint = 0; currentViewpoint < mNumberOfViewpoints; currentViewpoint++ )
        {
            if( !selectedViews.at(currentViewpoint) )
            {
                float currentProjectedI2 = 0.0f;
                bool seePolygons = false;
                for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
                {
                    unsigned int a_z = mHistogram->GetValue(currentViewpoint, currentPolygon);

                    if( a_z != 0 && !selectedPolygons.at(currentPolygon) )
                    {
                        seePolygons = true;

                        float aux = a_z / (float)mHistogram->GetSumPerPolygon(currentPolygon);

                        currentProjectedI2 += aux * scaledPolygonalMeasure.at(currentPolygon);
                    }
                }
                if( currentProjectedI2 > max && seePolygons )
                {
                    max = currentProjectedI2;
                    lastVQ = max;
                    viewpointToAdd = currentViewpoint;
                    viewpointFound = true;
                }
            }
        }
        if(!viewpointFound)
        {
            break;
        }
        i++;
    }
    if(viewpointFound)
    {
        bestViews.push_back(viewpointToAdd);
        selectedViews[viewpointToAdd] = true;
        for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
        {
            int value = mHistogram->GetValue(viewpointToAdd, currentPolygon);
            if(!selectedPolygons.at(currentPolygon) && value != 0 )
            {
                bool discard = (pDiscardingCriteria == 0) || (value > mMaxAreaPolygon.at(currentPolygon) * 0.50f);
                selectedPolygons[currentPolygon] = discard;
                if(discard)
                {
                    covered += mMaxAreaPolygon.at(currentPolygon);
                }
            }
        }
        Debug::Log(QString("%1 views selected, %2 covered, last VQ %3").arg(bestViews.size()).arg(100.0f*(covered / (float)mSumMaxArea)).arg(lastVQ));
    }

    return bestViews;
}

QVector< int > NBestViews::GetBestNViewsProjectedI3DiscardingPolygons(int pNumberOfViews, float pPercent, int pDiscardingCriteria ) const
{
    QVector< bool > selectedViews( mNumberOfViewpoints, false );
    QVector< bool > selectedPolygons( mNumberOfPolygons, false );
    QVector< int > bestViews;

    QVector<float> scaledPolygonalMeasure;
    if(mProjectedI3->IsLocalMeasureScaled())
    {
        scaledPolygonalMeasure = Tools::ScaleValues(mPolygonalI3->GetValues(), mProjectedI3->GetScaleLowerBound(), mProjectedI3->GetScaleUpperBound() );
    }
    else
    {
        scaledPolygonalMeasure = mPolygonalI3->GetValues();
    }

    int viewpointToAdd = mProjectedI3->GetNth(mNumberOfViewpoints - 1);
    bool viewpointFound = true;

    int i = 1;
    float lastVQ = mProjectedI3->GetValue(viewpointToAdd);
    unsigned int covered = 0;
    while( i < pNumberOfViews && ( covered / (float)mSumMaxArea ) < pPercent )
    {
        bestViews.push_back(viewpointToAdd);
        selectedViews[viewpointToAdd] = true;
        for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
        {
            int value = mHistogram->GetValue(viewpointToAdd, currentPolygon);
            if(!selectedPolygons.at(currentPolygon) && value != 0 )
            {
                bool discard = (pDiscardingCriteria == 0) || (value > mMaxAreaPolygon.at(currentPolygon) * 0.50f);
                selectedPolygons[currentPolygon] = discard;
                if(discard)
                {
                    covered += mMaxAreaPolygon.at(currentPolygon);
                }
            }
        }
        Debug::Log(QString("%1 views selected, %2 covered, last VQ %3").arg(bestViews.size()).arg(100.0f*(covered / (float)mSumMaxArea)).arg(lastVQ));

        float max = -FLT_MAX;
        viewpointFound = false;
        for( int currentViewpoint = 0; currentViewpoint < mNumberOfViewpoints; currentViewpoint++ )
        {
            if( !selectedViews.at(currentViewpoint) )
            {
                float currentProjectedI3 = 0.0f;
                bool seePolygons = false;
                for( int k = 0; k < mNumberOfPolygons; k++ )
                {
                    unsigned int a_z = mHistogram->GetValue(currentViewpoint, k);

                    if( a_z != 0 && !selectedPolygons.at(k) )
                    {
                        seePolygons = true;

                        float aux = a_z / (float)mHistogram->GetSumPerPolygon(k);

                        currentProjectedI3 += aux * scaledPolygonalMeasure.at(k);
                    }
                }
                if( currentProjectedI3 > max && seePolygons )
                {
                    max = currentProjectedI3;
                    lastVQ = max;
                    viewpointToAdd = currentViewpoint;
                    viewpointFound = true;
                }
            }
        }
        if(!viewpointFound)
        {
            break;
        }
        i++;
    }
    if(viewpointFound)
    {
        bestViews.push_back(viewpointToAdd);
        selectedViews[viewpointToAdd] = true;
        for( int currentPolygon = 0; currentPolygon < mNumberOfPolygons; currentPolygon++ )
        {
            int value = mHistogram->GetValue(viewpointToAdd, currentPolygon);
            if(!selectedPolygons.at(currentPolygon) && value != 0 )
            {
                bool discard = (pDiscardingCriteria == 0) || (value > mMaxAreaPolygon.at(currentPolygon) * 0.50f);
                selectedPolygons[currentPolygon] = discard;
                if(discard)
                {
                    covered += mMaxAreaPolygon.at(currentPolygon);
                }
            }
        }
        Debug::Log(QString("%1 views selected, %2 covered, last VQ %3").arg(bestViews.size()).arg(100.0f*(covered / (float)mSumMaxArea)).arg(lastVQ));

    }

    return bestViews;
}
