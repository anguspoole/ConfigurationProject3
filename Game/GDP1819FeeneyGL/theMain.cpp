//     ___                 ___ _     
//    / _ \ _ __  ___ _ _ / __| |    
//   | (_) | '_ \/ -_) ' \ (_ | |__  
//    \___/| .__/\___|_||_\___|____| 
//         |_|                       
//
#include "globalOpenGLStuff.h"
#include "globalStuff.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

//$(SourcePath)include;		// header (.h) files
//$(SourcePath)lib/x64;		// Library (.lib) files
//#include "include/GLFW/glfw3.h"
//#include "linmath.h"		***
#include <stdlib.h>
#include <stdio.h>		// printf();
#include <iostream>		// cout (console out)

#include <vector>		// "smart array" dynamic array

#include "cShaderManager.h"
#include "cMeshObject.h"
#include "cVAOMeshManager.h"

// Here' we include the actual class
// We will "pass around" the interface class pointer, though
#include "DebugRenderer/cDebugRenderer.h"

#include "cLightHelper.h"

#include "cAABB.h"

#include <curl/curl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

cDebugRenderer* g_pDebugRendererACTUAL = NULL;
iDebugRenderer* g_pDebugRenderer = NULL;



void UpdateWindowTitle(GLFWwindow* window);


void DoPhysicsUpdate( double deltaTime, 
					  std::vector< cMeshObject* > &vec_pObjectsToDraw );

std::vector< cMeshObject* > vec_pObjectsToDraw;

// To the right, up 4.0 units, along the x axis
//glm::vec3 g_lightPos = glm::vec3( 4.0f, 4.0f, 0.0f );
//float g_lightBrightness = 400000.0f;

unsigned int numberOfObjectsToDraw = 0;

//glm::vec3 g_CameraEye = glm::vec3( 0.0, -3.0, +100.0f );
//glm::vec3 g_CameraEye = glm::vec3( 32.0, 75.0, 136.0f );
//glm::vec3 g_CameraAt = glm::vec3( 0.0, 0.0, 0.0f );
cFlyCamera* g_pFlyCamera = NULL;

cJoystickManager* g_pJoysticks = NULL;

cShaderManager* pTheShaderManager = NULL;		// "Heap" variable
cVAOMeshManager* g_pTheVAOMeshManager = NULL;

cBasicTextureManager* g_pTheTextureManager = NULL;

cAABBHierarchy* g_pTheTerrain = new cAABBHierarchy();

// And example of the light
sLight* pTheOneLight = NULL;

std::vector<glm::vec3> pathPositions;

// If true, then the "ambient brightness" spheres will be drawn
bool g_bDrawDebugLightSpheres = true;

FT_Library mft;

FT_Face mface;

const char* mvs_text =
"#version 410\n"
"attribute vec4 coord;"
"varying vec2 texpos;"
"void main () {"
"	gl_Position = vec4(coord.xy, 0, 1);"
"	texpos = coord.zw;"
"}";

const char* mfs_text =
"#version 410\n"
"varying vec2 texpos;"
"uniform sampler2D tex;"
"uniform vec4 color;"
"void main () {"
"	gl_FragColor = vec4(1, 1, 1, texture2D(tex, texpos).r) * color;"
"}";

GLuint mvertex_shader, mfragment_shader, mprogramm;
//vertex array object
GLuint mvao;
//vertex buffer object
GLuint mdp_vbo;

GLint attribute_coord;
GLint uniform_tex;
GLint uniform_color;

GLuint program;

void LoadTerrainAABB(void);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void BubbleSort(std::vector< cMeshObject* > vec_pTransparentObject);

struct MemoryStruct {
	char *memory;
	size_t size;
};

//This function was obtained from https://curl.haxx.se/libcurl/c/getinmemory.html
//We have full permission to freely use this code
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	char *ptr = (char*)std::realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

GLFWwindow* window;

