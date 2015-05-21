//Definition include
#include "SceneLoader.h"

//Qt includes
#include <QFileInfo>
#include <QImageReader>

//Dependency includes
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

//Project includes
#include "Debug.h"

Scene * SceneLoader::LoadScene(const QString &pPath)
{
    Scene* sceneLoaded;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                                aiPrimitiveType_POINT |			//remove points and
                                aiPrimitiveType_LINE );			//lines
    // Impotem l'escena amb els següents paràmetres:
    //   aiProcess_GenNormals
    //   aiProcess_CalcTangentSpace
    //   aiProcess_Triangulate
    //   aiProcess_JoinIdenticalVertices
    //   aiProcess_PreTransformVertices
    //   aiProcess_RemoveRedundantMaterials
    //   aiProcess_OptimizeMeshes
    const aiScene* scene = importer.ReadFile(pPath.toLatin1().data(),
                                             aiProcess_GenNormals               |
                                             aiProcess_CalcTangentSpace         |
                                             aiProcess_Triangulate              |
                                             aiProcess_JoinIdenticalVertices    |
                                             aiProcess_PreTransformVertices     |
                                             aiProcess_RemoveRedundantMaterials |
                                             aiProcess_OptimizeMeshes           |
                                             aiProcess_SortByPType              |
                                             aiProcess_ImproveCacheLocality );

    if(scene)
    {
        QFileInfo fileInfo(pPath);
        unsigned int numSubMeshes = scene->mNumMeshes;
        QVector<Geometry *> meshesLoaded(numSubMeshes);
        // Recorrem totes les malles de l'escena
        for( unsigned int i = 0; i < numSubMeshes; i++ )
        {
            aiMesh* subMesh = scene->mMeshes[i];
            unsigned int numberOfVertices = subMesh->mNumVertices;

            QVector< glm::vec3 > vertexData(numberOfVertices);
            QVector< glm::vec3 > normalData;
            if( subMesh->HasNormals() )
            {
                normalData.resize(numberOfVertices);
            }
            QVector< glm::vec3 > tangentData;
            QVector< glm::vec3 > bitangentData;
            if( subMesh->HasTangentsAndBitangents() )
            {
                tangentData.resize(numberOfVertices);
                bitangentData.resize(numberOfVertices);
            }

            Geometry * currentMesh = new Geometry(QString(subMesh->mName.data), Geometry::Triangles);

            // Guardem els vertexs, normals, tangents i bitangents
            for( unsigned int j = 0; j < numberOfVertices; j++ )
            {
                aiVector3D tmp = subMesh->mVertices[j];
                vertexData[j] = glm::vec3(tmp.x, tmp.y, tmp.z);

                if( subMesh->HasNormals() )
                {
                    tmp = subMesh->mNormals[j];
                    normalData[j] = glm::vec3(tmp.x, tmp.y, tmp.z);
                }
                if( subMesh->HasTangentsAndBitangents() )
                {
                    tmp = subMesh->mTangents[j];
                    tangentData[j] = glm::vec3(tmp.x, tmp.y, tmp.z);

                    tmp = subMesh->mBitangents[j];
                    bitangentData[j] =  glm::vec3(tmp.x, tmp.y, tmp.z);
                }
            }

            if( subMesh->GetNumUVChannels() > 0 )
            {
                if( subMesh->HasTextureCoords(0) )
                {
                    if( subMesh->mNumUVComponents[0] == 2 )
                    {
                        // Guardem les coordenades de textura
                        QVector< glm::vec2 > textCoordData(numberOfVertices);
                        for( unsigned int j = 0; j < numberOfVertices; j++ )
                        {
                            textCoordData[j] = glm::vec2( subMesh->mTextureCoords[0][j].x, subMesh->mTextureCoords[0][j].y );
                        }
                        currentMesh->SetTextCoordsData( textCoordData.size(), textCoordData.data() );
                    }
                    else
                    {
                        Debug::Warning(QString("Only 2 UV components supported"));
                    }
                }
            }

            aiMaterial * actualAiMaterial = scene->mMaterials[subMesh->mMaterialIndex];

            Material* material = LoadMaterial(actualAiMaterial, fileInfo.absolutePath());

            currentMesh->SetMaterial(material);

            unsigned int numberOfFaces = subMesh->mNumFaces;
            QVector< unsigned int > indexData(numberOfFaces*3);
            for( unsigned int j = 0; j < subMesh->mNumFaces; j++ )
            {
                struct aiFace tmpFace = subMesh->mFaces[j];
                indexData[j*3] = tmpFace.mIndices[0];
                indexData[j*3+1] = tmpFace.mIndices[1];
                indexData[j*3+2] = tmpFace.mIndices[2];
            }
            currentMesh->SetVerticesData(vertexData.size(), vertexData.data());
            currentMesh->SetIndexsData(indexData.size(), indexData.data());
            currentMesh->SetNormalsData(normalData.size(), normalData.data());

            if((tangentData.size() > 0) && (bitangentData.size() > 0))
            {
                currentMesh->SetTangentData(tangentData.size(), tangentData.data());
                currentMesh->SetBitangentData(bitangentData.size(), bitangentData.data());
            }

            currentMesh->ComputeBoundingVolumes();
            currentMesh->ComputeAreasOfPolygons();

            meshesLoaded[i] = currentMesh;
        }
        sceneLoaded = new Scene( fileInfo.baseName(), meshesLoaded );
    }
    else
    {
        Debug::Error(QString("Impossible carregar amb l'AssimpLoader: %1").arg(importer.GetErrorString()));
        sceneLoaded = new Scene("Default");
    }
    return sceneLoaded;
}

