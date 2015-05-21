/// \file NBestViews.h
/// \class NBestViews
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _N_BEST_VIEWS_H_
#define _N_BEST_VIEWS_H_

//Qt includes
#include <QVector>

//Dependency includes
#include "PolygonalI1.h"
#include "PolygonalI2.h"
#include "PolygonalI3.h"
#include "VisibilityChannelHistogram.h"
#include "ProjectedLocalMeasurePVO.h"

class NBestViews
{
public:
    NBestViews(const VisibilityChannelHistogram *pVisibilityChannelHistogram);
    ~NBestViews();

    void SetDependencyProjectedI1(ProjectedLocalMeasurePVO* pProjectedI1);
    void SetDependencyPolygonalI1(PolygonalI1* pPolygonalI1);
    void SetDependencyProjectedI2(ProjectedLocalMeasurePVO* pProjectedI2);
    void SetDependencyPolygonalI2(PolygonalI2* pPolygonalI2);
    void SetDependencyProjectedI3(ProjectedLocalMeasurePVO* pProjectedI3);
    void SetDependencyPolygonalI3(PolygonalI3* pPolygonalI3);

    /// Method to get the best n views
    /// \pre 1 <= pNumberOfViews <= mNumberOfViewpoints && 0 <= pDiscardingCriteria <= 1
    QVector< int > GetBestNViewsProjectedI1DiscardingPolygons(int pNumberOfViews, float pPercent, int pDiscardingCriteria) const;
    QVector< int > GetBestNViewsProjectedI2DiscardingPolygons(int pNumberOfViews, float pPercent, int pDiscardingCriteria) const;
    QVector< int > GetBestNViewsProjectedI3DiscardingPolygons(int pNumberOfViews, float pPercent, int pDiscardingCriteria) const;

private:
    const VisibilityChannelHistogram* mHistogram;
    int mNumberOfViewpoints;
    int mNumberOfPolygons;

    //TODO:Falta fer el set i el checking de que estigui set
    ProjectedLocalMeasurePVO* mProjectedI1;
    PolygonalI1* mPolygonalI1;
    ProjectedLocalMeasurePVO* mProjectedI2;
    PolygonalI2* mPolygonalI2;
    ProjectedLocalMeasurePVO* mProjectedI3;
    PolygonalI3* mPolygonalI3;

    QVector< unsigned int > mMaxAreaPolygon;
    float mSumMaxArea;
};

#endif