int main(void)
{

	//*********************************************************************************
	//	LIBCURL - CHECKING FOR VALID LICENSE
	//*********************************************************************************

	bool validLicense = false;

	std::string email;
	std::string password;

	std::cout << "Please Enter Email: " << std::endl;
	std::cin >> email;
	std::cout << "Please Enter Password: " << std::endl;
	std::cin >> password;

	std::string loginInfo = "email=";
	loginInfo.append(email.c_str());
	loginInfo.append("&pass=");
	loginInfo.append(password.c_str());

	const char * postLogin = (const char*)loginInfo.c_str();

	//std::cout << postLogin << std::endl;

	long postSize = loginInfo.size();
	//std::cout << postSize << std::endl;

	struct MemoryStruct chunk;

	chunk.memory = (char*)std::malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	CURL * curl;
	CURLcode res;

	// Init winsock stuff
	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	if (curl)
	{
		//set the url
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/nsi/validatelicense.php");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postSize);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postLogin);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); //send data to function
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); //write to this memory location

		res = curl_easy_perform(curl); //perform curl request

		//Error check
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		curl_easy_cleanup(curl); //cleanup
	}

	curl_global_cleanup(); //cleanup

	std::cout << "License Result: " << chunk.memory << std::endl;

	std::ifstream inputstream("license.txt");
	std::string content((std::istreambuf_iterator<char>(inputstream)),
		(std::istreambuf_iterator<char>()));

	//Determine if license is valid
	if (!(std::strcmp(content.c_str(), chunk.memory)) && std::strcmp(content.c_str(), "INVALID"))
	{
		validLicense = true;
	}
	else
	{
		validLicense = false;
	}
	

	//*********************************************************************************
	//	SET UP OPENGL STUFF
	//*********************************************************************************
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
	{
        exit(EXIT_FAILURE);
	}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1024, 480, "Hello there", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

	// Mouse callbacks
	glfwSetCursorPosCallback( window, cursor_position_callback );
	glfwSetMouseButtonCallback( window, mouse_button_callback );
	glfwSetCursorEnterCallback( window, cursor_enter_callback );
	glfwSetScrollCallback( window, scroll_callback );
	glfwSetJoystickCallback(joystick_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

	::g_pJoysticks = new cJoystickManager();
	::g_pJoysticks->InitOrReset();

	// Joystick info
	std::cout << "Found: " << ::g_pJoysticks->getNumberOfJoysticksPresent() << " joysticks" << std::endl;
	if ( ::g_pJoysticks->getNumberOfJoysticksPresent() > 0 )
	{
		std::cout
			<< ">"
			<< ::g_pJoysticks->getJoystickName( ::g_pJoysticks->getFirstJoystickID() )
			<< "< is the 1st joystick found." << std::endl;
	}


	// Create the shader manager...
	//cShaderManager TheShaderManager;		// 
	//cShaderManager* pTheShaderManager;		// 
	pTheShaderManager = new cShaderManager();
	pTheShaderManager->setBasePath("assets/shaders/");

	cShaderManager::cShader vertexShader;
	cShaderManager::cShader fragmentShader;

	vertexShader.fileName = "vertex01.glsl";
	vertexShader.shaderType = cShaderManager::cShader::VERTEX_SHADER;
	
	fragmentShader.fileName = "fragment01.glsl";
	fragmentShader.shaderType = cShaderManager::cShader::FRAGMENT_SHADER;

	if ( pTheShaderManager->createProgramFromFile( "BasicUberShader", 
											       vertexShader, 
											       fragmentShader ) )
	{		// Shaders are OK
		std::cout << "Compiled shaders OK." << std::endl;
	}
	else
	{		
		std::cout << "OH NO! Compile error" << std::endl;
		std::cout << pTheShaderManager->getLastError() << std::endl;
	}

	// Load the uniform location values (some of them, anyway)
 	cShaderManager::cShaderProgram* pSP = ::pTheShaderManager->pGetShaderProgramFromFriendlyName("BasicUberShader");
	pSP->LoadUniformLocation("texture00");
	pSP->LoadUniformLocation("texture01");
	pSP->LoadUniformLocation("texture02");
	pSP->LoadUniformLocation("texture03");
	pSP->LoadUniformLocation("texture04");
	pSP->LoadUniformLocation("texture05");
	pSP->LoadUniformLocation("texture06");
	pSP->LoadUniformLocation("texture07");
	pSP->LoadUniformLocation("texBlendWeights[0]");
	pSP->LoadUniformLocation("texBlendWeights[1]");

	// Camera creation
	::g_pFlyCamera = new cFlyCamera();


	GLuint program = pTheShaderManager->getIDFromFriendlyName("BasicUberShader");


	::g_pTheVAOMeshManager = new cVAOMeshManager();

	// Create the texture manager
	::g_pTheTextureManager = new cBasicTextureManager();



	glm::vec4 waterOffset = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);


	// Loading the uniform variables here (rather than the inner draw loop)
	GLint objectColour_UniLoc = glGetUniformLocation( program, "objectColour" );
	GLint matModel_location = glGetUniformLocation(program, "matModel");
	GLint matView_location = glGetUniformLocation(program, "matView");
	GLint matProj_location = glGetUniformLocation(program, "matProj");

	GLint eyeLocation_location = glGetUniformLocation(program, "eyeLocation");

	//Pass in water offset for top
	GLint waterOffset_UniLoc = glGetUniformLocation(program, "waterOffset");
	GLint bOffsetWater_UniLoc = glGetUniformLocation(program, "bOffsetWater");

	//Pass in water offset for bottom
	GLint bOffsetWater2_UniLoc = glGetUniformLocation(program, "bOffsetWater2");
	
	// Note that this point is to the +interface+ but we're creating the actual object
	::g_pDebugRendererACTUAL = new cDebugRenderer();
	::g_pDebugRenderer = (iDebugRenderer*)::g_pDebugRendererACTUAL;

	if ( ! ::g_pDebugRendererACTUAL->initialize() )
	{
		std::cout << "Warning: couldn't init the debug renderer." << std::endl;
		std::cout << "\t" << ::g_pDebugRendererACTUAL->getLastError() << std::endl;
	}
	else
	{
		std::cout << "Debug renderer is OK" << std::endl;
	}

	//*********************************************************************************
	//	PRINT MESSAGE IF LICENSE INVALID
	//*********************************************************************************

	GLfloat green[4] = { 0, 1, 0, 1 };
	
	std::string invalidMsg = "INVALID LICENSE UNABLE TO CONTINUE";

	float sx = 2.0f / 1024;
	float sy = 2.0f / 480;

	GLfloat yoffset = 50.0f;
	GLfloat xoffset = 8 * sx;


	//If license invalid, display a message instead
	if (!validLicense)
	{
		init_gl();
		initfreetype();

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			fprintf(stderr, "Unable to init glad!");
			glfwTerminate();
			return GL_FALSE;
		}

		while (!glfwWindowShouldClose(window)) {

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(mprogramm);

			glDisable(GL_DEPTH);		// Enables the KEEPING of the depth information
			glDisable(GL_DEPTH_TEST);	// When drawing, checked the existing depth
			glDisable(GL_CULL_FACE);	// Discared "back facing" triangles

			glDepthMask(GL_TRUE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glUniform4fv(uniform_color, 1, green);

			glBindVertexArray(mvao);

			render_text(invalidMsg.c_str(), -1 + xoffset, 1 - yoffset * sy, sx, sy, yoffset);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		return 0;
	}


	//*********************************************************************************
	//	VALID LICENSE - LOAD MODELS AND PROCEED AS NORMAL
	//*********************************************************************************

	// Loading models was moved into this function
	LoadModelTypes( ::g_pTheVAOMeshManager, program );
	LoadModelsIntoScene(::g_pTheVAOMeshManager, ::vec_pObjectsToDraw );

	LoadTerrainAABB();

	// Get the current time to start with
	double lastTime = glfwGetTime();		



	// This would happen in the "light manager", where it would 
	//	do this for each light...
	pTheOneLight = new sLight();

	// Do this once...
	pTheOneLight->position_UniLoc = glGetUniformLocation( program, "theLights[0].position" );
	pTheOneLight->diffuse_UniLoc = glGetUniformLocation( program, "theLights[0].diffuse" );
	pTheOneLight->specular_UniLoc = glGetUniformLocation( program, "theLights[0].specular" );
	pTheOneLight->atten_UniLoc = glGetUniformLocation( program, "theLights[0].atten" );
	pTheOneLight->direction_UniLoc = glGetUniformLocation( program, "theLights[0].direction" );
	pTheOneLight->param1_UniLoc = glGetUniformLocation( program, "theLights[0].param1" );
	pTheOneLight->param2_UniLoc = glGetUniformLocation( program, "theLights[0].param2" );

//	pTheOneLight->position = glm::vec4( 25.0f, 25.0f, 0.0f, 1.0f );	
//	pTheOneLight->position = glm::vec4( 1.2f, 23.0f, 61.3f, 1.0f );	

	pTheOneLight->position = glm::vec4( -30.0f, 1000.0f, 20.0f, 1.0f );	
	pTheOneLight->atten.x = 0.0f;				// 	float constAtten = 0.0f;
	pTheOneLight->atten.y = 0.00000385720730f;		//	float linearAtten = 0.01f;
	pTheOneLight->atten.z = 0.000005f;		//	float quadAtten = 0.001f;
	pTheOneLight->diffuse = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );		// White light
	pTheOneLight->SetLightType( sLight::POINT_LIGHT );


	cLightHelper* pLightHelper = new cLightHelper();

	// Draw the "scene" (run the program)
	while (!glfwWindowShouldClose(window))
    {

		// Switch to the shader we want
		::pTheShaderManager->useShaderProgram( "BasicUberShader" );

        float ratio;
        int width, height;
 		//glm::mat4x4 mvp;		
		// Now these are split into separate parts
		// (because we need to know whe
		glm::mat4x4 matProjection = glm::mat4(1.0f);
		glm::mat4x4	matView = glm::mat4(1.0f);
 

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);


		glEnable( GL_DEPTH );		// Enables the KEEPING of the depth information
		glEnable( GL_DEPTH_TEST );	// When drawing, checked the existing depth
		glEnable( GL_CULL_FACE );	// Discared "back facing" triangles

		// Colour and depth buffers are TWO DIFF THINGS.
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		matProjection = glm::perspective( 0.6f,			// FOV
			                                ratio,		// Aspect ratio
			                                0.1f,			// Near clipping plane
			                                10000.0f );	// Far clipping plane

		//matView = glm::lookAt( g_CameraEye,	// Eye
		//	                    g_CameraAt,		// At
		//	                    glm::vec3( 0.0f, 1.0f, 0.0f ) );// Up
		//matView = glm::lookAt( ::g_pFlyCamera->eye,	// Eye
		//	                   ::g_pFlyCamera->getAtInWorldSpace(),		// At
		//	                   glm::vec3( 0.0f, 1.0f, 0.0f ) );// Up

	// FLY AROUND CAMERA (mouse + keyboard control)
		matView = glm::lookAt( ::g_pFlyCamera->eye,	// Eye
			                   ::g_pFlyCamera->getAtInWorldSpace(),		// At
			                   ::g_pFlyCamera->getUpVector() );// Up

		// Getting the inforamtion from the pFollow command object
