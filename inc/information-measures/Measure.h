/// \file Measure.h
/// \class Measure
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _MEASURE_H_
#define _MEASURE_H_

#include <QString>
#include "VisibilityChannelHistogram.h"

class Measure
{
public:
    Measure(const QString& pName);
    ~Measure();

    QString GetName() const;

    QVector<float> GetValues() const;
    float GetValue(int pIndex) const;
    QVector<float> GetScaledValues() const;
    int GetNth(int pNth) const;
    int GetPosition(int pViewpoint) const;

    bool Computed() const;
    void SetComputed(bool pComputed);
    virtual void Compute(const VisibilityChannelHistogram *pVisibilityChannelHistogram) = 0;
protected:
    QString mName;
    bool mComputed;
    QVector<float> mValues;
    QVector<float> mScaledValues;
    QVector<int> mSort;
    QVector<int> mPositions;
};

#endif