Material* SceneLoader::LoadMaterial(const aiMaterial* pAiMaterial, const QString& pScenePath)
{
    Material* material;

    ShowMaterialInformation(pAiMaterial);
    aiString name;
    if( AI_SUCCESS == pAiMaterial->Get(AI_MATKEY_NAME, name) )
    {
        material = new Material( QString("%1").arg(name.data) );
    }
    else
    {
        material = new Material( QString("No name") );
    }

    aiColor3D color;
    if( AI_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) )
    {
        material->SetKa( glm::vec3(color.r, color.g, color.b) );
    }
    if( AI_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) )
    {
        material->SetKd( glm::vec3(color.r, color.g, color.b) );
    }
    if( AI_SUCCESS == pAiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) )
    {
        material->SetKs( glm::vec3(color.r, color.g, color.b) );
    }

    float value;
    if( AI_SUCCESS == pAiMaterial->Get(AI_MATKEY_SHININESS, value) )
    {
        material->SetShininess( value );
    }

    // Obtenim la textura difusa
    if( pAiMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 )
    {
        aiString actualAiTexturePath;

        pAiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &actualAiTexturePath );
        QString actualTexturePath( actualAiTexturePath.data );

        QString finalTexturePath = pScenePath;
        finalTexturePath.append("/").append(actualTexturePath);

        QImage* imageTexture = new QImage();
        if( imageTexture->load(finalTexturePath) )
        {
            material->SetKdTexture(imageTexture);
        }
        else
        {
            QFile file(finalTexturePath);
            if(!file.exists())
            {
                Debug::Warning(QString("Image %1 not found").arg(finalTexturePath));
            }
            else
            {
                Debug::Warning(QString("Impossible to load the image: %1").arg(finalTexturePath));
                QFileInfo info(finalTexturePath);
                if(info.suffix() == "tga")
                {
                    Debug::Warning(QString("RLE compression not supported in tga files"));
                }
                QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
                QString output;
                for(int k = 0; k < supportedFormats.size(); k++)
                {
                    output += supportedFormats.at(k);
                    output += ", ";
                }
                Debug::Log(QString("Supported images: %1").arg(output));
            }
        }
        delete imageTexture;
    }
    return material;
}

