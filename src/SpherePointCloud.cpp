//Definition include
#include "SpherePointCloud.h"

//Dependency includes
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

glm::vec3 SpherePointCloud::Up(const glm::vec3 &pViewpoint)
{
    float max = glm::max( glm::abs(pViewpoint.x), glm::max( glm::abs(pViewpoint.y), glm::abs(pViewpoint.z) ) );
    glm::vec3 up( glm::max(max - glm::abs(pViewpoint.y - pViewpoint.z), 0.0f), glm::abs(pViewpoint.x) + glm::abs(pViewpoint.z), glm::abs(pViewpoint.y) );
    return glm::normalize(up);
}

SpherePointCloud::SpherePointCloud():
    mNumberOfPoints(0)
{
    mMesh = new Geometry("Sphere point cloud", Geometry::Points);
}

SpherePointCloud::SpherePointCloud( const SpherePointCloud& pSpherePointCloud )
{
    if(pSpherePointCloud.mMesh != NULL)
    {
        mMesh = new Geometry(*pSpherePointCloud.mMesh);
    }
    else
    {
        mMesh = NULL;
    }
    mNumberOfPoints = pSpherePointCloud.mNumberOfPoints;
    mFaces = pSpherePointCloud.mFaces;
    mVertices = pSpherePointCloud.mVertices;
    mNormals = pSpherePointCloud.mNormals;
    mVerticesInSphericalCoordinates = pSpherePointCloud.mVerticesInSphericalCoordinates;
    mVerticesInUVCoordinates = pSpherePointCloud.mVerticesInUVCoordinates;
    mNeighbours = pSpherePointCloud.mNeighbours;
}

SpherePointCloud::~SpherePointCloud()
{
    delete mMesh;
}

void SpherePointCloud::SetToQuasiUniform(unsigned char pDepth)
{
    // Create the initial vertices and faces from the sphere cloud
    CreateIcosahedron();

    QVector< unsigned int > tempFaces = mFaces;

    mFaces.clear();

    // Each triangle has to be subdivided as many times as the sphere depth
    for( int i = 0; i < tempFaces.size(); i += 3 )
    {
        int face1 = tempFaces.at(i);
        int face2 = tempFaces.at(i+1);
        int face3 = tempFaces.at(i+2);

        Subdivide( mVertices.at(face1), mVertices.at(face2), mVertices.at(face3), pDepth );
    }
    mNumberOfPoints = mVertices.size();
    ComputeQuasiUniformNeighbours();
    CreateVerticesInOtherCoordinates();
    SetMeshInformation();
}

QVector<unsigned int> SpherePointCloud::GetFaces() const
{
    return mFaces;
}

QVector<glm::vec3> SpherePointCloud::GetVertices() const
{
    return mVertices;
}

QVector<glm::vec2> SpherePointCloud::GetVerticesInSphericalCoordinates() const
{
    return mVerticesInSphericalCoordinates;
}

QVector<glm::vec2> SpherePointCloud::GetVerticesInUVCoordinates() const
{
    return mVerticesInUVCoordinates;
}

glm::vec3 SpherePointCloud::GetVertex(unsigned int pI) const
{
    return mVertices.at(pI);
}

QVector<QVector<int> > SpherePointCloud::GetNeighbours() const
{
    return mNeighbours;
}

QVector<int> SpherePointCloud::GetNeighbours(unsigned int pI) const
{
    return mNeighbours.at(pI);
}

void SpherePointCloud::ComputeQuasiUniformNeighbours()
{
    mNeighbours.clear();
    int nFaces = mFaces.size() / 3;
    Q_ASSERT( nFaces * 3 == mFaces.size() );

    for ( int i = 0; i < mNumberOfPoints; i++ )
    {
        QVector<int> vertexNeighbours;

        for ( int j = 0; j < nFaces; j++ )
        {
            if ( mFaces.at( 3 * j ) == i )
            {
                if ( vertexNeighbours.indexOf( mFaces.at( 3 * j + 1 ) ) == -1 )
                {
                    vertexNeighbours.push_back(mFaces.at( 3 * j + 1 ));
                }
                if ( vertexNeighbours.indexOf( mFaces.at( 3 * j + 2 ) ) == -1 )
                {
                    vertexNeighbours.push_back(mFaces.at( 3 * j + 2 ));
                }
            }

            if ( mFaces.at( 3 * j + 1 ) == i )
            {
                if ( vertexNeighbours.indexOf( mFaces.at( 3 * j ) ) == -1 )
                {
                    vertexNeighbours.push_back(mFaces.at( 3 * j ));
                }
                if ( vertexNeighbours.indexOf( mFaces.at( 3 * j + 2 ) ) == -1 )
                {
                    vertexNeighbours.push_back(mFaces.at( 3 * j + 2 ));
                }
            }

            if ( mFaces.at( 3 * j + 2 ) == i )
            {
                if ( vertexNeighbours.indexOf( mFaces.at( 3 * j ) ) == -1 )
                {
                    vertexNeighbours.push_back(mFaces.at( 3 * j ));
                }
                if ( vertexNeighbours.indexOf( mFaces.at( 3 * j + 1 ) ) == -1 )
                {
                    vertexNeighbours.push_back(mFaces.at( 3 * j + 1 ));
                }
            }
        }
        mNeighbours.push_back(vertexNeighbours);
    }
}

