#include "Game.h"



Game::Game(void)
{
}

Game::~Game(void)
{
	endGame();
}

void Game::ResetCamera(void)
{
	mpCamera->hardReset( width, height );
}

void Game::hookResize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;

	glViewport(0,0, width, height);

	mpCamera->setProjectionMatrix( (float)width, (float)height );
}

void Game::hookMouseMoved(int x, int y)
{
	if (x == width / 2 && y == height / 2)
		return;

	float dX = (width / 2.0f) - x;
	float dY = (height / 2.0f) - y;

	dX = -glm::radians(dX) * m_CameraLookSpeed;
	dY = -glm::radians(dY) * m_CameraLookSpeed;

	mpCamera->setPitch(mpCamera->getPitch() - dY);
	mpCamera->setYaw(mpCamera->getYaw() - dX);

	glutWarpPointer( width / 2, height / 2);
}

void Game::hookMousePassive(int x, int y)
{
	if (x == width / 2 && y == height / 2)
		return;

	glutWarpPointer( width / 2, height / 2);
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	
	mat4x4 mView = mpCamera->getViewMatrix();
	mat4x4 proj = mpCamera->getProjectionMatrix();
	mat4x4 viewProj = proj * mView;

	mpResourceManager->drawObject(mView, proj, viewProj, waterShaderManager, "water");
	mpResourceManager->drawAllObjects(mView, proj, viewProj, shaderManager);
	
}

void Game::update()
{
	mpCamera->update();
	mpTerrainManager->update(mpCamera);

	if(m_cloudSkybox->getIsPrefab() && mpCamera->getPos().y >= mpResourceManager->getObject("water")->getPos().y)
	{
		//cout << "switch to cloud box\n";
		m_cloudSkybox->setIsPrefab(false);
		m_underWaterSkybox->setIsPrefab(true);
	}
	else if(m_underWaterSkybox->getIsPrefab() && mpCamera->getPos().y < mpResourceManager->getObject("water")->getPos().y)
	{
		//cout << "switch to water box\n";
		m_cloudSkybox->setIsPrefab(true);
		m_underWaterSkybox->setIsPrefab(false);
	}
	
	m_underWaterSkybox->setPos(mpCamera->getPos());
	m_cloudSkybox->setPos(mpCamera->getPos());
	
	mpResourceManager->getObject("fishy")->modifyRotation(.1f, .1f, .1f);

	mpResourceManager->updateObjects(mpCamera->getPos());

	waterShaderManager->update();
}

void Game::start(int argNum, char* args[])
{
	m_CameraMoveSpeed = .5f;
	m_CameraLookSpeed = .2f;
	wireframe = false;
	fog = false;

	mpCamera = new Camera();
	mpResourceManager = new ResourceManager();
	mpTerrainManager = new TerrainManager();
	waterShaderManager = new Shader_Manager();

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);   
	shaderManager.InitializeStockShaders();
	
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	/*GLfloat fogColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};


	glFogi (GL_FOG_MODE, GL_LINEAR );
	glFogfv (GL_FOG_COLOR, fogColor);
	glFogf (GL_FOG_DENSITY, .02f);
	glHint (GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 1.0f);             // Fog Start Depth
	glFogf(GL_FOG_END, 100.0f);
	*/

	glEnable(GL_BLEND);
	  
	width = 800;
	height = 600;

	if( argNum > 1)
	{
		for (int i = 0; i < argNum; i++)
		{
			cout << args[i];
		}
	}

	setUpWorld(argNum, args);
}

