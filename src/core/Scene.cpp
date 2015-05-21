//Definition include
#include "Scene.h"

//System includes
#include <algorithm>

//Qt includes
#include <QApplication>
#include <QProgressDialog>
#include <QTime>

//Project includes
#include "Debug.h"
#include "MainWindow.h"
#include "Tools.h"

Scene::Scene(const QString& pName)
{
    mName = pName;
    mNumberOfMeshes = 0;
    mNumberOfPolygons = 0;
    mNumberOfVertices = 0;
    mBoundingSphere = new BoundingSphere();
}

Scene::Scene(const QString& pName, const QVector<Geometry *> &pMeshes)
{
    mName = pName;
    mBoundingSphere = NULL;
    mNumberOfMeshes = pMeshes.size();
    mNumberOfPolygons = 0;
    mNumberOfVertices = 0;
    for( int i = 0; i < mNumberOfMeshes; i++ )
    {
        //Afegim el mesh a la llista de meshes
        mMeshes.push_back(pMeshes.at(i));

        mNumberOfPolygons += pMeshes.at(i)->GetNumFaces();
        mNumberOfVertices += pMeshes.at(i)->GetNumVertices();

        //Recalculem el bounding volume
        BoundingSphere* newBoundingSphere = BoundingSphere::Merge(mBoundingSphere, pMeshes.at(i)->GetBoundingSphere());
        if( mBoundingSphere != NULL )
        {
            delete mBoundingSphere;
        }
        mBoundingSphere = newBoundingSphere;
    }
    if( mBoundingSphere == NULL )
    {
        mBoundingSphere = new BoundingSphere();
    }
}

Scene::Scene(const Scene& pScene)
{
    mNumberOfMeshes = pScene.mNumberOfMeshes;
    mMeshes.resize(mNumberOfMeshes);
    for( int i = 0; i < mNumberOfMeshes; i++ )
    {
        mMeshes[i] = new Geometry(*pScene.mMeshes.at(i));
    }
    mName = pScene.mName;
    mBoundingSphere = new BoundingSphere(*pScene.mBoundingSphere);
    mNumberOfPolygons = pScene.mNumberOfPolygons;
    mNumberOfVertices = pScene.mNumberOfVertices;
}

Scene::~Scene()
{
    for( int i = 0; i < mMeshes.size(); i++ )
    {
        delete mMeshes.at(i);
    }
    delete mBoundingSphere;
}

void Scene::Add(Geometry *pMesh)
{
    //Afegim el mesh a la llista de meshes
    mMeshes.push_back(pMesh);

    mNumberOfMeshes++;
    mNumberOfPolygons += pMesh->GetNumFaces();
    mNumberOfVertices += pMesh->GetNumVertices();

    //Recalculem el bounding volume
    BoundingSphere* newBoundingSphere = BoundingSphere::Merge(mBoundingSphere, pMesh->GetBoundingSphere());
    if( mBoundingSphere != NULL )
    {
        delete mBoundingSphere;
    }
    mBoundingSphere = newBoundingSphere;
}

void Scene::Add(Scene* pScene)
{
    for( int i = 0; i < pScene->mNumberOfMeshes; i++ )
    {
        Add(pScene->GetMesh(i));
    }
}

Geometry* Scene::GetMesh(int pIndex) const
{
    return mMeshes.at(pIndex);
}

QString Scene::GetName() const
{
    return mName;
}

BoundingSphere* Scene::GetBoundingSphere() const
{
    return mBoundingSphere;
}

int Scene::GetNumberOfPolygons() const
{
    return mNumberOfPolygons;
}

int Scene::GetNumberOfVertices() const
{
    return mNumberOfVertices;
}

int Scene::GetNumberOfMeshes() const
{
    return mNumberOfMeshes;
}

void Scene::ShowInformation() const
{
    Debug::Log( QString("Scene name: %1").arg(mName) );
    Debug::Log( QString("Number of vertices: %1").arg(mNumberOfVertices) );
    Debug::Log( QString("Number of polygons: %1").arg(mNumberOfPolygons) );
}

int Scene::GetPolygonOffset(Geometry *pMesh) const
{
    bool notFound = true;
    int offset = 0;
    int i = 0;
    while(notFound && i < mMeshes.size())
    {
        if(mMeshes.at(i) != pMesh)
        {
            offset += mMeshes.at(i)->GetNumFaces();
        }
        else
        {
            notFound = false;
        }
        i++;
    }
    if(notFound)
    {
        Debug::Error("No s'ha trobat el mesh");
    }
    return offset;
}

QVector< glm::vec3 > Scene::GetVerticesOfPolygon( int pPolygon ) const
{
    QVector< glm::vec3 > vertices;

    if( mMeshes.size() == 0 )
    {
        Debug::Warning( "Scene without meshes" );
    }
    else
    {
        int currentMesh = 0;
        int numPolygons = mMeshes.at(currentMesh)->GetNumFaces();
        while( pPolygon >= numPolygons )
        {
            currentMesh++;
            numPolygons += mMeshes.at(currentMesh)->GetNumFaces();
        }
        int meshPolygon = pPolygon - ( numPolygons - mMeshes.at(currentMesh)->GetNumFaces() );

        if( mMeshes.at(currentMesh)->GetTopology() == Geometry::Triangles )
        {
            int i;

            for( i = 0; i < 3; i++ )
            {
                vertices.push_back( mMeshes.at(currentMesh)->GetVertexByIndexPosition( meshPolygon * 3 + i ) );
            }
        }
        else
        {
            Debug::Error( "Scene::GetVerticesOfPolygon not implementet for different topologies than Triangles" );
        }
    }

    return vertices;
}

QVector< float > Scene::GetSerializedPolygonAreas() const
{
    QVector< float > areas;

    for( int i = 0; i < mMeshes.size(); i++ )
    {
        QVector< float > meshAreas = mMeshes.at(i)->GetAreasOfPolygons();
        areas += meshAreas;
    }

    return areas;
}

void Scene::Normalize()
{
    glm::vec3 center = mBoundingSphere->GetCenter();
    float radius = mBoundingSphere->GetRadius();
    for( int i = 0; i < mMeshes.size(); i++ )
    {
        mMeshes.at(i)->Normalize(center, radius);
    }
    mBoundingSphere->SetCenter(glm::vec3(0.0f, 0.0f, 0.0f));
    mBoundingSphere->SetRadius(1.0f);
}

void Scene::Transform(const glm::mat4 &pTransform)
{
    for( int i = 0; i < mMeshes.size(); i++ )
    {
        mMeshes.at(i)->Transform(pTransform);
    }
}
