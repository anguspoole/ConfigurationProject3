#include "globalOpenGLStuff.h"
#include "globalStuff.h"		// for g_pRogerRabbit

#include "cVAOMeshManager.h"
#include "cMeshObject.h"

#include "DebugRenderer/cDebugRenderer.h"

#include <iostream>

// Loading models was moved into this function
void LoadModelTypes( cVAOMeshManager* pTheVAOMeshManager, GLuint shaderProgramID )
{
	sModelDrawInfo terrainInfo;
	terrainInfo.meshFileName = "IslandsMerged.ply";	// "MeshLab_Fractal_Terrain_xyz.ply";
	// Will alow me to update the vertex data in the mesh
	terrainInfo.bVertexBufferIsDynamic = true;
	pTheVAOMeshManager->LoadModelIntoVAO(terrainInfo, shaderProgramID);

	sModelDrawInfo shipInfo;
	shipInfo.meshFileName = "Sky_Pirate_Decimated_Cannons_Combined_xyz_n._uv (blender smart unwrap).ply";	// "MeshLab_Fractal_Terrain_xyz.ply";
	// Will alow me to update the vertex data in the mesh
	shipInfo.bVertexBufferIsDynamic = true;
	pTheVAOMeshManager->LoadModelIntoVAO(shipInfo, shaderProgramID);

	sModelDrawInfo houseInfo;
	houseInfo.meshFileName = "Wood_House_xyz_n._uv (blender smart unwrap).ply";	// "MeshLab_Fractal_Terrain_xyz.ply";
	// Will alow me to update the vertex data in the mesh
	houseInfo.bVertexBufferIsDynamic = true;
	pTheVAOMeshManager->LoadModelIntoVAO(houseInfo, shaderProgramID);

	sModelDrawInfo sphereInfo;
	sphereInfo.meshFileName = "Sphere_320_faces_xyz_n_GARBAGE_uv.ply";			// "Sphere_320_faces_xyz.ply";
	pTheVAOMeshManager->LoadModelIntoVAO(sphereInfo, shaderProgramID);

	sModelDrawInfo sphereInvertedNormalsInfo;
	sphereInvertedNormalsInfo.meshFileName = "Sphere_320_faces_xyz_n_GARBAGE_uv_INVERTED_NORMALS.ply";			// "Sphere_320_faces_xyz.ply";
	pTheVAOMeshManager->LoadModelIntoVAO(sphereInvertedNormalsInfo, shaderProgramID);

	// Load the textures, too
	::g_pTheTextureManager->SetBasePath("assets/textures");

	if ( ! ::g_pTheTextureManager->Create2DTextureFromBMPFile("Justin.bmp", true ) )
	{
		std::cout << "Didn't load texture" << std::endl;
	}
	::g_pTheTextureManager->Create2DTextureFromBMPFile("grass.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("brick-wall.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("water2_256.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("causticwater.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("battlelandsand1.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("stone002.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("wood_bark.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("leaves.bmp", true );
	::g_pTheTextureManager->Create2DTextureFromBMPFile("Smoke_1.bmp", true );

	if ( ::g_pTheTextureManager->Create2DTextureFromBMPFile("rock_cave_stylized_height.bmp", true ) )
	{
		std::cout << "loaded height map, too" << std::endl;
	}

	// Load the cube map
	::g_pTheTextureManager->SetBasePath("assets/textures/cubemaps");
	std::string errorString;
	//if ( ::g_pTheTextureManager->CreateCubeTextureFromBMPFiles("CityCubeMap", 
	//	 "city_lf.bmp", "city_rt.bmp",				// reverse these
	//	 "city_dn.bmp", "city_up.bmp",				// Rotate the image "right 90 degrees")
	//	 "city_ft.bmp", "city_bk.bmp", true, errorString ) )
	//{
	//	std::cout << "Loaded the city cube map OK" << std::endl;
	//}
	//else
	//{
	//	std::cout << "Error: city cube map DIDN't load. On no!" << std::endl;
	//}
	if (::g_pTheTextureManager->CreateCubeTextureFromBMPFiles("CityCubeMap",
		"TropicalSunnyDayLeft2048.bmp", "TropicalSunnyDayRight2048.bmp",
		"TropicalSunnyDayDown2048.bmp", "TropicalSunnyDayUp2048.bmp",
		"TropicalSunnyDayFront2048.bmp", "TropicalSunnyDayBack2048.bmp", true, errorString))
	//if ( ::g_pTheTextureManager->CreateCubeTextureFromBMPFiles("CityCubeMap", 
	//	 "SpaceBox_right1_posX.bmp", "SpaceBox_left2_negX.bmp", 
	//	 "SpaceBox_top3_posY.bmp", "SpaceBox_bottom4_negY.bmp", 
	//	 "SpaceBox_front5_posZ.bmp", "SpaceBox_back6_negZ.bmp", true, errorString ) )
	{
		std::cout << "Loaded the city cube map OK" << std::endl;
	}
	else
	{
		std::cout << "Error: city cube map DIDN't load. On no!" << std::endl;
	}
	//if ( ::g_pTheTextureManager->CreateCubeTextureFromBMPFiles("CityCubeMap", 
	//	 "TropicalSunnyDayLeft2048.bmp", "TropicalSunnyDayRight2048.bmp",		// Alternate these
	//	 "TropicalSunnyDayDown2048.bmp", "TropicalSunnyDayUp2048.bmp", 			// Rotate these 90 degrees
	//	 "TropicalSunnyDayFront2048.bmp", "TropicalSunnyDayBack2048.bmp", true, errorString ) )
	//{
	//	std::cout << "Loaded the city cube map OK" << std::endl;
	//}
	//else
	//{
	//	std::cout << "Error: city cube map DIDN't load. On no!" << std::endl;
	//}


	return;
}

// This is not a good place to put this, but it's near the LoadModels...
//extern cDebugRenderer* g_pDebugRenderer;// = NULL;


	
// Loads the models we are drawing into the vector
void LoadModelsIntoScene(cVAOMeshManager* pTheVAOMeshManager, std::vector<cMeshObject*> &vec_pObjectsToDraw )
{

	{	//The Island
		cMeshObject* pTerrain = new cMeshObject();
		pTerrain->position = glm::vec3(0.0f, 0.0f, 0.0f);
		pTerrain->setDiffuseColour( glm::vec3(0.5f, 0.5f, 0.3f) );
		pTerrain->setSpecularPower( 10.0f );
		pTerrain->setUniformScale(1.0f);


		pTerrain->bUseVertexColour = false;
//		pTerrain->u
		pTerrain->meshName = "IslandsMerged.ply";		// "MeshLab_Fractal_Terrain_xyz.ply";
		pTerrain->friendlyName = "MainIsland";
		//pTerrain->bIsWireFrame = true;
		pTerrain->bIsVisible = true;

		sTextureInfo sand;
		sand.name = "battlelandsand1.bmp";
		sand.strength = 1.0f;
		pTerrain->vecTextures.push_back(sand);

		sBox* pBox = new sBox();
		sModelDrawInfo cube;
		cube.meshFileName = pTerrain->meshName;
		pTheVAOMeshManager->FindDrawInfoByModelName(cube);

		for (int i = 0; i < cube.numberOfTriangles; i++)
		{
			sTriangle* pTri = new sTriangle();
			pTri->v[0] = glm::vec3(cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_1].x, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_1].y, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_1].z);
			//pTri->v[0] = glm::vec3((pTri->v[0].x * scaleX), pTri->v[0].y * scaleY, pTri->v[0].z * scaleZ);
			//pTri->v[0] += glm::vec3(posX, posY, posZ);
			pTri->v[1] = glm::vec3(cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_2].x, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_2].y, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_2].z);
			//pTri->v[1] = glm::vec3((pTri->v[1].x * scaleX), pTri->v[1].y * scaleY, pTri->v[1].z * scaleZ);
			//pTri->v[1] += glm::vec3(posX, posY, posZ);
			pTri->v[2] = glm::vec3(cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_3].x, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_3].y, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_3].z);
			//pTri->v[2] = glm::vec3((pTri->v[2].x * scaleX), pTri->v[2].y * scaleY, pTri->v[2].z * scaleZ);
			//pTri->v[2] += glm::vec3(posX, posY, posZ);

			glm::vec3 A = pTri->v[1] - pTri->v[0];
			glm::vec3 B = pTri->v[2] - pTri->v[0];

			glm::vec3 sn = glm::cross(A, B);


			//pTri->n = glm::vec3(cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_1].nx, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_1].ny, cube.pVerticesFromFile[cube.pTriangles[i].vertex_index_1].nz);
			pTri->n = sn;
			pBox->boxTris.push_back(pTri);
		}
		pTerrain->pTheShape = pBox;
		pTerrain->shapeType = cMeshObject::BOX;

		pTerrain->pDebugRenderer = ::g_pDebugRenderer;
		vec_pObjectsToDraw.push_back(pTerrain);

	}

	{	//The Ship
		cMeshObject* pShip = new cMeshObject();
		pShip->position = glm::vec3(30.0f, 20.0f, -100.0f);
		pShip->setDiffuseColour(glm::vec3(1.0f, 0.0f, 0.0f));
		pShip->setSpecularPower(70.0f);
		pShip->setUniformScale(1.0f);
		pShip->bDontLight = true;


		pShip->bUseVertexColour = false;
		//		pShip->u
		pShip->meshName = "Sky_Pirate_Decimated_Cannons_Combined_xyz_n._uv (blender smart unwrap).ply";		// "MeshLab_Fractal_Terrain_xyz.ply";
		pShip->friendlyName = "Ship";
		//pShip->bIsWireFrame = true;
		pShip->bIsVisible = true;

		pShip->pDebugRenderer = ::g_pDebugRenderer;
		vec_pObjectsToDraw.push_back(pShip);

	}

	{	//The House
		cMeshObject* pHouse = new cMeshObject();
		pHouse->position = glm::vec3(-30.0f, 27.0f, -100.0f);
		pHouse->setDiffuseColour(glm::vec3(0.0f, 1.0f, 0.0f));
		pHouse->setSpecularPower(70.0f);
		pHouse->setUniformScale(3.0f);
		pHouse->bDontLight = true;


		pHouse->bUseVertexColour = false;
		//		pHouse->u
		pHouse->meshName = "Wood_House_xyz_n._uv (blender smart unwrap).ply";		// "MeshLab_Fractal_Terrain_xyz.ply";
		pHouse->friendlyName = "House";
		//pHouse->bIsWireFrame = true;
		pHouse->bIsVisible = true;

		pHouse->pDebugRenderer = ::g_pDebugRenderer;
		vec_pObjectsToDraw.push_back(pHouse);

	}

	{	//The Ship Sphere
		cMeshObject* pSphere = new cMeshObject();
		pSphere->position = glm::vec3(30.0f, 30.0f, -100.0f);
		pSphere->setDiffuseColour(glm::vec3(1.0f, 0.0f, 0.0f));
		pSphere->setSpecularPower(70.0f);
		pSphere->setUniformScale(12.5f);


		pSphere->bUseVertexColour = false;
		//		pSphere->u
		pSphere->meshName = "Sphere_320_faces_xyz_n_GARBAGE_uv.ply";		// "MeshLab_Fractal_Terrain_xyz.ply";
		pSphere->friendlyName = "ShipSphere";
		pSphere->bIsWireFrame = true;
		pSphere->bIsVisible = false;

		sSphere * sphere = new sSphere(12.5f);
		pSphere->shapeType = cMeshObject::SPHERE;
		pSphere->pTheShape = sphere;


		pSphere->pDebugRenderer = ::g_pDebugRenderer;
		vec_pObjectsToDraw.push_back(pSphere);
	}

	{	//The House Sphere
		cMeshObject* pSphere = new cMeshObject();
		pSphere->position = glm::vec3(-30.0f, 30.0f, -100.0f);
		pSphere->setDiffuseColour(glm::vec3(0.0f, 1.0f, 0.0f));
		pSphere->setSpecularPower(70.0f);
		pSphere->setUniformScale(5.0f);


		pSphere->bUseVertexColour = false;
		//		pSphere->u
		pSphere->meshName = "Sphere_320_faces_xyz_n_GARBAGE_uv.ply";		// "MeshLab_Fractal_Terrain_xyz.ply";
		pSphere->friendlyName = "HouseSphere";
		pSphere->bIsWireFrame = true;
		pSphere->bIsVisible = false;

		sSphere * sphere = new sSphere(5.0f);
		pSphere->shapeType = cMeshObject::SPHERE;
		pSphere->pTheShape = sphere;

		pSphere->pDebugRenderer = ::g_pDebugRenderer;
		vec_pObjectsToDraw.push_back(pSphere);
	}

	{	//The Ship Sphere
		cMeshObject* pSphere = new cMeshObject();
		pSphere->position = glm::vec3(30.0f, 30.0f, -100.0f);
		pSphere->setDiffuseColour(glm::vec3(1.0f, 1.0f, 1.0f));
		pSphere->setSpecularPower(100.0f);
		pSphere->setUniformScale(5.0f);


		pSphere->bUseVertexColour = false;
		//		pSphere->u
		pSphere->meshName = "Sphere_320_faces_xyz_n_GARBAGE_uv.ply";		// "MeshLab_Fractal_Terrain_xyz.ply";
		pSphere->friendlyName = "DebugSphere";
		pSphere->bIsWireFrame = false;
		pSphere->bIsVisible = false;
		pSphere->bDontLight = true;

		sSphere * sphere = new sSphere(12.5f);
		pSphere->shapeType = cMeshObject::SPHERE;
		pSphere->pTheShape = sphere;


		pSphere->pDebugRenderer = ::g_pDebugRenderer;
		vec_pObjectsToDraw.push_back(pSphere);
	}


	{	// This will be our "skybox" object.
		// (could also be a cube, or whatever)
		cMeshObject* pSkyBoxObject = new cMeshObject();
		pSkyBoxObject->setDiffuseColour( glm::vec3( 1.0f, 105.0f/255.0f, 180.0f/255.0f ) );
		pSkyBoxObject->bUseVertexColour = false;
		pSkyBoxObject->friendlyName = "SkyBoxObject";
		float scale = 5000.0f;	
		pSkyBoxObject->nonUniformScale = glm::vec3(scale,scale,scale);
		pSkyBoxObject->meshName = "Sphere_320_faces_xyz_n_GARBAGE_uv_INVERTED_NORMALS.ply";			// "Sphere_320_faces_xyz.ply";
//		pSkyBoxObject->bIsWireFrame = true;

		// Invisible until I need to draw it
		pSkyBoxObject->bIsVisible = false;

		vec_pObjectsToDraw.push_back( pSkyBoxObject );
	}

	return;
}