//		matView = glm::lookAt( pFollow->currentLocation,	// Eye
//			                   glm::vec3(0.0f, 0.0f, 0.0f),		// At
//			                   glm::vec3(0.0f, 1.0f, 0.0f) );// Up



//		glUniform3f( eyeLocation_location, ::g_CameraEye.x, ::g_CameraEye.y, ::g_CameraEye.z );
		glUniform3f( eyeLocation_location, ::g_pFlyCamera->eye.x, ::g_pFlyCamera->eye.y, ::g_pFlyCamera->eye.z );

		glUniformMatrix4fv( matView_location, 1, GL_FALSE, glm::value_ptr(matView));
		glUniformMatrix4fv( matProj_location, 1, GL_FALSE, glm::value_ptr(matProjection));

		//Water offset
		glUniform4f(waterOffset_UniLoc, waterOffset.x, waterOffset.y, waterOffset.z, waterOffset.w);

			
		// Do all this ONCE per frame
		glUniform4f( pTheOneLight->position_UniLoc, 
						pTheOneLight->position.x, pTheOneLight->position.y, pTheOneLight->position.z, 1.0f );
		glUniform4f( pTheOneLight->diffuse_UniLoc, 
						pTheOneLight->diffuse.x, pTheOneLight->diffuse.y, pTheOneLight->diffuse.z, 1.0f );
		glUniform4f( pTheOneLight->param2_UniLoc, 1.0f, 0.0f, 0.0f, 0.0f );	// Turns it "on")
		glUniform4f( pTheOneLight->atten_UniLoc, 
						pTheOneLight->atten.x, pTheOneLight->atten.y, pTheOneLight->atten.z, pTheOneLight->atten.w );

		// Now pass the things we need for spots and directional, too:
		glUniform4f( pTheOneLight->direction_UniLoc, 
						pTheOneLight->direction.x, 
						pTheOneLight->direction.y,
						pTheOneLight->direction.z,
						pTheOneLight->direction.w );
		glUniform4f( pTheOneLight->param1_UniLoc, 
						pTheOneLight->param1.x,		// lightType
						pTheOneLight->param1.y,		// inner angle
						pTheOneLight->param1.z,		// outer angle
						pTheOneLight->param1.w );	// TBD



		{
			// ***************************************
			 // Draw the skybox first 
			cMeshObject* pSkyBox = findObjectByFriendlyName("SkyBoxObject");
//			cMeshObject* pSkyBox = findObjectByFriendlyName("SkyPirate");
			// Place skybox object at camera location
			pSkyBox->position = g_pFlyCamera->eye;
			pSkyBox->bIsVisible = true;
			pSkyBox->bIsWireFrame = false;

//			glm::vec3 oldPosition = pSkyBox->position;
//			glm::vec3 oldScale = pSkyBox->nonUniformScale;
//			pSkyBox->setUniformScale(100.0f);

//			glDisable( GL_CULL_FACE );		// Force drawing the sphere
	//		                                // Could also invert the normals
			// Draw the BACK facing (because the normals of the sphere face OUT and we 
			//  are inside the centre of the sphere..
//			glCullFace( GL_FRONT );

			// Bind the cube map texture to the cube map in the shader
			GLuint cityTextureUNIT_ID = 30;			// Texture unit go from 0 to 79
			glActiveTexture( cityTextureUNIT_ID + GL_TEXTURE0 );	// GL_TEXTURE0 = 33984

			int cubeMapTextureID = ::g_pTheTextureManager->getTextureIDFromName("CityCubeMap");

			// Cube map is now bound to texture unit 30
			//glBindTexture( GL_TEXTURE_2D, cubeMapTextureID );
			glBindTexture( GL_TEXTURE_CUBE_MAP, cubeMapTextureID );

			//uniform samplerCube textureSkyBox;
			GLint skyBoxCubeMap_UniLoc = glGetUniformLocation( program, "textureSkyBox" );
			glUniform1i( skyBoxCubeMap_UniLoc, cityTextureUNIT_ID );

			//uniform bool useSkyBoxTexture;
			GLint useSkyBoxTexture_UniLoc = glGetUniformLocation( program, "useSkyBoxTexture" );
			glUniform1f( useSkyBoxTexture_UniLoc, (float)GL_TRUE );

			glm::mat4 matIdentity = glm::mat4(1.0f);
			DrawObject( pSkyBox, matIdentity, program );

			//glEnable( GL_CULL_FACE );
			//glCullFace( GL_BACK );

			pSkyBox->bIsVisible = false;
			glUniform1f( useSkyBoxTexture_UniLoc, (float)GL_FALSE );

//			pSkyBox->position = oldPosition;
//			pSkyBox->nonUniformScale = oldScale;
			// ***************************************
		}


		std::vector< cMeshObject* > vec_pTransparentObject;


		// Draw all the objects in the "scene"
		for (unsigned int objIndex = 0;
			objIndex != (unsigned int)vec_pObjectsToDraw.size();
			objIndex++)
		{
			cMeshObject* pCurrentMesh = vec_pObjectsToDraw[objIndex];

			if (pCurrentMesh->materialDiffuse.a < 1.0f) {
				// transparent, distance from camera
				pCurrentMesh->distanceFromCamera = glm::distance(pCurrentMesh->position, g_pFlyCamera->getAtInWorldSpace());
				vec_pTransparentObject.push_back(pCurrentMesh);
				continue;

			}
			glm::mat4x4 matModel = glm::mat4(1.0f);			// mat4x4 m, p, mvp;
			
			if (pCurrentMesh->friendlyName == "LowerWater")
			{
				glUniform1f(bOffsetWater2_UniLoc, (float)GL_TRUE);
				glUniform4f(waterOffset_UniLoc, waterOffset.x, waterOffset.y, waterOffset.z, waterOffset.w);
			}
			
			DrawObject(pCurrentMesh, matModel, program);
			glUniform1f(bOffsetWater2_UniLoc, (float)GL_FALSE);
		}//for ( unsigned int objIndex = 0; 

		BubbleSort(vec_pTransparentObject);

		// Draw transparent objects
		for (unsigned int objIndex = 0;
			objIndex != (unsigned int)vec_pTransparentObject.size();
			objIndex++)
		{
			cMeshObject* pCurrentMesh = vec_pTransparentObject[objIndex];
			// check order
			//std::cout << "Distance from camera: " << pCurrentMesh->distanceFromCamera << std::endl;
			glm::mat4x4 matModel = glm::mat4(1.0f);			// mat4x4 m, p, mvp;

			if (pCurrentMesh->friendlyName == "UpperWater")
			{
				glUniform1f(bOffsetWater_UniLoc, (float)GL_TRUE);
				glUniform4f(waterOffset_UniLoc, waterOffset.x, waterOffset.y, waterOffset.z, waterOffset.w);
			}
			DrawObject(pCurrentMesh, matModel, program);
			glUniform1f(bOffsetWater_UniLoc, (float)GL_FALSE);
		}


		// Draw a reflective bunny
		{
			GLint bAddReflect_UniLoc = glGetUniformLocation( program, "bAddReflect" );
//			glUniform1f( bAddReflect_UniLoc, (float)GL_TRUE );

			GLint bAddRefract_UniLoc = glGetUniformLocation( program, "bAddRefract" );
			glUniform1f( bAddRefract_UniLoc, (float)GL_TRUE );

			//cMeshObject* pBunny = findObjectByFriendlyName("Roger");

			//glm::vec3 oldPos = pBunny->position;
			//glm::vec3 oldScale = pBunny->nonUniformScale;
			//glm::quat oldOrientation = pBunny->getQOrientation();

			//pBunny->position = glm::vec3(0.0f,25.0f,0.0f);
			//pBunny->setUniformScale(100.0f);
			//pBunny->setMeshOrientationEulerAngles( 0.0f, 0.0f, 0.0f, true );

			//glm::mat4x4 matModel = glm::mat4(1.0f);			// mat4x4 m, p, mvp;

			//DrawObject(pBunny, matModel, program);

			//pBunny->position = oldPos;
			//pBunny->nonUniformScale = oldScale;
			//pBunny->setQOrientation(oldOrientation);

			glUniform1f( bAddReflect_UniLoc, (float)GL_FALSE );
			glUniform1f( bAddRefract_UniLoc, (float)GL_FALSE );
		}

		//{// Height map vertex displacement
		//	cMeshObject* pTerrain = findObjectByFriendlyName("The Terrain");

		//	glm::vec3 oldPos = pTerrain->position;
		//	pTerrain->position.y += 50.0f;
		//	pTerrain->bIsWireFrame = true;
		//	pTerrain->bUseVertexColour = true;
		//	pTerrain->setDiffuseColour( glm::vec3(1.0f, 1.0f, 1.0f ) );
		//	pTerrain->bDontLight = true;

		//	// Set all the things in our shader
		//	GLint texHeightMap_UniLoc = glGetUniformLocation( program, "texHeightMap");
		//	GLint bUseHeightMap_UniLoc = glGetUniformLocation( program, "bUseHeightMap");
		//	GLint heightMapRatio_UniLoc = glGetUniformLocation( program, "heightMapRatio");

		//	glUniform1f( bUseHeightMap_UniLoc, (float)GL_TRUE );

		//	//glUniform1f( heightMapRatio_UniLoc, 50.0f );
		//	glUniform1f( heightMapRatio_UniLoc, HACK_HEIGHT_MAP_RATIO_ADJUST );

		//	HACK_HEIGHT_MAP_RATIO_ADJUST += 0.1f;
		//	if ( HACK_HEIGHT_MAP_RATIO_ADJUST > 250.0f )
		//	{
		//		HACK_HEIGHT_MAP_RATIO_ADJUST = 0.0f;
		//	}

		//	// Texture binding to the texture unit
		//	GLuint heightTextureID = ::g_pTheTextureManager->getTextureIDFromName("rock_cave_stylized_height.bmp");

		//	GLuint textureUNIT_ID = 50;			// Texture unit go from 0 to 79
		//	glActiveTexture( textureUNIT_ID + GL_TEXTURE0 );	// GL_TEXTURE0 = 33984

		//	glBindTexture( GL_TEXTURE_2D, heightTextureID );

		//	glUniform1i( texHeightMap_UniLoc, textureUNIT_ID );
		//	//
	
		//	glm::mat4x4 matModel = glm::mat4(1.0f);			// mat4x4 m, p, mvp;
		//	DrawObject(pTerrain, matModel, program);

		//	glUniform1f( bUseHeightMap_UniLoc, (float)GL_FALSE );

		//	// Set everything back
		//	pTerrain->position = oldPos;
		//	pTerrain->bIsWireFrame = false;
		//	pTerrain->bUseVertexColour = false;
		//	pTerrain->bDontLight = false;
		//}// 



		// High res timer (likely in ms or ns)
		double currentTime = glfwGetTime();		
		double deltaTime = currentTime - lastTime; 


		double MAX_DELTA_TIME = 0.1;	// 100 ms
		if ( deltaTime > MAX_DELTA_TIME)
		{
			deltaTime = MAX_DELTA_TIME;
		}



		waterOffset.x += (0.1f * deltaTime);
		waterOffset.y += (0.017f * deltaTime);
		waterOffset.z -= (0.13f * deltaTime);
		waterOffset.z -= (0.013f * deltaTime);




		for ( unsigned int objIndex = 0; 
			  objIndex != (unsigned int)vec_pObjectsToDraw.size(); 
			  objIndex++ )
		{	
			cMeshObject* pCurrentMesh = vec_pObjectsToDraw[objIndex];
			
			pCurrentMesh->Update( deltaTime );

		}//for ( unsigned int objIndex = 0; 

		
		// Call the debug renderer call
