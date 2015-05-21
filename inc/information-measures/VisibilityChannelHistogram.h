/// \file VisibilityChannelHistogram.h
/// \class VisibilityChannelHistogram
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _VISIBILITY_CHANNEL_HISTOGRAM_H_
#define _VISIBILITY_CHANNEL_HISTOGRAM_H_

//Qt includes
#include <QVector>

class VisibilityChannelHistogram
{
public:
    VisibilityChannelHistogram(int pNumberOfViewpoints, int pNumberOfPolygons);
    VisibilityChannelHistogram(const VisibilityChannelHistogram *pVisibilityChannelHistogram);
    int GetNumberOfViewpoints() const;
    int GetNumberOfPolygons() const;
    unsigned int GetSumPerViewpoint(int pViewpoint) const;
    unsigned int GetSumPerPolygon(int pPolygon) const;
    QVector<float> GetMeanProjectedArea() const;
    unsigned int GetTotalSum() const;
    void SetValues(int pViewpoint, const QVector< unsigned int > &pValues);
    unsigned int GetValue(int pViewpoint, int pPolygon) const;
    void Compute();
private:
    QVector< QVector< unsigned int > > mValues;
    int mNumberOfViewpoints;
    int mNumberOfPolygons;
    QVector< unsigned int > mSumPerViewpoint;
    QVector< unsigned int > mSumPerPolygon;
    QVector< float > mMeanProjectedArea;
    unsigned int mTotalSum;
};

#endif
