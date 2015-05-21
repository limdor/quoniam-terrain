//Definition include
#include "ViewpointsMesh.h"

//Dependency includes
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

//Qt includes
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>

//Project includes
#include "PerspectiveCamera.h"
#include "OrthographicCamera.h"
#include "Debug.h"
#include "SpherePointCloud.h"

ViewpointsMesh::ViewpointsMesh(const QVector< Camera * >& pCameras)
{
    mMesh = new Geometry( "List of viewpoints", Geometry::Points );

    int numberOfViewpoints = pCameras.size();
    mVertices.resize(numberOfViewpoints);
    mCameras.resize(numberOfViewpoints);
    for( int i = 0; i < numberOfViewpoints; i++ )
    {
        mVertices[i] = pCameras.at(i)->GetPosition();
        mCameras[i] = pCameras.at(i);
    }
    mMesh->SetVerticesData( numberOfViewpoints, mVertices.data() );
    mMesh->ComputeBoundingVolumes();

    mNeighbours.resize(numberOfViewpoints);
}

ViewpointsMesh::ViewpointsMesh(const glm::vec3& pCenter, float pRadius, float pAngle, float pAspectRatio, int pSubdivision)
{
    mMesh = new Geometry( "Sphere of viewpoints", Geometry::Points );

    //Creació de l'esfera de punts de vista
    SpherePointCloud sphere;
    sphere.SetToQuasiUniform(pSubdivision);

    mVertices = sphere.GetVertices();
    int numberOfViewpoints = mVertices.size();
    mCameras.resize(numberOfViewpoints);
    for( int i = 0; i < numberOfViewpoints; i++ )
    {
        mVertices[i] = mVertices.at(i) * pRadius + pCenter;
        mCameras[i] = new PerspectiveCamera( 0.05f * pRadius, pRadius * 3.0f, pCenter, SpherePointCloud::Up(mVertices.at(i)), mVertices.at(i), pAngle, pAspectRatio );
        mCameras[i]->mName = QString("%1").arg(i);
    }
    mMesh->SetVerticesData( numberOfViewpoints, mVertices.data() );
    mMesh->ComputeBoundingVolumes();

    SetFaces( sphere.GetFaces() );

    mNeighbours.resize(numberOfViewpoints);
    for( int i = 0; i < numberOfViewpoints; i++ )
    {
        QVector<int> neigh = sphere.GetNeighbours(i);
        int numberOfNeighbours = neigh.size();
        mNeighbours[i].resize(numberOfNeighbours);
        for( int j = 0; j < numberOfNeighbours; j++ )
        {
            mNeighbours[i][j] = neigh.at(j);
        }
    }
}

