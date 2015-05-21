/// \file Scene.h
/// \class Scene
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _SCENE_H_
#define _SCENE_H_

//Dependency includes
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

//Project includes
#include "BoundingSphere.h"
#include "Geometry.h"

/// Class representing a scene made of different meshes
class Scene
{
public:
    /// Constructor
    Scene(const QString& pName);
    Scene(const QString& pName, const QVector<Geometry *>& pMeshes);
    /// Copy constructor
    Scene(const Scene& pScene);
    /// Destructor
    ~Scene();

    /// Add a mesh into the scene
    void Add(Geometry* pMesh);
    /// Add a entire scene into the scene
    void Add(Scene* pScene);
    /// Get the mesh at the position \param pIndex
    Geometry* GetMesh(int pIndex) const;
    /// Get the name of the scene
    QString GetName() const;
    /// Get the bounding sphere
    BoundingSphere* GetBoundingSphere() const;
    /// Get the number of polygons
    int GetNumberOfPolygons() const;
    /// Get the number of vertices
    int GetNumberOfVertices() const;
    /// Get the number of meshes
    int GetNumberOfMeshes() const;
    /// Show the information of the scene
    void ShowInformation() const;
    /// Get the polygon offset of the given mesh
    int GetPolygonOffset(Geometry* pMesh) const;
    /// Get the list of vertices of the given polygon
    QVector< glm::vec3 > GetVerticesOfPolygon( int pPolygon ) const;
    /// Get the area of the polygons serialized
    QVector< float > GetSerializedPolygonAreas() const;
    /// Normalize the scene to center (0, 0, 0) and radius 1
    void Normalize();
    void Transform(const glm::mat4 &pTransform);

private:
    /// Name of the scene
    QString mName;
    /// Meshes used by the scene
    QVector<Geometry *> mMeshes;
    /// Bounding sphere
    BoundingSphere* mBoundingSphere;
    /// Number of meshes
    int mNumberOfMeshes;
    /// Number of polygons
    int mNumberOfPolygons;
    /// Number of vertices
    int mNumberOfVertices;
};

#endif