//#ifdef _DEBUG
		::g_pDebugRendererACTUAL->RenderDebugObjects( matView, matProjection, deltaTime );
//#endif 

	//	// **********************************************
	//{// START OF: AABB debug stuff
	//	//HACK: Draw Debug AABBs...

	//	// Get that from FindObjectByID()
	//	cMeshObject* pTheBunny = findObjectByFriendlyName("Roger");
	//	// Highlight the AABB that the rabbit is in (Or the CENTRE of the rabbit, anyway)

	//	float sideLength = 20.0f;
	//	unsigned long long ID_of_AABB_We_are_in = cAABB::generateID( pTheBunny->position, sideLength );

	//	// Is there a box here? 
	//	std::map< unsigned long long /*ID of the AABB*/, cAABB* >::iterator itAABB_Bunny			
	//		= ::g_pTheTerrain->m_mapAABBs.find(ID_of_AABB_We_are_in);

	//	// Is there an AABB there? 
	//	if ( itAABB_Bunny != ::g_pTheTerrain->m_mapAABBs.end() )
	//	{
	//		// Yes, then get the triangles and do narrow phase collision

	//		std::cout << "ID = " << ID_of_AABB_We_are_in 
	//			<< " with " << itAABB_Bunny->second->vecTriangles.size() <<" triangles" << std::endl;

	//		// *******************************************************************
	//		// Here you can pass this vector of triangles into your narrow phase (aka project #1)
	//		// and do whatever collision response you want
	//		// *******************************************************************
	//	}
	//	else
	//	{
	//		std::cout << "ID = " << ID_of_AABB_We_are_in << " NOT PRESENT near bunny" << std::endl;
	//	}


	//	std::map< unsigned long long /*ID of the AABB*/, cAABB* >::iterator itAABB 
	//		= ::g_pTheTerrain->m_mapAABBs.begin();
	//	for ( ; itAABB !=  ::g_pTheTerrain->m_mapAABBs.end(); itAABB++ )
	//	{
	//		// You could draw a mesh cube object at the location, 
	//		// but be careful that it's scalled and placed at the right location.
	//		// i.e. our cube is centred on the origin and is ++2++ units wide, 
	//		// because it's +1 unit from the centre (on all sides).

	//		// Since this is debug, and the "draw debug line thing" is working, 
	//		// this will just draw a bunch of lines... 

	//		cAABB* pCurrentAABB = itAABB->second;

	//		glm::vec3 cubeCorners[6];

	//		cubeCorners[0] = pCurrentAABB->getMinXYZ();
	//		cubeCorners[1] = pCurrentAABB->getMinXYZ();
	//		cubeCorners[2] = pCurrentAABB->getMinXYZ();
	//		cubeCorners[3] = pCurrentAABB->getMinXYZ();
	//		cubeCorners[4] = pCurrentAABB->getMinXYZ();
	//		cubeCorners[5] = pCurrentAABB->getMinXYZ();

	//		// Max XYZ
	//		cubeCorners[1].x += pCurrentAABB->getSideLength();
	//		cubeCorners[1].y += pCurrentAABB->getSideLength();
	//		cubeCorners[1].z += pCurrentAABB->getSideLength();

	//		cubeCorners[2].x += pCurrentAABB->getSideLength();

	//		cubeCorners[3].y += pCurrentAABB->getSideLength();

	//		cubeCorners[4].z += pCurrentAABB->getSideLength();

	//		// TODO: And the other corners... 
	//		cubeCorners[5].x += pCurrentAABB->getSideLength();
	//		cubeCorners[5].y += pCurrentAABB->getSideLength();

	//		// Draw line from minXYZ to maxXYZ
	//		::g_pDebugRenderer->addLine( cubeCorners[0], cubeCorners[1], 
	//									 glm::vec3(1,1,1), 0.0f );
	//	}
	//}// END OF: Scope for aabb debug stuff
	//// ********************************************************************************
		




		// update the "last time"
		lastTime = currentTime;

		// The physics update loop
		DoPhysicsUpdate( deltaTime, vec_pObjectsToDraw );

		for (int i = 0; i < pathPositions.size(); i++)
		{
			cMeshObject * pDebugBall = findObjectByFriendlyName("DebugSphere");
			pDebugBall->position = pathPositions[i];
			glm::mat4 matWorld = glm::mat4(1.0f);
			pDebugBall->bIsVisible = true;
			DrawObject(pDebugBall, matWorld, ::pTheShaderManager->getIDFromFriendlyName("BasicUberShader"));
			pDebugBall->bIsVisible = false;
		}



		//********************************************************
		// Can get that from the VAOMesh Manager...
		sModelDrawInfo theTerrainMesh;
		theTerrainMesh.meshFileName = "MeshLab_Fractal_Terrain_xyz_n_uv.ply";