ViewpointsMesh::ViewpointsMesh(const glm::vec3& pCenter, float pRadius, const QString& pFileName)
{
    QFile file(pFileName);

    if( file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QVector< glm::vec3 > centerPoints;
        QVector< glm::vec3 > upperLeftPoints;
        QVector< glm::vec3 > upperRightPoints;
        QVector< glm::vec3 > lowerLeftPoints;
        QVector< glm::vec3 > lowerRightPoints;
        QVector< QString > imageNames;

        mMesh = new Geometry( "Viewpoints from file", Geometry::Points );

        //Fitxer xml
        if( pFileName.endsWith( QString("xml"), Qt::CaseInsensitive ) )
        {
            QXmlStreamReader * xmlReader = new QXmlStreamReader(&file);
            if( xmlReader->readNextStartElement() )
            {
                if( xmlReader->name() == "viewpoints" )
                {
                    while( xmlReader->readNextStartElement() )
                    {
                        if( xmlReader->name() == "viewpoint" )
                        {
                            imageNames.push_back( xmlReader->attributes().value("image").toString() );

                            while( xmlReader->readNextStartElement() )
                            {
                                glm::vec3 point;

                                point.x = xmlReader->attributes().value("x").toString().toFloat();
                                point.y = xmlReader->attributes().value("y").toString().toFloat();
                                point.z = xmlReader->attributes().value("z").toString().toFloat();

                                if(xmlReader->name() == "center")
                                {
                                    centerPoints.push_back(point);
                                }
                                else if(xmlReader->name() == "ul")
                                {
                                    upperLeftPoints.push_back(point);
                                }
                                else if(xmlReader->name() == "ur")
                                {
                                    upperRightPoints.push_back(point);
                                }
                                else if(xmlReader->name() == "lr")
                                {
                                    lowerRightPoints.push_back(point);
                                }
                                else if(xmlReader->name() == "ll")
                                {
                                    lowerLeftPoints.push_back(point);
                                }
                            }
                        }
                    }

                }
            }
            if (xmlReader->hasError())
            {
                Debug::Error(QString("[Line: %1, Column:%2] Error in LoadViewpointsFromFile - GeneralError file %3: %4, error: %5").arg(xmlReader->lineNumber()).arg(xmlReader->columnNumber()).arg(pFileName).arg(xmlReader->errorString()).arg(xmlReader->error()));
            }

            delete xmlReader;
        }
        else if( pFileName.endsWith( QString("txt"), Qt::CaseInsensitive ) )
        {
            QTextStream * textReader = new QTextStream(&file);

            //Reading the header
            textReader->readLine();
            while( !textReader->atEnd() )
            {
                QString line = textReader->readLine();
                QStringList list = line.split(QRegExp("\\s*,\\s*"));
                if( list.size() == 16 )
                {
                    imageNames.push_back( list.at(0) );

                    centerPoints.push_back( glm::vec3( list.at(1).toFloat(), list.at(2).toFloat(), list.at(3).toFloat() ) );
                    upperLeftPoints.push_back( glm::vec3( list.at(4).toFloat(), list.at(5).toFloat(), list.at(6).toFloat() ) );
                    upperRightPoints.push_back( glm::vec3( list.at(7).toFloat(), list.at(8).toFloat(), list.at(9).toFloat() ) );
                    lowerRightPoints.push_back( glm::vec3( list.at(10).toFloat(), list.at(11).toFloat(), list.at(12).toFloat() ) );
                    lowerLeftPoints.push_back( glm::vec3( list.at(13).toFloat(), list.at(14).toFloat(), list.at(15).toFloat() ) );
                }
                else
                {
                    Debug::Error( "File format unexpected!" );
                }

            }
        }
        else
        {
            Debug::Warning( "Unknown file format!" );
        }
        file.close();
        int numberOfViewpoints = imageNames.size();
        mCameras.resize(numberOfViewpoints);
        mVertices.resize(numberOfViewpoints);
        for( int i = 0; i < numberOfViewpoints; i++ )
        {
            glm::vec3 leftPoint = ( ( upperLeftPoints.at(i) - lowerLeftPoints.at(i) ) / 2.0f ) + lowerLeftPoints.at(i);
            glm::vec3 rightPoint = ( ( upperRightPoints.at(i) - lowerRightPoints.at(i) ) / 2.0f ) + lowerRightPoints.at(i);
            glm::vec3 lookAtPoint = ( ( lowerRightPoints.at(i) - lowerLeftPoints.at(i) ) / 2.0f ) + leftPoint;
            float aspectRatio = glm::length( lowerRightPoints.at(i) - lowerLeftPoints.at(i) ) / glm::length( upperRightPoints.at(i) - lowerRightPoints.at(i) );
            glm::vec3 v1 = rightPoint - centerPoints.at(i);
            glm::vec3 v2 = leftPoint - centerPoints.at(i);
            float angle = glm::degrees( glm::acos( glm::dot( v1, v2 ) / ( glm::length( v1 ) * glm::length( v2 ) ) ) );

            Debug::Log( QString("Image number: %1, Aspect ratio: %2, Angle: %3, Center: (%4, %5, %6)").arg(imageNames.at(i)).arg(aspectRatio).arg(angle).arg(centerPoints.at(i).x).arg(centerPoints.at(i).y).arg(centerPoints.at(i).z) );
            mCameras[i] = new PerspectiveCamera( 0.05f * pRadius, pRadius * 2.0f + glm::length( pCenter - centerPoints.at(i) ), lookAtPoint, glm::cross( v1, v2 ), centerPoints.at(i), angle / aspectRatio, aspectRatio );
            mCameras.at(i)->mName = imageNames.at(i);
            mVertices[i] = centerPoints.at(i);
        }
        mMesh->SetVerticesData( numberOfViewpoints, mVertices.data() );
        mMesh->ComputeBoundingVolumes();

        mNeighbours.resize(numberOfViewpoints);
    }
    else
    {
        if(!file.exists())
        {
            Debug::Error(QString("File with viewpoints not found: %1").arg(pFileName));
        }
        else
        {
            Debug::Error(QString("Impossible to open file with viewpoints: %1").arg(pFileName));
        }
        mMesh = NULL;
    }
}

