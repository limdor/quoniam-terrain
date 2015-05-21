/// \file SpherePointCloud.h
/// \class SpherePointCloud
/// \author Francisco González
/// \author Marc Ruiz
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _SPHERE_POINT_CLOUD_H_
#define _SPHERE_POINT_CLOUD_H_

//Qt includes
#include <QVector>

//Dependency includes
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

//Project includes
#include "Geometry.h"

/**
 * Sphere of points uniformly or quasi-uniformly distributed
 */
class SpherePointCloud {

public:
    /// Retorna un vector cap amunt per al punt de vista passat.
    static glm::vec3 Up(const glm::vec3 &pViewpoint);

    /// Constructor
    SpherePointCloud();
    /// Copy constructor
    SpherePointCloud( const SpherePointCloud& pSpherePointCloud );
    /// Destructor
    ~SpherePointCloud();

    /// Generate 10*4^depth+2 points quasi-uniformly distributed using the recursive
    /// subdivision of faces, starting from an icosahedron
    void SetToQuasiUniform(unsigned char pDepth);

    /// Return the faces of the triangles
    QVector<unsigned int> GetFaces() const;
    /// Return the points in cartesian coordinates
    QVector<glm::vec3> GetVertices() const;
    /// Return the points in spherical coordinates (radius, theta, phi)
    QVector<glm::vec2> GetVerticesInSphericalCoordinates() const;
    /// Return the points in grid coordinates (uniformly distributed between 0 and 1 in two dimensions)
    QVector<glm::vec2> GetVerticesInUVCoordinates() const;

    /// Return the vertex \a pI
    glm::vec3 GetVertex(unsigned int pI) const;
    /// Return the neighbours of every point
    QVector< QVector<int> > GetNeighbours() const;
    /// Return the neighbours of the point \a pI
    QVector<int> GetNeighbours(unsigned int pI) const;

private:
    /// Compute the neighbours of the quasi uniform distribution
    void ComputeQuasiUniformNeighbours();
    /// Create the vertexs, faces and normals of an icosahedron (level 0)
    void CreateIcosahedron();
    /// Find if a vertex already exists, if exists return the index
    bool FindSphereCloudVertex(const glm::vec3 &pV, unsigned int &pPosition) const;
    /// Create a new face with 3 vertex from the point cloud
    void CreateSphereCloudTriangle(const glm::vec3 &pV1, const glm::vec3 &pV2, const glm::vec3 &pV3);
    /// Subdivide the point cloud recursively until \a pDepth
    void Subdivide(glm::vec3 pV1, glm::vec3 pV2, glm::vec3 pV3, unsigned short pDepth);
    /// Compute the spherical and grid coordinates from the cartesian coordinates
    void CreateVerticesInOtherCoordinates();
    /// Configure the mesh
    void SetMeshInformation();

protected:
    /// Mesh to paint the sphere point cloud
    Geometry* mMesh;
    /// Number of points
    int mNumberOfPoints;
    /// Faces
    QVector< unsigned int > mFaces;
    /// Vertexs in cartesian coordinates
    QVector< glm::vec3 > mVertices;
    /// Normals of the vertexs
    QVector< glm::vec3 > mNormals;
    /// Vertexs in spherical coordinates
    QVector< glm::vec2 > mVerticesInSphericalCoordinates;
    /// Vertexs in grid coordinates
    QVector< glm::vec2 > mVerticesInUVCoordinates;
    /// Neighbours of the vertexs
    QVector< QVector<int> > mNeighbours;
};

#endif