//		theTerrainMesh.meshFileName = "singleTriangle_xyz.ply";
		::g_pTheVAOMeshManager->FindDrawInfoByModelName( theTerrainMesh );

		// Move the terrain a little bit...
//	for ( unsigned int index = 0; index != theTerrainMesh.numberOfVertices; index++ )
//	{
//		// There is an array of vertices like this:
//		//	sVert_xyz_rgb* pVerticesToVBO;	// = new sVert_xyz_rgb[ARRAYSIZE]
//
//		theTerrainMesh.pVerticesToVBO[index].y += 0.1f;
//		//theTerrainMesh.pVerticesToVBO[index].y *= sin(glfwGetTime()) * 1.0f;
//		theTerrainMesh.pVerticesToVBO[index].r = getRandBetween0and1<float>();
//		theTerrainMesh.pVerticesToVBO[index].g = getRandBetween0and1<float>();
//		theTerrainMesh.pVerticesToVBO[index].b = getRandBetween0and1<float>();
//	}

		// Change the terrain mesh (in the GPU)
		::g_pTheVAOMeshManager->UpdateModelVertexInformation( theTerrainMesh );



		// 

		// Get that from FindObjectByID()
		cMeshObject* pTheBunny = findObjectByFriendlyName("Roger");

		std::vector<sClosestPointData> vecClosestPoints;
		//CalculateClosestPointsOnMesh( theTerrainMesh, 
		//							  pTheBunny->position, 
		//							  vecClosestPoints );

		// Find the "closest triangle"
		// Go through each point and compare, finding the closest
		// (What if you have more than one that's the same distance?)
		// Assume the 1st one is the closet one.
		if ( vecClosestPoints.size() != 0 )
		{
			float minDistance = glm::distance( pTheBunny->position, vecClosestPoints[0].thePoint );
			unsigned int minTriangleIndex = vecClosestPoints[0].triangleIndex;

			for ( unsigned int triIndex = 0; triIndex != vecClosestPoints.size(); triIndex++ )
			{
			
				// glm::length() seems to work here, too 
				float curDist = glm::distance( pTheBunny->position, vecClosestPoints[triIndex].thePoint );

				if ( curDist < minDistance ) 
				{
					minDistance = curDist;
					minTriangleIndex = vecClosestPoints[triIndex].triangleIndex;
				}
			}
		}//if ( vecClosestPoints.size() != 0 )