ViewpointsMesh::ViewpointsMesh(const ViewpointsMesh& pViewpointsMesh)
{
    if(pViewpointsMesh.mMesh != NULL)
    {
        mMesh = new Geometry(*pViewpointsMesh.mMesh);
    }
    else
    {
        mMesh = NULL;
    }
    mVertices = pViewpointsMesh.mVertices;
    mNeighbours = pViewpointsMesh.mNeighbours;
    int numberOfViewpoints = pViewpointsMesh.mCameras.size();
    mCameras.resize(numberOfViewpoints);
    for( int i = 0; i < numberOfViewpoints; i++ )
    {
        Camera* camera = pViewpointsMesh.mCameras.at(i);
        mCameras[i] = camera->Clone();
    }
    mFaces = pViewpointsMesh.mFaces;
}

ViewpointsMesh::~ViewpointsMesh()
{
    for( int i = 0; i < mCameras.size(); i++ )
    {
        delete mCameras[i];
    }
    delete mMesh;
}

void ViewpointsMesh::SetCameras( const QVector< Camera * >& pCameras )
{
    mCameras.resize( pCameras.size() );
    mVertices.resize( pCameras.size() );
    for( int i = 0; i < pCameras.size(); i++ )
    {
        mCameras[i] = pCameras.at(i);
        mVertices[i] = pCameras.at(i)->GetPosition();
    }
    mMesh->SetVerticesData( mVertices.size(), mVertices.data() );
    mMesh->ComputeBoundingVolumes();
}

void ViewpointsMesh::SetNeighbours( const QVector< QVector< int > >& pNeighbours )
{
    mNeighbours.resize( pNeighbours.size() );
    for( int i = 0; i < pNeighbours.size(); i++ )
    {
        mNeighbours[i].resize( pNeighbours.at(i).size() );
        for( int j = 0; j < pNeighbours.at(i).size(); j++ )
        {
            mNeighbours[i][j] = pNeighbours.at(i).at(j);
        }
    }
}

void ViewpointsMesh::SetFaces( const QVector< unsigned int >& pFaces )
{
    mFaces.resize( pFaces.size() );
    for( int i = 0; i < pFaces.size(); i++ )
    {
        mFaces[i] = pFaces.at(i);
    }
    mMesh->SetIndexsData( mFaces.size(), mFaces.data() );
    mMesh->SetTopology( Geometry::Triangles );
}

Camera* ViewpointsMesh::GetViewpoint( int pIndex ) const
{
    return mCameras.at(pIndex);
}

int ViewpointsMesh::GetNearestViewpoint( const glm::vec3& pPoint ) const
{
    int nearest;

    float minDistance = FLT_MAX;
    for( int i = 0; i < mVertices.size(); i++ )
    {
        float distance = glm::distance( pPoint, mVertices.at(i) );
        if( distance < minDistance )
        {
            minDistance = distance;
            nearest = i;
        }
    }

    return nearest;
}

int ViewpointsMesh::GetNumberOfViewpoints() const
{
    return mCameras.size();
}

QVector< QVector< int > > ViewpointsMesh::GetNeighbours() const
{
    return mNeighbours;
}

QVector< int > ViewpointsMesh::GetNeighbours( int pViewpoint ) const
{
    return mNeighbours.at(pViewpoint);
}

int ViewpointsMesh::GetNeighbour( int pViewpoint, int pNeighbourIndex ) const
{
    return mNeighbours.at(pViewpoint).at(pNeighbourIndex);
}

Geometry* ViewpointsMesh::GetMesh() const
{
    return mMesh;
}

QVector< glm::vec3 > ViewpointsMesh::GetVertices() const
{
    return mVertices;
}

QVector< unsigned int > ViewpointsMesh::GetFaces() const
{
    return mFaces;
}