void SpherePointCloud::CreateIcosahedron()
{
    glm::vec3 v( 1.0f, 0.0f, ( 1.0f + glm::sqrt( 5.0f ) ) / 2.0f );
    v = glm::normalize(v);

    const double ICOSAHEDRON_X = v.x;
    const double ICOSAHEDRON_Z = v.z;

    mNumberOfPoints = 12;
    mVertices.resize(mNumberOfPoints);
    mVertices[ 0] = glm::vec3( -ICOSAHEDRON_X, 0.0f,  ICOSAHEDRON_Z );
    mVertices[ 1] = glm::vec3(  ICOSAHEDRON_X, 0.0f,  ICOSAHEDRON_Z );
    mVertices[ 2] = glm::vec3( -ICOSAHEDRON_X, 0.0f, -ICOSAHEDRON_Z );
    mVertices[ 3] = glm::vec3(  ICOSAHEDRON_X, 0.0f, -ICOSAHEDRON_Z );
    mVertices[ 4] = glm::vec3( 0.0f,  ICOSAHEDRON_Z,  ICOSAHEDRON_X );
    mVertices[ 5] = glm::vec3( 0.0f,  ICOSAHEDRON_Z, -ICOSAHEDRON_X );
    mVertices[ 6] = glm::vec3( 0.0f, -ICOSAHEDRON_Z,  ICOSAHEDRON_X );
    mVertices[ 7] = glm::vec3( 0.0f, -ICOSAHEDRON_Z, -ICOSAHEDRON_X );
    mVertices[ 8] = glm::vec3(  ICOSAHEDRON_Z,  ICOSAHEDRON_X, 0.0f );
    mVertices[ 9] = glm::vec3( -ICOSAHEDRON_Z,  ICOSAHEDRON_X, 0.0f );
    mVertices[10] = glm::vec3(  ICOSAHEDRON_Z, -ICOSAHEDRON_X, 0.0f );
    mVertices[11] = glm::vec3( -ICOSAHEDRON_Z, -ICOSAHEDRON_X, 0.0f );

    mNormals = mVertices;

    mFaces.resize(60);
    mFaces[ 0] =  1; mFaces[ 1] =  4; mFaces[ 2] =  0;
    mFaces[ 3] =  4; mFaces[ 4] =  9; mFaces[ 5] =  0;
    mFaces[ 6] =  4; mFaces[ 7] =  5; mFaces[ 8] =  9;
    mFaces[ 9] =  8; mFaces[10] =  5; mFaces[11] =  4;
    mFaces[12] =  1; mFaces[13] =  8; mFaces[14] =  4;
    mFaces[15] =  1; mFaces[16] = 10; mFaces[17] =  8;
    mFaces[18] = 10; mFaces[19] =  3; mFaces[20] =  8;
    mFaces[21] =  8; mFaces[22] =  3; mFaces[23] =  5;
    mFaces[24] =  3; mFaces[25] =  2; mFaces[26] =  5;
    mFaces[27] =  3; mFaces[28] =  7; mFaces[29] =  2;
    mFaces[30] =  3; mFaces[31] = 10; mFaces[32] =  7;
    mFaces[33] = 10; mFaces[34] =  6; mFaces[35] =  7;
    mFaces[36] =  6; mFaces[37] = 11; mFaces[38] =  7;
    mFaces[39] =  6; mFaces[40] =  0; mFaces[41] = 11;
    mFaces[42] =  6; mFaces[43] =  1; mFaces[44] =  0;
    mFaces[45] = 10; mFaces[46] =  1; mFaces[47] =  6;
    mFaces[48] = 11; mFaces[49] =  0; mFaces[50] =  9;
    mFaces[51] =  2; mFaces[52] = 11; mFaces[53] =  9;
    mFaces[54] =  5; mFaces[55] =  2; mFaces[56] =  9;
    mFaces[57] = 11; mFaces[58] =  2; mFaces[59] =  7;
}

inline bool equal(const glm::vec3 & v1, const glm::vec3 & v2, float error)
{
    glm::vec3 v = v1 - v2;

    return ( glm::abs( v.x ) <= error && glm::abs( v.y ) <= error && glm::abs( v.z ) <= error );
}