//		std::cout << minTriangleIndex << std::endl;
//
//		// Mode the debug sphere to these points
//		// We can only draw one, so pick the 1st one..
//		cMeshObject* pDebugSphere = findObjectByFriendlyName("DebugSphere");
///
//		pDebugSphere->bIsVisible = true;
//		for ( unsigned int index = 0; index != vecClosestPoints.size(); index++ )
//		{
//			pDebugSphere->position = vecClosestPoints[index].thePoint;
//
//			// Draw it 
//			glm::mat4 matModel = glm::mat4(1.0f);	// Identity matrix
//
////			DrawObject( pDebugSphere, matModel, program );
//
//		}
//		pDebugSphere->bIsVisible = false;

		//********************************************************

		// At this point the scene is drawn...
		UpdateWindowTitle(window);

		glfwSwapBuffers(window);		// Shows what we drew

        glfwPollEvents();

		ProcessAsyncKeys(window);

		ProcessAsyncMouse(window);

		ProcessAsyncJoysticks(window, ::g_pJoysticks);



    }//while (!glfwWindowShouldClose(window))



	// Delete stuff
	delete pTheShaderManager;
	delete ::g_pTheVAOMeshManager;
	delete ::g_pTheTextureManager;

	// 
	delete ::g_pDebugRenderer;
	delete ::g_pFlyCamera;
	delete ::g_pJoysticks;

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}



