/// \file ViewpointsMesh.h
/// \class ViewpointsMesh
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _VIEWPOINTS_MESH_H_
#define _VIEWPOINTS_MESH_H_

//Qt includes
#include <QVector>

//Dependency includes
#include "glm/vec3.hpp"

//Project includes
#include "Camera.h"
#include "Geometry.h"

/// Classe per crear una malla de punts de vista
class ViewpointsMesh
{
public:
    /// Constructor de la malla de punts de vista donada una llista de cameres
    ViewpointsMesh(const QVector< Camera * >& pCameras);
    /// Constructor de la malla de punts de vista basada amb la descomposició del màxim sólid platònic
    ViewpointsMesh(const glm::vec3& pCenter, float pRadius, float pAngle, float pAspectRatio, int pSubdivision);
    /// Constructor de la malla de punts de vista donat un fitxer xml o txt
    /// \param pCenter, pRadius Centre i radi de l'escena per tal d'ajustar el near plane i el far plane
    ViewpointsMesh(const glm::vec3& pCenter, float pRadius, const QString& pFileName);
    /// Copy constructor
    ViewpointsMesh(const ViewpointsMesh& pViewpointsMesh);
    /// Destructor de la malla de punts de vista
    ~ViewpointsMesh();

    /// S'assigna la llista de punts de vista
    void SetCameras(const QVector< Camera * >& pCameras);
    /// S'assigna la relació de veïns de cada punt de vista
    void SetNeighbours(const QVector< QVector< int > >& pNeighbours);
    /// S'assigna la llista de cares de la malla poligonal
    void SetFaces(const QVector< unsigned int >& pFaces);

    /// Retorna el punt de vista de la posició \a pIndex.
    Camera* GetViewpoint( int pIndex ) const;
    /// Retorna el punt de vista més proper a \a pPoint.
    int GetNearestViewpoint( const glm::vec3& pPoint ) const;
    /// Retorna el nombre de punts de vista
    int GetNumberOfViewpoints() const;
    /// Retorna la relació de veïns de cada punt de vista
    QVector< QVector< int > > GetNeighbours() const;
    /// Retorna els veïns del punt de vista \a pViewpoint
    QVector< int > GetNeighbours( int pViewpoint ) const;
    /// Retorna el veí número \a pNeighbourIndex del punt de vista \a pViewpoint
    int GetNeighbour( int pViewpoint, int pNeighbourIndex ) const;
    /// Retorna la malla poligonal
    Geometry* GetMesh() const;
    /// Retorna la llista de posicions dels punts de vista
    QVector< glm::vec3 > GetVertices() const;
    /// Retorna la llista de cares de la malla poligonal
    QVector< unsigned int > GetFaces() const;

private:
    /// Llista de punts de vista
    QVector< Camera * > mCameras;
    /// Llista de posicions dels punts de vista
    QVector< glm::vec3 > mVertices;
    /// Llista de veïns de cada punt de vista
    QVector< QVector< int > > mNeighbours;
    /// Malla poligonal per tal de poder pintar la malla de punts de vista
    Geometry* mMesh;
    /// Llista de cares de la malla poligonal
    QVector< unsigned int > mFaces;
};

#endif
