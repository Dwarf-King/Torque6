//-----------------------------------------------------------------------------
// Copyright (c) 2014 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "console/consoleTypes.h"
#include "forwardMaterialAsset.h"
#include "graphics/utilities.h"

// Script bindings.
#include "forwardMaterialAsset_ScriptBinding.h"

// Debug Profiling.
#include "debug/profiler.h"

// bgfx/bx
#include <bgfx.h>
#include <bx/fpumath.h>

// Assimp - Asset Import Library
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/types.h>

//------------------------------------------------------------------------------

ConsoleType( ForwardMaterialAssetPtr, TypeForwardMaterialAssetPtr, sizeof(AssetPtr<ForwardMaterialAsset>), ASSET_ID_FIELD_PREFIX )

//-----------------------------------------------------------------------------

ConsoleGetType( TypeForwardMaterialAssetPtr )
{
    // Fetch asset Id.
    return (*((AssetPtr<ForwardMaterialAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType( TypeForwardMaterialAssetPtr )
{
    // Was a single argument specified?
    if( argc == 1 )
    {
        // Yes, so fetch field value.
        const char* pFieldValue = argv[0];

        // Fetch asset pointer.
        AssetPtr<ForwardMaterialAsset>* pAssetPtr = dynamic_cast<AssetPtr<ForwardMaterialAsset>*>((AssetPtrBase*)(dptr));

        // Is the asset pointer the correct type?
        if ( pAssetPtr == NULL )
        {
            // No, so fail.
            Con::warnf( "(TypeForwardMaterialAssetPtr) - Failed to set asset Id '%d'.", pFieldValue );
            return;
        }

        // Set asset.
        pAssetPtr->setAssetId( pFieldValue );

        return;
   }

    // Warn.
    Con::warnf( "(TypeForwardMaterialAssetPtr) - Cannot set multiple args to a single asset." );
}

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(ForwardMaterialAsset);

//------------------------------------------------------------------------------

ForwardMaterialAsset::ForwardMaterialAsset()
{
   //
}

//------------------------------------------------------------------------------

ForwardMaterialAsset::~ForwardMaterialAsset()
{
   //
}

void ForwardMaterialAsset::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();
}

void ForwardMaterialAsset::applyMaterial(Rendering::RenderData* renderData, Scene::BaseComponent* component)
{
   Parent::applyMaterial(renderData, component);

   // Render to Forward View
   renderData->view = Graphics::ViewTable::Forward;

   // Forward Lighting Uniforms
   if ( component != NULL && renderData->uniforms != NULL )
   {
      F32 lightPosRadius[4][4];
      F32 lightColorAttn[4][4];

      // Find Nearest Lights
      // TODO: Replace with Bounding Volume Hiearchy
      Vector<Rendering::LightData*> nearestLights = Rendering::getNearestLights(component->getWorldPosition());
      for( S32 t = 0; t < nearestLights.size(); ++t )
      {
         dMemcpy(lightPosRadius[t], nearestLights[t]->position, sizeof(F32) * 3);
         lightPosRadius[t][3] = nearestLights[t]->radius;
         dMemcpy(lightColorAttn[t], nearestLights[t]->color, sizeof(F32) * 3);
         lightColorAttn[t][3] = nearestLights[t]->attenuation;
      }

      // Directional Light - Direction
      Rendering::UniformData uDirLightDirection(Graphics::Shader::getUniformVec3("dirLightDirection"));
      uDirLightDirection.data = new F32[3];
      F32 dirLightDirection[3] = {0, -1.0f, 0};
      dMemcpy(uDirLightDirection.data, dirLightDirection, sizeof(dirLightDirection));
      renderData->uniforms->push_back(uDirLightDirection);

      // Directional Light - Light Color
      Rendering::UniformData uDirLightColor(Graphics::Shader::getUniformVec3("dirLightColor"));
      uDirLightColor.data = new F32[3];
      F32 dirLightColor[3] = {1.0f, 0, 1.0f};
      dMemcpy(uDirLightColor.data, dirLightColor, sizeof(dirLightColor));
      renderData->uniforms->push_back(uDirLightColor);

      // [PosX, PosY, PosZ, Radius]
      Rendering::UniformData uLightPosRadius(Graphics::Shader::getUniformVec4("lightPosRadius", 4), NULL, nearestLights.size());
      uLightPosRadius.data = new F32[4][4];
      dMemcpy(uLightPosRadius.data, lightPosRadius, sizeof(lightPosRadius));
      ///renderData->uniforms->push_back(uLightPosRadius);

      // [ColorR, ColorG, ColorB, Attenuation(0-1)]
      Rendering::UniformData uLightColorAttn(Graphics::Shader::getUniformVec4("lightColorAttn", 4), NULL, nearestLights.size());
      uLightColorAttn.data = new F32[4][4];
      dMemcpy(uLightColorAttn.data, lightColorAttn, sizeof(lightColorAttn));
      //renderData->uniforms->push_back(uLightColorAttn);
   }
}