void LoadTerrainAABB(void)
{
	// *******
	// This REALLY should be inside the cAABBHierarchy, likely... 
	// *******


	// Read the graphics mesh object, and load the triangle info
	//	into the AABB thing.
	// Where is the mesh (do the triangles need to be transformed)??

	//cMeshObject* pTerrain = findObjectByFriendlyName("PlayerShip");
	cMeshObject* pTerrain = findObjectByFriendlyName("MainIsland");

	sModelDrawInfo terrainMeshInfo;
	terrainMeshInfo.meshFileName = pTerrain->meshName;

	::g_pTheVAOMeshManager->FindDrawInfoByModelName(terrainMeshInfo);


	// How big is our AABBs? Side length?
	float sideLength = 20.0f;		// Play with this lenght
									// Smaller --> more AABBs, fewer triangles per AABB
									// Larger --> More triangles per AABB

	for (unsigned int triIndex = 0; triIndex != terrainMeshInfo.numberOfTriangles; triIndex++)
	{
		// for each triangle, for each vertex, determine which AABB the triangle should be in
		// (if your mesh has been transformed, then you need to transform the tirangles 
		//  BEFORE you do this... or just keep the terrain UNTRANSFORMED)

		sPlyTriangle currentTri = terrainMeshInfo.pTriangles[triIndex];

		sPlyVertex currentVerts[3];
		currentVerts[0] = terrainMeshInfo.pVerticesFromFile[currentTri.vertex_index_1];
		currentVerts[1] = terrainMeshInfo.pVerticesFromFile[currentTri.vertex_index_2];
		currentVerts[2] = terrainMeshInfo.pVerticesFromFile[currentTri.vertex_index_3];

		// This is the structure we are eventually going to store in the AABB map...
		cAABB::sAABB_Triangle curAABBTri;
		curAABBTri.verts[0].x = currentVerts[0].x;
		curAABBTri.verts[0].y = currentVerts[0].y;
		curAABBTri.verts[0].z = currentVerts[0].z;
		curAABBTri.verts[1].x = currentVerts[1].x;
		curAABBTri.verts[1].y = currentVerts[1].y;
		curAABBTri.verts[1].z = currentVerts[1].z;
		curAABBTri.verts[2].x = currentVerts[2].x;
		curAABBTri.verts[2].y = currentVerts[2].y;
		curAABBTri.verts[2].z = currentVerts[2].z;

		// Is the triangle "too big", and if so, split it (take centre and make 3 more)
		// (Pro Tip: "too big" is the SMALLEST side is greater than HALF the AABB length)
		// Use THOSE triangles as the test (and recursively do this if needed),
		// +++BUT+++ store the ORIGINAL triangle info NOT the subdivided one
		// 
		// For the student to complete... 
		// 


		for (unsigned int vertIndex = 0; vertIndex != 3; vertIndex++)
		{
			glm::vec3 min = curAABBTri.verts[0];
			min.y = 0.0f;
			glm::vec3 maxXYZ = min + sideLength;
			maxXYZ.y = 0.0f;

			// What AABB is "this" vertex in? 
			unsigned long long AABB_ID =
				cAABB::generateID(min,
					maxXYZ);

			// Do we have this AABB alredy? 
			std::map< unsigned long long/*ID AABB*/, cAABB* >::iterator itAABB
				= ::g_pTheTerrain->m_mapAABBs.find(AABB_ID);

			if (itAABB == ::g_pTheTerrain->m_mapAABBs.end())
			{
				// We DON'T have an AABB, yet
				cAABB* pAABB = new cAABB();
				// Determine the AABB location for this point
				// (like the generateID() method...)
				glm::vec3 minXYZ = curAABBTri.verts[0];
				minXYZ.y = 0.0f;

				float xSize = (float)(maxXYZ.x - minXYZ.x);
				float ySize = (float)(maxXYZ.y - minXYZ.y);
				float zSize = (float)(maxXYZ.z - minXYZ.z);

				/*minXYZ.x = (floor(maxXYZ.x / minXYZ.x));
				minXYZ.y = (floor(maxXYZ.y / minXYZ.y));
				minXYZ.z = (floor(maxXYZ.z / minXYZ.z));*/
				minXYZ.x = (floor(minXYZ.x / xSize) * xSize);
				minXYZ.y = (floor(minXYZ.y / ySize) * ySize);
				minXYZ.z = (floor(minXYZ.z / zSize) * zSize);

				maxXYZ = minXYZ + sideLength;

				/*minXYZ.x = (floor(minXYZ.x / sideLength)) * sideLength;
				minXYZ.y = (floor(minXYZ.y / sideLength)) * sideLength;
				minXYZ.z = (floor(minXYZ.z / sideLength)) * sideLength;*/

				pAABB->setMinXYZ(minXYZ);
				pAABB->setMaxXYZ(maxXYZ);
				//pAABB->setSideLegth( sideLength );
				// Note: this is the SAME as the AABB_ID...
				unsigned long long the_AABB_ID = pAABB->getID();

				::g_pTheTerrain->m_mapAABBs[the_AABB_ID] = pAABB;

				// Then set the iterator to the AABB, by running find again
				itAABB = ::g_pTheTerrain->m_mapAABBs.find(the_AABB_ID);
			}//if( itAABB == ::g_pTheTerrain->m_mapAABBs.end() )

			// At this point, the itAABB ++IS++ pointing to an AABB
			// (either there WAS one already, or I just created on)

			itAABB->second->vecTriangles.push_back(curAABBTri);

		}//for ( unsigned int vertIndex = 0;

	}//for ( unsigned int triIndex

		// At runtime, need a "get the triangles" method...

	return;
}

