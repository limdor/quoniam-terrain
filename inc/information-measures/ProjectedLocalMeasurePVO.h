/// \file ProjectedLocalMeasurePVO.h
/// \class ProjectedLocalMeasurePVO
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _PROJECTED_LOCAL_MEASURE_PVO_H_
#define _PROJECTED_LOCAL_MEASURE_PVO_H_

#include "Measure.h"

class ProjectedLocalMeasurePVO : public Measure
{
public:
    ProjectedLocalMeasurePVO(const QString& pName);
    ~ProjectedLocalMeasurePVO();

    void Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram);
    void AddDpendencyLocalMeasure(Measure* pLocalMeasure);
    void SetScaleDependencyLocalMeasure(float pLowerBound, float pUpperBound);
    bool IsLocalMeasureScaled() const;
    float GetScaleLowerBound() const;
    float GetScaleUpperBound() const;
private:
    Measure* mLocalMeasure;
    bool mLocalMeasureCreated;
    bool mScaleLocalMeasure;
    float mScaleLocalMeasureLowerBound;
    float mScaleLocalMeasureUpperBound;
};

#endif