void Game::setUpWorld(int argNum, char* args[])
{
	int waterTerrainSize = 8000;
	int terrainSize = 3000;
	int terrainTriDensity = 100;

	if(argNum > 1)
	{
		for( int i = 1; i < argNum; i++)
		{
			mpResourceManager->LoadFile(args[i]);
		}
	}
	else
	{
		waterShaderManager->init("WaterVertShader.vp", "WaterFragShader.fp");

		mpResourceManager->LoadFile("Assets/Fish/FISHY.obj");

		m_cloudSkybox = new Skybox("Assets/Skybox/cloudBox.jpg", mpResourceManager, 3000,"cloudSkybox");
		m_cloudSkybox->setIsPrefab(true);
		m_underWaterSkybox = new Skybox("Assets/WaterBox/waterSkybox.png", mpResourceManager,3000, "underwaterSkybox" );

		//mpTerrainManager->addNewRepeatableTerrain(mpResourceManager, "Assets/Heightmap/heightmap_water.jpg", "Assets/Heightmap/heightmap_waterTexture.jpg", vec3(-terrainSize * .5f, -150, -terrainSize * .5f), terrainSize, 200, terrainTriDensity);
		//terrain repeater possibly bunk

		mpTerrainManager->createTerrain(mpResourceManager, "Assets/Heightmap/heightmap_water.jpg", "Assets/Heightmap/heightmap_waterTexture.jpg", terrainSize, 200, terrainTriDensity, "repeatHM", true );
		mpTerrainManager->setHeightOfRepeatingTerrain("repeatHM", -200.0f);


		/*m_heightmap = new Heightmap(mpResourceManager, "Assets/Heightmap/heightmap_water.jpg", "Assets/Heightmap/heightmap_waterTexture.jpg", terrainSize, terrainSize, 200, terrainTriDensity, "heightMap");
		m_heightmap->setPos(vec3(-terrainSize * .5f, -150, -terrainSize * .5f));

		m_heightmap2 = new Heightmap(mpResourceManager, "Assets/Heightmap/heightmap_water.jpg", "Assets/Heightmap/heightmap_waterTexture.jpg", terrainSize, terrainSize, 200, terrainTriDensity, "heightMap2");
		m_heightmap2->setPos(vec3(-terrainSize * .5f, -150, -terrainSize * .5f - terrainSize + m_heightmap2->getOffset().x));
		*/
		m_waterMap = new Heightmap(mpResourceManager, "Assets/Water/waterTexture.jpg", waterTerrainSize, waterTerrainSize, 200, "water");
		m_waterMap->setPos(vec3(-waterTerrainSize * .5f  , 150, -waterTerrainSize * .5f));
		m_waterMap->setIsPrefab(true);
	
		Object* fishy = mpResourceManager->addNewObject("fishy", mpResourceManager->getObject("Assets/Fish")->getModelMap());
		fishy->Translate(5,5,5);
		
	}

	ResetCamera();
	
}

void Game::hookSpecialKey(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT)
	{
		mpCamera->setYaw(mpCamera->getYaw() + 0.1f);
	}

	if (key == GLUT_KEY_RIGHT)
	{
		mpCamera->setYaw(mpCamera->getYaw() - 0.1f);
	}

	if( key == GLUT_KEY_UP)
	{
		mpCamera->setPitch(mpCamera->getPitch() + 0.05f);
	}
	if( key == GLUT_KEY_DOWN)
	{
		mpCamera->setPitch(mpCamera->getPitch() - 0.05f);
	}
	if( key == GLUT_KEY_F2)
	{
		mpCamera->screenShot(width, height);
	}
	if( key == GLUT_KEY_F1)
	{
		ResetCamera();
	}
	if( key == GLUT_KEY_F3)
	{
		if(!wireframe)
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			wireframe = true;
		}
		else
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			wireframe = false;
		}
	}
	if (key == GLUT_KEY_F6)
	{
		glutFullScreen();
	}

}

void Game::hookKey(unsigned char key, int x, int y)
{
	if(key == 'w' || key == 'W')
	{
		mpCamera->command(vec3(1,0,0));
	}
	if(key == 's' || key == 'S')
	{
		mpCamera->command(vec3(-1,0,0));
	}
	if(key == 'e' || key == 'E')
	{
		mpCamera->command(vec3(0,1,0));
	}
	if(key == 'q' || key == 'Q')
	{
		mpCamera->command(vec3(0,-1,0));
	}
	if( key == 'a' || key == 'A')
	{
		mpCamera->command(vec3(0,0,-1));
	}
	if( key == 'd' || key == 'D')
	{
		mpCamera->command(vec3(0,0,1));
	}
	if(key == 27)
	{
		endGame();
	}
	if(key == 9)
	{
		mpCamera->FlyMode(!mpCamera->FlyMode());
	}
	if(key == 'g' || key == 'G')
	{
		if( fog)
		{
			fog =  false;
			glDisable(GL_FOG);
			cout << "no fog\n";
		}
		else
		{
			fog = true;
			glEnable(GL_FOG);
			cout << "yes fog\n";
		}
	}
}

void Game::endGame()
{
	IMG_Quit();

	delete mpResourceManager;
	delete mpCamera;
	delete mpTerrainManager;
	delete waterShaderManager;

	exit(0);
}