void UpdateWindowTitle(GLFWwindow* window)
{
	// #include sstream 
	std::stringstream ssTitle;

	ssTitle			// std::cout 
		<< pTheOneLight->atten.x << ", " 
		<< pTheOneLight->atten.y << ", " 
		<< pTheOneLight->atten.z;

	glfwSetWindowTitle( window, ssTitle.str().c_str() );

	return;
}

cMeshObject* findObjectByFriendlyName(std::string theNameToFind)
{
	for ( unsigned int index = 0; index != vec_pObjectsToDraw.size(); index++ )
	{
		// Is this it? 500K - 1M
		// CPU limited Memory delay = 0
		// CPU over powered (x100 x1000) Memory is REAAAAALLY SLOW
		if ( vec_pObjectsToDraw[index]->friendlyName == theNameToFind )
		{
			return vec_pObjectsToDraw[index];
		}
	}

	// Didn't find it.
	return NULL;	// 0 or nullptr
}


cMeshObject* findObjectByUniqueID(unsigned int ID_to_find)
{
	for ( unsigned int index = 0; index != vec_pObjectsToDraw.size(); index++ )
	{
		if ( vec_pObjectsToDraw[index]->getUniqueID() == ID_to_find )
		{
			return vec_pObjectsToDraw[index];
		}
	}

	// Didn't find it.
	return NULL;	// 0 or nullptr
}


void swap(cMeshObject* xp, cMeshObject*   yp)
{
	cMeshObject temp = *xp;
	*xp = *yp;
	*yp = temp;
}

void BubbleSort(std::vector< cMeshObject* > vec_pTransparentObject) {
	int i, j;
	for (i = 0; i < vec_pTransparentObject.size(); i++)

		// Last i elements are already in place    
		for (j = 0; j < vec_pTransparentObject.size() - i - 1; j++) {
			if (vec_pTransparentObject[j]->distanceFromCamera < vec_pTransparentObject[j + 1]->distanceFromCamera) {

				swap(vec_pTransparentObject[j], vec_pTransparentObject[j + 1]);
			}
		}
}

GLboolean init_gl() {

	//create shaders
	mvertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(mvertex_shader, 1, &mvs_text, NULL);
	glCompileShader(mvertex_shader);

	mfragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(mfragment_shader, 1, &mfs_text, NULL);
	glCompileShader(mfragment_shader);

	mprogramm = glCreateProgram();
	glAttachShader(mprogramm, mvertex_shader);
	glAttachShader(mprogramm, mfragment_shader);

	glLinkProgram(mprogramm);

	//get vertex attribute/s id/s
	attribute_coord = glGetAttribLocation(mprogramm, "coord");
	uniform_tex = glGetUniformLocation(mprogramm, "tex");
	uniform_color = glGetUniformLocation(mprogramm, "color");

	if (attribute_coord == -1 || uniform_tex == -1 || uniform_color == -1) {
		fprintf(stderr, "unable to get attribute or uniform from shader\n");
		return GL_FALSE;
	}

	//generate and bind vbo 
	glGenBuffers(1, &mdp_vbo);

	//generate and bind vao
	glGenVertexArrays(1, &mvao);

	return GL_TRUE;
}

GLboolean initfreetype() {

	if (FT_Init_FreeType(&mft))
	{
		fprintf(stderr, "unable to init free type\n");
		return GL_FALSE;
	}

	if (FT_New_Face(mft, "assets\\fonts\\times.ttf", 0, &mface))
	{
		fprintf(stderr, "unable to open font\n");
		return GL_FALSE;
	}

	//set font size
	FT_Set_Pixel_Sizes(mface, 0, 48);


	if (FT_Load_Char(mface, 'X', FT_LOAD_RENDER))
	{
		fprintf(stderr, "unable to load character\n");
		return GL_FALSE;
	}

	return GL_TRUE;
}

void render_text(const char *text, float x, float y, float sx, float sy, float yOffset) {

	char *p;
	FT_GlyphSlot g = mface->glyph;

	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(uniform_tex, 0);

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Set up the VBO for our vertex data */
	glEnableVertexAttribArray(attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, mdp_vbo);
	glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

	float xOrigin = x;

	int mwidth;
	int mheight;
	glfwGetFramebufferSize(window, &mwidth, &mheight);

	/* Loop through all characters */
	for (p = (char*)text; *p; p++) {

		//If we run into a newline, simply insert the newline ourselves
		if (*p == ('\n'))
		{
			x = xOrigin;
			yOffset += 50.0f;
			y = 1 - yOffset * sy;
			continue;
		}
		//Skip line feed
		if (*p == ('\t'))
		{
			continue;
		}

		//Insert newline if we reach the end of the window
		if (x + sx > (0.99) - sx)
		{
			x = xOrigin;
			yOffset += 50.0f;
			y = 1 - yOffset * sy;
		}

		/* Try to load and render the character */
		if (FT_Load_Char(mface, *p, FT_LOAD_RENDER))
			continue;

		/* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
		);

		/* Calculate the vertex and texture coordinates */
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] = {
			{ x2, -y2, 0, 0 },
		{ x2 + w, -y2, 1, 0 },
		{ x2, -y2 - h, 0, 1 },
		{ x2 + w, -y2 - h, 1, 1 },
		};

		//glBindVertexArray(mvao);
		/* Draw the character on the screen */
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Advance the cursor to the start of the next character */
		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glDisableVertexAttribArray(attribute_coord);
	glDeleteTextures(1, &tex);

	p = 0;
	delete p;
}