bool SpherePointCloud::FindSphereCloudVertex(const glm::vec3 & pV, unsigned int & pPosition) const
{
    int i = 0;
    bool trobat = false;

    while ( i < mVertices.size() && !trobat )
    {
        if ( equal( pV, mVertices.at(i), 0.0001f ) ) trobat = true;
        else i++;
    }
    pPosition = i;

    return trobat;
}

void SpherePointCloud::CreateSphereCloudTriangle(const glm::vec3 & pV1, const glm::vec3 & pV2, const glm::vec3 & pV3)
{
    unsigned int pos;

    // Add the triangle (v1, v2, v3) to the list --> Add vertexs (if needed) and faces!!!

    if( FindSphereCloudVertex( pV1, pos ) )
    {
        // Vertex finded at position pos --> Only the index of the face is added
        mFaces.push_back( pos );
    }
    else
    {
        // Add the vertex and the index of the face
        mVertices.push_back( pV1 );
        mNormals.push_back( pV1 );
        mFaces.push_back( mVertices.size() - 1 );
    }

    if( FindSphereCloudVertex( pV2, pos ) )
    {
        // Vertex finded at position pos --> Only the index of the face is added
        mFaces.push_back( pos );
    }
    else
    {
        // Add the vertex and the index of the face
        mVertices.push_back( pV2 );
        mNormals.push_back( pV2 );
        mFaces.push_back( mVertices.size() - 1 );
    }

    if( FindSphereCloudVertex( pV3, pos ) )
    {
        // Vertex finded at position pos --> Only the index of the face is added
        mFaces.push_back( pos );
    }
    else
    {
        // Add the vertex and the index of the face
        mVertices.push_back( pV3 );
        mNormals.push_back( pV3 );
        mFaces.push_back( mVertices.size() - 1 );
    }
}

/**
 * \todo TODO: En teoria es podrien passar els vectors com a referències constants,
 * però llavors a vegades es modifiquen màgicament. (!!!!!!!!!!!!!!!!!!!!!!!)
 */
void SpherePointCloud::Subdivide(glm::vec3 pV1, glm::vec3 pV2, glm::vec3 pV3, unsigned short pDepth)
{
    if( pDepth == 0 )
    {
        CreateSphereCloudTriangle( pV1, pV2, pV3 );
    }
    else
    {
        // Find the midpoint of each edge of the triangle
        glm::vec3 v12 = pV1 + pV2;
        glm::vec3 v23 = pV2 + pV3;
        glm::vec3 v31 = pV3 + pV1;

        // It's not necessary to divide by 2 the midpoints because we normalize them
        v12 = glm::normalize(v12);
        v23 = glm::normalize(v23);
        v31 = glm::normalize(v31);

        Subdivide( pV1, v12, v31, pDepth - 1 );
        Subdivide( pV2, v23, v12, pDepth - 1 );
        Subdivide( pV3, v31, v23, pDepth - 1 );
        Subdivide( v12, v23, v31, pDepth - 1 );
    }
}

void SpherePointCloud::CreateVerticesInOtherCoordinates()
{
    mVerticesInSphericalCoordinates.resize(mNumberOfPoints);
    mVerticesInUVCoordinates.resize(mNumberOfPoints);
    for( int i = 0; i < mNumberOfPoints; i++ )
    {
        glm::vec2 sphericalCoord, gridCoord;
        const glm::vec3 & v = mVertices.at(i);

        // Theta (from 0 to 2*Pi)
        if( v.x == 0.0f && v.y == 0.0f )
        {
            sphericalCoord.x = 0.0f;
        }
        else
        {
            sphericalCoord.x = glm::acos( v.x / glm::sqrt( v.x * v.x + v.y * v.y ) );
            if( v.y < 0.0f )
            {
                sphericalCoord.x = -sphericalCoord.x;
            }
            sphericalCoord.x = sphericalCoord.x + 3.1415926535f;
        }
        // Phi (from 0 to Pi)
        sphericalCoord.y = glm::acos( v.z );//The radius is 1

        // Theta (from 0 to 1 uniformly distributed)
        gridCoord.x = sphericalCoord.x / (2.0f * 3.1415926535f);
        // Phi (from 0 to 1 uniformly distributed)
        gridCoord.y = (glm::cos(sphericalCoord.y) + 1.0f) / 2.0f;

        mVerticesInSphericalCoordinates[i] = sphericalCoord;
        mVerticesInUVCoordinates[i] = gridCoord;
    }
}

void SpherePointCloud::SetMeshInformation()
{
    mMesh->SetVerticesData(mNumberOfPoints, mVertices.data());
    mMesh->SetNormalsData(mNumberOfPoints, mNormals.data());
    if(mFaces.size() > 0)
    {
        mMesh->SetIndexsData(mFaces.size(), mFaces.data());
        mMesh->SetTopology(Geometry::Triangles);
    }
    mMesh->ComputeBoundingVolumes();
}
