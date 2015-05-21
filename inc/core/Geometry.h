/// \file Geometry.h
/// \class Geometry
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

//Dependency includes
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

//Project includes
#include "AxisAlignedBoundingBox.h"
#include "BoundingSphere.h"
#include "Material.h"

class Scene;
class GPUGeometry;

/// Class to wrap a 3d mesh that is not stored into the GPU until the GetGPUGeometry method is called.
class Geometry
{
public:
    /// Enumeration to define the topology of the mesh
    enum Topology
    {
        Points = GL_POINTS,
        Lines = GL_LINES,
        Line_Strip = GL_LINE_STRIP,
        Line_Loop = GL_LINE_LOOP,
        Triangles = GL_TRIANGLES
    };
    /// Constructor
    Geometry(const QString &pName, Topology pT);
    /// Copy constructor (vertex neighbours have to be set again)
    Geometry(const Geometry& pGeometry);
    /// Destructor
    ~Geometry();

    /// Set the vertices of the mesh
    void SetVerticesData(unsigned int pSize, const glm::vec4 *pData);
    /// Set the vertices of the mesh
    void SetVerticesData(unsigned int pSize, const glm::vec3 *pData);
    /// Set the vertices of the mesh
    void SetVerticesData(unsigned int pSize, const glm::vec2 *pData);
    /// Set the normals of the mesh
    void SetNormalsData(unsigned int pSize, const glm::vec3 *pData);
    /// Set the colors of the mesh
    void SetColorData(unsigned int pSize, const glm::vec4 *pData);
    /// Set the colors of the mesh
    void SetColorData(unsigned int pSize, const glm::vec3 *pData);
    /// Set the texture coordinates of the mesh
    void SetTextCoordsData(unsigned int pSize, const glm::vec2 *pData);
    /// Set the tangents of the mesh
    void SetTangentData(unsigned int pSize, const glm::vec3 *pData);
    /// Set the bitangents of the mesh
    void SetBitangentData(unsigned int pSize, const glm::vec3 *pData);

    /// Set the information of connectivities between vertices of the mesh
    void SetIndexsData(unsigned int pSize, const unsigned int *pData);

    QVector<float> GetVerticesData() const;
    unsigned int GetVerticesStride() const;

    /// Set the name of the mesh
    void SetName(const QString &pName);
    /// Set the topology
    void SetTopology( Topology pTopology );

    /// Set the material that will be used for the renderer
    void SetMaterial(Material* pMaterial);
    /// Get the material used for the renderer
    Material* GetMaterial() const;

    /// Have to be rendered?
    void SetVisible(bool pVisible);

    /// Compute the bounding volumes
    void ComputeBoundingVolumes();
    /// Show information of the mesh like faces, vertices and diameter of the bounding sphere
    void ShowInformation() const;

    /// Compute the area of each polygon
    void ComputeAreasOfPolygons();
    /// Get the area of the polygons
    QVector< float > GetAreasOfPolygons() const;
    /// Get the area of the polygon \param pPolygon
    float GetAreaOfPolygon( int pPolygon ) const;

    /// Get the bounding box
    AxisAlignedBoundingBox* GetBoundingBox() const;
    /// Get the bounding sphere
    BoundingSphere* GetBoundingSphere() const;

    /// Draw the mesh
    void Draw();
    /// Get the number of indexes
    int GetNumIndices() const;
    /// Get the number of faces
    int GetNumFaces() const;
    /// Get the number of vertices
    int GetNumVertices() const;
    /// Get the topology
    Topology GetTopology() const;
    /// Get the vertex position given the index value
    glm::vec3 GetVertexByIndexPosition(int pIndex) const;
    /// Get the index value givent the position in the array
    unsigned int GetValueOfIndex(int pPosition) const;

    void Normalize(const glm::vec3& pCenter, float pRadius);
    void Transform(const glm::mat4 &pTransform);

private:
    /// Get the GPUGeometry creating it if it's necessary
    const GPUGeometry* GetGPUGeometry();

    /// Data of the positions of the vertices
    QVector<float> mVertexData;
    /// Stride of the vertices
    unsigned int mVertexStride;
    /// Data of the normals of the vertices
    QVector<float> mNormalData;
    /// Data of the colors of the vertices
    QVector<float> mColorData;
    /// Stride of the colors
    unsigned int mColorStride;
    /// Data of the texture coordinates of the vertices
    QVector<float> mTextCoordsData;
    /// Data of the tangents of the vertices
    QVector<float> mTangentData;
    /// Data of the bitangents of the vertices
    QVector<float> mBitangentData;

    /// Data of the connectivity between vertices
    QVector<unsigned int> mIndexData;

    /// Area of the polygons
    QVector<float> mAreasOfPolygons;
    /// Material used to render the mesh
    Material* mMaterial;
    /// Have to be rendered?
    bool mVisible;

    /// Name of the mesh
    QString mName;
    /// Topology of the mesh
    Topology mTopology;

    /// Axis-aligned bounding box
    AxisAlignedBoundingBox* mBoundingBox;
    /// Bounding sphere
    BoundingSphere* mBoundingSphere;
    /// GPU version of the mesh
    GPUGeometry* mGPUGeometry;
    /// Boolean to know if the GPUGeometry needs to be updated
    bool mNeedGPUGeometryUpdate;
};
#endif
