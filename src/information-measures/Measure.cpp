#include "Measure.h"

Measure::Measure(const QString &pName)
{
    mName = pName;
    mComputed = false;
}

Measure::~Measure()
{

}

QString Measure::GetName() const
{
    return mName;
}

QVector<float> Measure::GetValues() const
{
    return mValues;
}

float Measure::GetValue(int pIndex) const
{
    return mValues.at(pIndex);
}

QVector<float> Measure::GetScaledValues() const
{
    return mScaledValues;
}

int Measure::GetNth(int pNth) const
{
    return mSort.at(pNth);
}

int Measure::GetPosition(int pViewpoint) const
{
    return mPositions.at(pViewpoint);
}

bool Measure::Computed() const
{
    return mComputed;
}

void Measure::SetComputed(bool pComputed)
{
    mComputed = pComputed;
}