void SceneLoader::ShowMaterialInformation(const aiMaterial* pMaterial)
{
    aiString name;
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_NAME, name) )
    {
        Debug::Log( QString("Name: %1").arg(name.data) );
    }
    aiColor3D color;
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) )
    {
        Debug::Log( QString("   Ambient color: (%1, %2, %3)").arg(color.r).arg(color.g).arg(color.b) );
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) )
    {
        Debug::Log( QString("   Diffuse color: (%1, %2, %3)").arg(color.r).arg(color.g).arg(color.b) );
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) )
    {
        Debug::Log( QString("   Emissive color: (%1, %2, %3)").arg(color.r).arg(color.g).arg(color.b) );
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_REFLECTIVE, color) )
    {
        Debug::Log( QString("   Reflective color: (%1, %2, %3)").arg(color.r).arg(color.g).arg(color.b) );
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) )
    {
        Debug::Log( QString("   Specular color: (%1, %2, %3)").arg(color.r).arg(color.g).arg(color.b) );
    }
    float value;
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHININESS, value) )
    {
        Debug::Log( QString("   Shininess: %1").arg(value) );
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, value) )
    {
        Debug::Log( QString("   Shininess strength: %1").arg(value) );
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, color) )
    {
        Debug::Log( QString("   Transparent color: (%1, %2, %3)").arg(color.r).arg(color.g).arg(color.b) );
    }
    int intValue;
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_ENABLE_WIREFRAME, intValue) )
    {
        if( intValue == 0 )
        {
            Debug::Log( QString("   Wireframe: Disabled") );
        }
        else if( intValue == 1 )
        {
            Debug::Log( QString("   Wireframe: Enabled") );
        }
        else
        {
            Debug::Warning( QString("   Wireframe: Unexpected value") );
        }
    }
    if( AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHADING_MODEL, intValue) )
    {
        if( intValue == aiShadingMode_Flat )
        {
            Debug::Log( QString("   Shading model: Flat") );
        }
        else if( intValue == aiShadingMode_Gouraud )
        {
            Debug::Log( QString("   Shading model: Gouraud (simple)") );
        }
        else if( intValue == aiShadingMode_Phong )
        {
            Debug::Log( QString("   Shading model: Phong") );
        }
        else if( intValue == aiShadingMode_Blinn )
        {
            Debug::Log( QString("   Shading model: Blinn-Phong") );
        }
        else if( intValue == aiShadingMode_Toon )
        {
            Debug::Log( QString("   Shading model: Toon (comic)") );
        }
        else if( intValue == aiShadingMode_OrenNayar )
        {
            Debug::Log( QString("   Shading model: Oren-Nayar") );
        }
        else if( intValue == aiShadingMode_Minnaert )
        {
            Debug::Log( QString("   Shading model: Minnaert") );
        }
        else if( intValue == aiShadingMode_CookTorrance )
        {
            Debug::Log( QString("   Shading model: Cook-Torrance") );
        }
        else if( intValue == aiShadingMode_NoShading )
        {
            Debug::Log( QString("   Shading model: No shading") );
        }
        else if( intValue == aiShadingMode_Fresnel )
        {
            Debug::Log( QString("   Shading model: Fresnel") );
        }
        else
        {
            Debug::Warning( QString("   Shading model: Unexpected value") );
        }
    }
    unsigned int aux = pMaterial->GetTextureCount(aiTextureType_AMBIENT);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of ambient textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_AMBIENT, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_DIFFUSE);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of diffuse textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_DIFFUSE, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_DISPLACEMENT);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of displacement textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_DISPLACEMENT, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_EMISSIVE);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of emissive textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_EMISSIVE, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_HEIGHT);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of height textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_HEIGHT, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_LIGHTMAP);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of lightmap textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_LIGHTMAP, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_NORMALS);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of normals textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_NORMALS, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_OPACITY);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of opacity textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_OPACITY, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_REFLECTION);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of reflection textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_REFLECTION, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_SHININESS);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of shininess textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_SHININESS, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_SPECULAR);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of specular textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_SPECULAR, i);
        }
    }
    aux = pMaterial->GetTextureCount(aiTextureType_UNKNOWN);
    if( aux > 0 )
    {
        Debug::Log( QString("   Number of unknown textures: %1").arg( aux ) );
        for( unsigned int i = 0; i < aux; i++ )
        {
            ShowTextureInformation(pMaterial, aiTextureType_UNKNOWN, i);
        }
    }
}

void SceneLoader::ShowTextureInformation(const aiMaterial* pMaterial, aiTextureType pType, unsigned int pTextureNumber)
{
    aiString path;
    aiTextureMapping mapping;
    unsigned int uvindex;
    float blend;
    aiTextureOp op;
    aiTextureMapMode mapmode;
    Debug::Log( QString("      Information of texture %1").arg(pTextureNumber) );
    if( AI_SUCCESS == pMaterial->GetTexture( pType, pTextureNumber, &path, &mapping, &uvindex, &blend, &op, &mapmode ) )
    {
        Debug::Log( QString("         Path: %1").arg(path.data) );
        Debug::Log( QString("         UV index: %1").arg(uvindex) );
        Debug::Log( QString("         Blend: %1").arg(blend) );
    }
    else
    {
        Debug::Log( QString("         Impossible to get the texture") );
    }
}
