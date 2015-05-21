/// \file SceneLoader.h
/// \class SceneLoader
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _SCENE_LOADER_H_
#define _SCENE_LOADER_H_

//Qt includes
#include <QString>

//Dependency includes
#include "assimp/material.h"

//Project includes
#include "Material.h"
#include "Scene.h"

/// Class to load scenes from files
class SceneLoader
{
public:
    /// Create a scene from the given file
    static Scene * LoadScene(const QString &pPath);
private:
    /// Convert a aiMaterial to a Material
    static Material* LoadMaterial(const aiMaterial* pAiMaterial, const QString& pScenePath);

    /// Show the information of the material
    static void ShowMaterialInformation(const aiMaterial* pMaterial);
    /// Show the information of the texture
    static void ShowTextureInformation(const aiMaterial* pMaterial, aiTextureType pType, unsigned int pTextureNumber);
};
#endif
