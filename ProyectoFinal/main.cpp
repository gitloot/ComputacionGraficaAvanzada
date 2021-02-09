#define _USE_MATH_DEFINES
#include <cmath>
//glew include
#include <GL/glew.h>

//std includes
#include <string>
#include <iostream>

// contains new std::shuffle definition
#include <algorithm>
#include <random>

//glfw include
#include <GLFW/glfw3.h>

// program include
#include "Headers/TimeManager.h"

// Shader include
#include "Headers/Shader.h"

// Model geometric includes
#include "Headers/Sphere.h"
//#include "Headers/Cylinder.h"
#include "Headers/Box.h"
#include "Headers/FirstPersonCamera.h"
#include "Headers/ThirdPersonCamera.h"

//GLM include
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Headers/Texture.h"

// Include loader Model class
#include "Headers/Model.h"

// Include Terrain
#include "Headers/Terrain.h"

#include "Headers/AnimationUtils.h"

// Include Colision headers functions
#include "Headers/Colisiones.h"

// ShadowBox include
#include "Headers/ShadowBox.h"

// OpenAL include
#include <AL/alut.h>

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

int screenWidth;
int screenHeight;

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

GLFWwindow *window;

Shader shader;
//Shader con skybox
Shader shaderSkybox;
//Shader con multiples luces
Shader shaderMulLighting;
//Shader para el terreno
Shader shaderTerrain;
//Shader para las particulas de fountain
Shader shaderParticlesFountain;
//Shader para las particulas de fuego
Shader shaderParticlesFire;
//Shader para visualizar el buffer de profundidad
Shader shaderViewDepth;
//Shader para dibujar el buffer de profunidad
Shader shaderDepth;

std::shared_ptr<Camera> camera(new ThirdPersonCamera());
//Para la camara en primera persona (b)
std::shared_ptr<FirstPersonCamera> fpscamera(new FirstPersonCamera());

float distanceFromTarget = 2.0;//7.0;

//Variables adicionales

//Para el calculo de la distancia de un objetivo a un punto
float distance = 0.0f;
glm::vec3 objetivoPos;
glm::vec3 objetoPos;
glm::vec3 diferencia;

//Variables que determinan la posición y velocidad de los modelos de las naves
int fighter01Speed = 1;
int checkpointFighter01 = 0;
int checkpointFighter02 = 0;
float fighter02Speed = 5.0f;
int checkpointFighter03 = 0;
float fighter03Speed = 4.0f;
int checkpointFighter04 = 0;
float fighter04Speed = 4.5f;
int posFighter01 = 1;
int posActual = 4;
int lapFighter01 = 1;
int lapFighter02 = 1;
int lapFighter03 = 1;
int lapFighter04 = 1;

//Pathfiding de los NPC
std::vector<glm::vec3> pathNPC = {
	glm::vec3(66.015625f, 0.0f, -25.78125f),//850,380 0
	glm::vec3(61.1328125f, 0.0f, -41.40625f),//825, 300 1 
	glm::vec3(51.3671875f, 0.0f, -55.078125f),//775, 230 2
	glm::vec3(46.484375f, 0.0f, -58.0078125f),// 750, 215 3
	glm::vec3(32.8125f, 0.0f, -70.703125f), //680, 150 4 
	glm::vec3(17.1875f, 0.0f, -76.5625f), //600, 120 Salto 5
	glm::vec3(-64.84375f, 0.0f, 23.046875f),//180, 630 6
	glm::vec3(-53.125f, 0.0f, 48.4375f),//240, 760 7
	glm::vec3(-35.546875f, 0.0f, 64.0625f),//330, 840 8
	glm::vec3(-12.109375f, 0.0f, 75.78125f),//450, 900 9
	glm::vec3(17.1875f, 0.0f, 75.78125f),//600, 900 10
	glm::vec3(44.53125f, 0.0f, 60.15625f),//740, 820 11
	glm::vec3(64.0625f, 0.0f, 36.71875f),//840, 700 12
	glm::vec3(70.8984375f, 0.0f, 6.4453125f),//875, 545 13
	glm::vec3(68.9453125f, 0.0f, -20.8984375f),//865, 405 14
};

//Orientacion del modelo de tipo NPC en determinado punto
std::vector<float> orientationPathNPC = {
	5.0f,//205.0f,//155.0f,//850,380 0  
	25.0f,//220.f,//140.0f,//825, 300 1 
	40.0f,//235.0f,//125.0f,//775, 230 2  
	60.0f,//245.0f,//115.0f,// 750, 215 3  
	60.0f,//252.0f,//108.0f, //680, 150 4 
	65.0f, //600, 120 Salto 5
	185.0f,//180, 630 6
	200.0f,//240, 760 7
	230.0f,//330, 840 8
	250.0f,//450, 900 9
	275.0f,//600, 900 10
	285.0f,//740, 820 11
	315.0f,//840, 700 12
	330.0f,//875, 545 13
	5.0f//865, 405 14
};

//Para el control en el angulo de movimiento de la camara 
float angle = 0.0f;
float angleAux = 0.0f;

Sphere skyboxSphere(20, 20);
Box boxCollider;
Sphere sphereCollider(10, 10);

ShadowBox * shadowBox;

// Models complex instances
Model modelFighter01;
Model modelFighter02;
Model modelFighter03;
Model modelFighter04;
Model modelBarrier1;
Model modelPortal;
Model modelPortal2;

// Terrain model instance
Terrain terrain(-1, -1, 200, 8, "../Textures/heightmapPF.png");

GLuint textureCespedID, textureWallID, textureWindowID, textureHighwayID, textureLandingPadID;
GLuint textureTerrainBackgroundID, textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint textureParticleFountainID, textureParticleFireID, texId;
GLuint skyboxTextureID;

GLenum types[6] = {
GL_TEXTURE_CUBE_MAP_POSITIVE_X,
GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

std::string fileNames[6] = { "../Textures/skybox2/skybox2_px.jpg",
							"../Textures/skybox2/skybox2_nx.jpg",  //
							"../Textures/skybox2/skybox2_py.jpg",
							"../Textures/skybox2/skybox2_ny.jpg",
							"../Textures/skybox2/skybox2_pz.jpg",  //
							"../Textures/skybox2/skybox2_nz.jpg" };

/* "../Textures/mp_bloodvalley/blood-valley_ft.tga",
	"../Textures/mp_bloodvalley/blood-valley_bk.tga",
	"../Textures/mp_bloodvalley/blood-valley_up.tga",
	"../Textures/mp_bloodvalley/blood-valley_dn.tga",
	"../Textures/mp_bloodvalley/blood-valley_rt.tga",
	"../Textures/mp_bloodvalley/blood-valley_lf.tga" };*/

bool exitApp = false;
int lastMousePosX, offsetX = 0;
int lastMousePosY, offsetY = 0;

// Model matrix definitions
glm::mat4 modelMatrixFighter01 = glm::mat4(1.0);
glm::mat4 modelMatrixFighter02 = glm::mat4(1.0);
glm::mat4 modelMatrixFighter03 = glm::mat4(1.0);
glm::mat4 modelMatrixFighter04 = glm::mat4(1.0);
glm::mat4 modelMatrixPortal2 = glm::mat4(1.0);

// Model barrier posiciones y orientacion en puntos especificos
std::vector<glm::vec3> barrier1Position = {
//Barrera externa
glm::vec3(30.859375, 0, -82.421875), // (670,90)
glm::vec3(56.25, 0, -67.7734375), glm::vec3(70.8984375, 0, -42.3828125), glm::vec3(77.734375, 0, -28.7109375), // (800,165) (875,295) (910,365)
glm::vec3(80.0, 0, 0), glm::vec3(75.0, 0, 40.0),  glm::vec3(62.5, 0, 60.0), //(921.6,512) (896,716.8) (832,819.2)
glm::vec3(51.3671875, 0, 66.015625), glm::vec3(9.375, 0, 85.546875), glm::vec3(-10.15625, 0, 84.5703125),  //(775,850) (560, 925) (460, 950)
glm::vec3(-43.359375,0,74.8046875), glm::vec3(-62.890625,0,54.296875), glm::vec3(-74.609375, 0, 25.0),  // (295,895) (190,790) (130,640)
glm::vec3(-76.5625,0,-2.34375), glm::vec3(-73.6328125,0,-24.8046875), glm::vec3(-70.703125,0,-40.4296875), // (120, 500) (135,385) (150, 305)
glm::vec3(-52.1484375,0,-66.796875),glm::vec3(-39.453125,0,-75.5859375),  // (245, 170) (310,125)
}; 

std::vector<float> barrier1Orientation = { 
//Barrera externa
333.0f,
126.5f, 108.5f, 95.0f,
90.0f, 75.0f, 60.0f,
37.5f, 7.0f, 0.0f, 
315.0f, 295.0f, 287.0f, 
270.0f, 265.0f, 253.0f,
225.0f, 220.0f,
};

std::vector<glm::vec3> barrier1Position2 = {
//Barrera interna
glm::vec3(23.046875, 0, -62.890625), glm::vec3(40.625, 0, -50.1953125), glm::vec3(46.484375, 0, -42.3828125), //(630,190) (720, 255) (750,295)
glm::vec3(57.2265625, 0, -20.8984375), glm::vec3(62.109375, 0, -2.34375), glm::vec3(58.203125, 0, 20.703125),//(805, 405) (830,500) (810,618)
glm::vec3(47.4609375, 0, 38.671875), glm::vec3(31.8359375, 0, 56.25), glm::vec3(15.234375, 0, 65.0390625), // (755,710) (675,800) (590,845)
glm::vec3(-8.203125, 0, 66.9921875), glm::vec3(-25.78125, 0, 58.203125), glm::vec3(-42.3828125, 0, 41.6015625),// (470,855) (380,810) (295,725)
glm::vec3(-53.125, 0, 23.046875), glm::vec3(-59.9609375, 0, 2.5390625), glm::vec3(-56.0546875, 0, -16.9921875), // (240,630) (205,525) (225,425) 
glm::vec3(-51.171875, 0, -28.7109375), glm::vec3(-39.453125, 0, -47.265625), glm::vec3(-27.734375, 0, -58.984375) // (250,365) (310,270) (370,210)
};

std::vector<float> barrier1Orientation2 = {
//Barrera interna
325.0f, 315.0f, 298.5f,
282.0f, 275.0f, 72.0f,
55.5f, 42.5f, 23.0f,
175.0f, 140.0f, 127.87f,
111.5f, 95.0f, 67.5f,
59.01f, 48.8f, 226.5f
};

//Blending model sin orden (c)
std::map<std::string, glm::vec3> blendingSinOrden = {
	{"fighter01", glm::vec3(70.8984375, 0, -2.34375)},
	{"fighter02", glm::vec3(70.8984375, 0, -1.34375)},
	{"fighter03", glm::vec3(70.8984375, 0, -1.34375)},
	{"fighter04", glm::vec3(70.8984375, 0, -1.34375)},
	//{"fountain", glm::vec3(5.0, 0.0, -40.0)},
	{"fire", glm::vec3(0.0, 0.0, 7.0)}
};

double deltaTime;
double currTime, lastTime;

// Definition for the particle system
GLuint initVel, startTime;
GLuint VAOParticles;
GLuint nParticles = 8000;
double currTimeParticlesAnimation, lastTimeParticlesAnimation;

// Definition for the particle system fire
GLuint initVelFire, startTimeFire;
GLuint VAOParticlesFire;
GLuint nParticlesFire = 1000;
GLuint posBuf[2], velBuf[2], age[2];
GLuint particleArray[2];
GLuint feedback[2];
GLuint drawBuf = 1;
float particleSize = 0.5, particleLifetime = 5.0;
double currTimeParticlesAnimationFire, lastTimeParticlesAnimationFire;

//Para activar y desactivar la camara en primera persona (b)
bool enableCameraChange = false;
bool enableFirstCamera = false;
int state = 2;

// Colliders
std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> > collidersOBB;
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> > collidersSBB;

// Framesbuffers
GLuint depthMap, depthMapFBO;

/**********************
 * OpenAL config
 */

 // OpenAL Defines
#define NUM_BUFFERS 5
#define NUM_SOURCES 5
#define NUM_ENVIRONMENTS 1
// Listener
ALfloat listenerPos[] = { 0.0, 0.0, 4.0 };
ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
ALfloat listenerOri[] = { 0.0, 0.0, 1.0, 0.0, 1.0, 0.0 };
// Source 0
ALfloat source0Pos[] = { -2.0, 0.0, 0.0 };
ALfloat source0Vel[] = { 0.0, 0.0, 0.0 };
// Source 1
ALfloat source1Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source1Vel[] = { 0.0, 0.0, 0.0 };
// Source 2
ALfloat source2Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source2Vel[] = { 0.0, 0.0, 0.0 };
// Source 3
ALfloat source3Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source3Vel[] = { 0.0, 0.0, 0.0 };
// Source 4
ALfloat source4Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source4Vel[] = { 0.0, 0.0, 0.0 };
// Buffers
ALuint buffer[NUM_BUFFERS];
ALuint source[NUM_SOURCES];
ALuint environment[NUM_ENVIRONMENTS];
// Configs
ALsizei size, freq;
ALenum format;
ALvoid *data;
int ch;
ALboolean loop;
std::vector<bool> sourcesPlay = { true, true, true };

// Se definen todos las funciones.
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes);
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
	int mode);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void initParticleBuffers();
void init(int width, int height, std::string strTitle, bool bFullScreen);
void destroy();
bool processInput(bool continueApplication = true);
void prepareScene();
void prepareDepthScene();
void renderScene(bool renderParticles = true);
void determinarPos();

void initParticleBuffers() {
	// Generate the buffers
	glGenBuffers(1, &initVel);   // Initial velocity buffer
	glGenBuffers(1, &startTime); // Start time buffer

	// Allocate space for all buffers
	int size = nParticles * 3 * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, initVel);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, startTime);
	glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), NULL, GL_STATIC_DRAW);

	// Fill the first velocity buffer with random velocities
	glm::vec3 v(0.0f);
	float velocity, theta, phi;
	GLfloat *data = new GLfloat[nParticles * 3];
	for (unsigned int i = 0; i < nParticles; i++) {

		theta = glm::mix(0.0f, glm::pi<float>() / 6.0f, ((float)rand() / RAND_MAX));
		phi = glm::mix(0.0f, glm::two_pi<float>(), ((float)rand() / RAND_MAX));

		v.x = sinf(theta) * cosf(phi);
		v.y = cosf(theta);
		v.z = sinf(theta) * sinf(phi);

		velocity = glm::mix(0.6f, 0.8f, ((float)rand() / RAND_MAX));
		v = glm::normalize(v) * velocity;

		data[3 * i] = v.x;
		data[3 * i + 1] = v.y;
		data[3 * i + 2] = v.z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, initVel);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

	// Fill the start time buffer
	delete[] data;
	data = new GLfloat[nParticles];
	float time = 0.0f;
	float rate = 0.00075f;
	for (unsigned int i = 0; i < nParticles; i++) {
		data[i] = time;
		time += rate;
	}
	glBindBuffer(GL_ARRAY_BUFFER, startTime);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete[] data;

	glGenVertexArrays(1, &VAOParticles);
	glBindVertexArray(VAOParticles);
	glBindBuffer(GL_ARRAY_BUFFER, initVel);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, startTime);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void initParticleBuffersFire() {
	// Generate the buffers
	glGenBuffers(2, posBuf);    // position buffers
	glGenBuffers(2, velBuf);    // velocity buffers
	glGenBuffers(2, age);       // age buffers

	// Allocate space for all buffers
	int size = nParticlesFire * sizeof(GLfloat);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, 3 * size, 0, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, age[0]);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, age[1]);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);

	// Fill the first age buffer
	std::vector<GLfloat> initialAge(nParticlesFire);
	float rate = particleLifetime / nParticlesFire;
	for (unsigned int i = 0; i < nParticlesFire; i++) {
		int index = i - nParticlesFire;
		float result = rate * index;
		initialAge[i] = result;
	}
	// Shuffle them for better looking results
	//Random::shuffle(initialAge);
	auto rng = std::default_random_engine{};
	std::shuffle(initialAge.begin(), initialAge.end(), rng);
	glBindBuffer(GL_ARRAY_BUFFER, age[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, initialAge.data());

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create vertex arrays for each set of buffers
	glGenVertexArrays(2, particleArray);

	// Set up particle array 0
	glBindVertexArray(particleArray[0]);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, age[0]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	// Set up particle array 1
	glBindVertexArray(particleArray[1]);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, age[1]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	// Setup the feedback objects
	glGenTransformFeedbacks(2, feedback);

	// Transform feedback 0
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);

	// Transform feedback 1
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}

// Implementacion de todas las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen) {

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		exit(-1);
	}

	screenWidth = width;
	screenHeight = height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (bFullScreen)
		window = glfwCreateWindow(width, height, strTitle.c_str(),
			glfwGetPrimaryMonitor(), nullptr);
	else
		window = glfwCreateWindow(width, height, strTitle.c_str(), nullptr,
			nullptr);

	if (window == nullptr) {
		std::cerr
			<< "Error to create GLFW window, you can try download the last version of your video card that support OpenGL 3.3+"
			<< std::endl;
		destroy();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetWindowSizeCallback(window, reshapeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Failed to initialize glew" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Inicialización de los shaders
	shader.initialize("../Shaders/colorShader.vs", "../Shaders/colorShader.fs");
	shaderSkybox.initialize("../Shaders/skyBox.vs", "../Shaders/skyBox_fog.fs");
	shaderMulLighting.initialize("../Shaders/iluminacion_textura_animation_shadow.vs", "../Shaders/multipleLights_shadow.fs");
	shaderTerrain.initialize("../Shaders/terrain_shadow.vs", "../Shaders/terrain_shadow.fs");
	shaderParticlesFountain.initialize("../Shaders/particlesFountain.vs", "../Shaders/particlesFountain.fs");
	shaderParticlesFire.initialize("../Shaders/particlesFire.vs", "../Shaders/particlesFire.fs", { "Position", "Velocity", "Age" });
	shaderViewDepth.initialize("../Shaders/texturizado.vs", "../Shaders/texturizado_depth_view.fs");
	shaderDepth.initialize("../Shaders/shadow_mapping_depth.vs", "../Shaders/shadow_mapping_depth.fs");

	// Inicializacion de los objetos.
	skyboxSphere.init();
	skyboxSphere.setShader(&shaderSkybox);
	skyboxSphere.setScale(glm::vec3(20.0f, 20.0f, 20.0f));

	boxCollider.init();
	boxCollider.setShader(&shader);
	boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

	sphereCollider.init();
	sphereCollider.setShader(&shader);
	sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

	terrain.init();
	terrain.setShader(&shaderTerrain);
	terrain.setPosition(glm::vec3(100, 0, 100));

	//Fighter01
	modelFighter01.loadModel("../models/ProyFinalModels/fighter01/fighter01.fbx");//("../models/ProyFinalModels/fighter01.fbx");
	modelFighter01.setShader(&shaderMulLighting);

	//Fighter02
	modelFighter02.loadModel("../models/ProyFinalModels/fighter02/fighter02.fbx");//("../models/ProyFinalModels/fighter01.fbx");
	modelFighter02.setShader(&shaderMulLighting);

	//Fighter03
	modelFighter03.loadModel("../models/ProyFinalModels/fighter03/fighter03.fbx");//("../models/ProyFinalModels/fighter01.fbx");
	modelFighter03.setShader(&shaderMulLighting);

	//Fighter04
	modelFighter04.loadModel("../models/ProyFinalModels/fighter04/fighter04.fbx");//("../models/ProyFinalModels/fighter01.fbx");
	modelFighter04.setShader(&shaderMulLighting);

	//Barrier
	modelBarrier1.loadModel("../models/ProyFinalModels/barrier/barrier2.fbx");//("../models/ProyFinalModels/fighter01.fbx");
	modelBarrier1.setShader(&shaderMulLighting);

	//Portal
	modelPortal.loadModel("../models/ProyFinalModels/portal/portal.fbx");
	modelPortal.setShader(&shaderMulLighting);

	//Portal 2
	modelPortal2.loadModel("../models/ProyFinalModels/portal/portal.fbx");
	modelPortal2.setShader(&shaderMulLighting);

	camera->setPosition(glm::vec3(0.0, 0.0, 10.0));
	camera->setDistanceFromTarget(distanceFromTarget);
	camera->setSensitivity(1.0);
	//fps camera position (b)
	fpscamera->setPosition(glm::vec3(0.0, 3.0, 4.0));

	// Definimos el tamanio de la imagen
	int imageWidth, imageHeight;
	FIBITMAP *bitmap;
	unsigned char *data;

	// Carga de texturas para el skybox
	Texture skyboxTexture = Texture("");
	glGenTextures(1, &skyboxTextureID);
	// Tipo de textura CUBE MAP
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++) {
		skyboxTexture = Texture(fileNames[i]);
		FIBITMAP *bitmap = skyboxTexture.loadImage(true);
		unsigned char *data = skyboxTexture.convertToData(bitmap, imageWidth,
			imageHeight);
		if (data) {
			glTexImage2D(types[i], 0, GL_RGBA, imageWidth, imageHeight, 0,
				GL_BGRA, GL_UNSIGNED_BYTE, data);
		}
		else
			std::cout << "Failed to load texture" << std::endl;
		skyboxTexture.freeImage(bitmap);
	}
	/*
	// Definiendo la textura a utilizar
	Texture textureCesped("../Textures/cesped.jpg");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureCesped.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureCesped.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureCespedID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureCespedID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureCesped.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureWall("../Textures/whiteWall.jpg");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureWall.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureWall.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureWallID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureWallID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureWall.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureWindow("../Textures/ventana.png");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureWindow.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureWindow.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureWindowID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureWindowID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureWindow.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureHighway("../Textures/highway.jpg");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureHighway.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureHighway.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureHighwayID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureHighwayID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureHighway.freeImage(bitmap);
	
	// Definiendo la textura a utilizar
	Texture textureLandingPad("../Textures/landingPad.jpg");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureLandingPad.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureLandingPad.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureLandingPadID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureLandingPadID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureLandingPad.freeImage(bitmap);
	*/
	// Definiendo la textura a utilizar
	Texture textureTerrainBackground("../Textures/lava.png");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureTerrainBackground.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureTerrainBackground.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureTerrainBackgroundID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureTerrainBackgroundID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureTerrainBackground.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureTerrainR("../Textures/buttons.png");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureTerrainR.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureTerrainR.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureTerrainRID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureTerrainRID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureTerrainR.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureTerrainG("../Textures/grassFlowers.png");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureTerrainG.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureTerrainG.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureTerrainGID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureTerrainGID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureTerrainG.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureTerrainB("../Textures/mypath2.png");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureTerrainB.loadImage();
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureTerrainB.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureTerrainBID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureTerrainBID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureTerrainB.freeImage(bitmap);

	// Definiendo la textura a utilizar
	Texture textureTerrainBlendMap("../Textures/blendMapPF.png");
	// Carga el mapa de bits (FIBITMAP es el tipo de dato de la libreria)
	bitmap = textureTerrainBlendMap.loadImage(true);
	// Convertimos el mapa de bits en un arreglo unidimensional de tipo unsigned char
	data = textureTerrainBlendMap.convertToData(bitmap, imageWidth,
		imageHeight);
	// Creando la textura con id 1
	glGenTextures(1, &textureTerrainBlendMapID);
	// Enlazar esa textura a una tipo de textura de 2D.
	glBindTexture(GL_TEXTURE_2D, textureTerrainBlendMapID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Verifica si se pudo abrir la textura
	if (data) {
		// Transferis los datos de la imagen a memoria
		// Tipo de textura, Mipmaps, Formato interno de openGL, ancho, alto, Mipmaps,
		// Formato interno de la libreria de la imagen, el tipo de dato y al apuntador
		// a los datos
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		// Generan los niveles del mipmap (OpenGL es el ecargado de realizarlos)
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	/*
	Texture textureParticlesFountain("../Textures/bluewater.png");
	bitmap = textureParticlesFountain.loadImage();
	data = textureParticlesFountain.convertToData(bitmap, imageWidth, imageHeight);
	glGenTextures(1, &textureParticleFountainID);
	glBindTexture(GL_TEXTURE_2D, textureParticleFountainID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	textureParticlesFountain.freeImage(bitmap);
	*/
	Texture textureParticleFire("../Textures/fire.png");
	bitmap = textureParticleFire.loadImage();
	data = textureParticleFire.convertToData(bitmap, imageWidth, imageHeight);
	glGenTextures(1, &textureParticleFireID);
	glBindTexture(GL_TEXTURE_2D, textureParticleFireID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;
	// Libera la memoria de la textura
	textureTerrainBlendMap.freeImage(bitmap);

	std::uniform_real_distribution<float> distr01 = std::uniform_real_distribution<float>(0.0f, 1.0f);
	std::mt19937 generator;
	std::random_device rd;
	generator.seed(rd());
	int size = nParticlesFire * 2;
	std::vector<GLfloat> randData(size);
	for (int i = 0; i < randData.size(); i++) {
		randData[i] = distr01(generator);
	}

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_1D, texId);
	glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, size);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, size, GL_RED, GL_FLOAT, randData.data());
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	shaderParticlesFire.setInt("Pass", 1);
	shaderParticlesFire.setInt("ParticleTex", 0);
	shaderParticlesFire.setInt("RandomTex", 1);
	shaderParticlesFire.setFloat("ParticleLifetime", particleLifetime);
	shaderParticlesFire.setFloat("ParticleSize", particleSize);
	shaderParticlesFire.setVectorFloat3("Accel", glm::value_ptr(glm::vec3(0.0f, 0.1f, 0.0f)));
	shaderParticlesFire.setVectorFloat3("Emitter", glm::value_ptr(glm::vec3(0.0f)));

	glm::mat3 basis;
	glm::vec3 u, v, n;
	v = glm::vec3(0, 1, 0);
	n = glm::cross(glm::vec3(1, 0, 0), v);
	if (glm::length(n) < 0.00001f) {
		n = glm::cross(glm::vec3(0, 1, 0), v);
	}
	u = glm::cross(v, n);
	basis[0] = glm::normalize(u);
	basis[1] = glm::normalize(v);
	basis[2] = glm::normalize(n);
	shaderParticlesFire.setMatrix3("EmitterBasis", 1, false, glm::value_ptr(basis));

	/*******************************************
	 * Inicializacion de los buffers de la fuente
	 *******************************************/
	//initParticleBuffers();

	/*******************************************
	 * Inicializacion de los buffers del fuego
	 *******************************************/
	initParticleBuffersFire();

	/*******************************************
	 * Inicializacion del framebuffer para
	 * almacenar el buffer de profunidadad
	 *******************************************/
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*******************************************
	 * OpenAL init
	 *******************************************/
	alutInit(0, nullptr);
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
	alGetError(); // clear any error messages
	if (alGetError() != AL_NO_ERROR) {
		printf("- Error creating buffers !!\n");
		exit(1);
	}
	else {
		printf("init() - No errors yet.");
	}
	// Config source 0
	// Generate buffers, or else no sound will happen!
	alGenBuffers(NUM_BUFFERS, buffer);
	buffer[0] = alutCreateBufferFromFile("../sounds/nave1.wav");
	buffer[1] = alutCreateBufferFromFile("../sounds/fire.wav");
	buffer[2] = alutCreateBufferFromFile("../sounds/nave2.wav");
	buffer[3] = alutCreateBufferFromFile("../sounds/nave3.wav");
	buffer[4] = alutCreateBufferFromFile("../sounds/nave4.wav");
	int errorAlut = alutGetError();
	if (errorAlut != ALUT_ERROR_NO_ERROR) {
		printf("- Error open files with alut %d !!\n", errorAlut);
		exit(2);
	}


	alGetError(); /* clear error */
	alGenSources(NUM_SOURCES, source);

	if (alGetError() != AL_NO_ERROR) {
		printf("- Error creating sources !!\n");
		exit(2);
	}
	else {
		printf("init - no errors after alGenSources\n");
	}
	//FIghter01 usuario
	alSourcef(source[0], AL_PITCH, 1.0f);
	alSourcef(source[0], AL_GAIN, 3.0f);
	alSourcefv(source[0], AL_POSITION, source0Pos);
	alSourcefv(source[0], AL_VELOCITY, source0Vel);
	alSourcei(source[0], AL_BUFFER, buffer[0]);
	alSourcei(source[0], AL_LOOPING, AL_TRUE);
	alSourcef(source[0], AL_MAX_DISTANCE, 2000);
	//Fuego
	alSourcef(source[1], AL_PITCH, 1.0f);
	alSourcef(source[1], AL_GAIN, 0.3f);
	alSourcefv(source[1], AL_POSITION, source1Pos);
	alSourcefv(source[1], AL_VELOCITY, source1Vel);
	alSourcei(source[1], AL_BUFFER, buffer[1]);
	alSourcei(source[1], AL_LOOPING, AL_TRUE);
	alSourcef(source[1], AL_MAX_DISTANCE, 2000);
	//Fighter02 NPC
	alSourcef(source[2], AL_PITCH, 1.0f);
	alSourcef(source[2], AL_GAIN, 3.0f);
	alSourcefv(source[2], AL_POSITION, source2Pos);
	alSourcefv(source[2], AL_VELOCITY, source2Vel);
	alSourcei(source[2], AL_BUFFER, buffer[2]);
	alSourcei(source[2], AL_LOOPING, AL_TRUE);
	alSourcef(source[2], AL_MAX_DISTANCE, 500);
	//Fighter03 NPC
	alSourcef(source[3], AL_PITCH, 1.0f);
	alSourcef(source[3], AL_GAIN, 3.0f);
	alSourcefv(source[3], AL_POSITION, source3Pos);
	alSourcefv(source[3], AL_VELOCITY, source3Vel);
	alSourcei(source[3], AL_BUFFER, buffer[3]);
	alSourcei(source[3], AL_LOOPING, AL_TRUE);
	alSourcef(source[3], AL_MAX_DISTANCE, 2000);
	//Fighter04 NPC
	alSourcef(source[4], AL_PITCH, 1.0f);
	alSourcef(source[4], AL_GAIN, 3.0f);
	alSourcefv(source[4], AL_POSITION, source4Pos);
	alSourcefv(source[4], AL_VELOCITY, source4Vel);
	alSourcei(source[4], AL_BUFFER, buffer[4]);
	alSourcei(source[4], AL_LOOPING, AL_TRUE);
	alSourcef(source[4], AL_MAX_DISTANCE, 2000);
}

void destroy() {
	glfwDestroyWindow(window);
	glfwTerminate();
	// --------- IMPORTANTE ----------
	// Eliminar los shader y buffers creados.

	// Shaders Delete
	shader.destroy();
	shaderMulLighting.destroy();
	shaderSkybox.destroy();
	shaderTerrain.destroy();
	shaderParticlesFountain.destroy();
	shaderParticlesFire.destroy();

	// Basic objects Delete
	skyboxSphere.destroy();
	boxCollider.destroy();
	sphereCollider.destroy();

	// Terrains objects Delete
	terrain.destroy();

	// Custom objects Delete
	modelFighter01.destroy();
	modelFighter02.destroy();
	modelFighter03.destroy();
	modelFighter04.destroy();
	modelBarrier1.destroy();
	modelPortal.destroy();
	modelPortal2.destroy();

	// Textures Delete
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &textureCespedID);
	glDeleteTextures(1, &textureWallID);
	glDeleteTextures(1, &textureWindowID);
	glDeleteTextures(1, &textureHighwayID);
	glDeleteTextures(1, &textureLandingPadID);
	glDeleteTextures(1, &textureTerrainBackgroundID);
	glDeleteTextures(1, &textureTerrainRID);
	glDeleteTextures(1, &textureTerrainGID);
	glDeleteTextures(1, &textureTerrainBID);
	glDeleteTextures(1, &textureTerrainBlendMapID);
	glDeleteTextures(1, &textureParticleFountainID);
	glDeleteTextures(1, &textureParticleFireID);

	// Cube Maps Delete
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDeleteTextures(1, &skyboxTextureID);

	// Remove the buffer of the fire particles
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(2, posBuf);
	glDeleteBuffers(2, velBuf);
	glDeleteBuffers(2, age);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDeleteTransformFeedbacks(2, feedback);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAOParticlesFire);
	/*
	// Remove the buffer of the fountain particles
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &initVel);
	glDeleteBuffers(1, &startTime);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAOParticles);*/
}

void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes) {
	screenWidth = widthRes;
	screenHeight = heightRes;
	glViewport(0, 0, widthRes, heightRes);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
	int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			exitApp = true;
			break;
		}
	}
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
	offsetX = xpos - lastMousePosX;
	offsetY = ypos - lastMousePosY;
	lastMousePosX = xpos;
	lastMousePosY = ypos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	distanceFromTarget -= yoffset;
	camera->setDistanceFromTarget(distanceFromTarget);
}

void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod) {
	if (state == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		}
	}
}

bool processInput(bool continueApplication) {
	if (exitApp || glfwWindowShouldClose(window) != 0) {
		return false;
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && enableCameraChange) {
		enableCameraChange = false;
		enableFirstCamera = !enableFirstCamera;
		fpscamera->setPosition(camera->getPosition());
	}
	else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
		enableCameraChange = true;

	if (enableFirstCamera) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			fpscamera->moveFrontCamera(true, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			fpscamera->moveFrontCamera(false, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			fpscamera->moveRightCamera(false, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			fpscamera->moveRightCamera(true, deltaTime);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			fpscamera->mouseMoveCamera(offsetX, offsetY, deltaTime);
	}
	else {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			camera->mouseMoveCamera(offsetX, 0.0, deltaTime);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			camera->mouseMoveCamera(0.0, offsetY, deltaTime);
	}

	offsetX = 0;
	offsetY = 0;

	//Fighter01
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !enableFirstCamera)
	{
		state = 1;
		modelMatrixFighter01 = glm::translate(modelMatrixFighter01, glm::vec3(0.0, 0.0, 0.5 * fighter01Speed));
		modelMatrixFighter01 = glm::rotate(modelMatrixFighter01, glm::radians(1.0f), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !enableFirstCamera)
	{
		state = 0;
		modelMatrixFighter01 = glm::translate(modelMatrixFighter01, glm::vec3(0.0, 0.0, 0.5 * fighter01Speed));
		modelMatrixFighter01 = glm::rotate(modelMatrixFighter01, glm::radians(-1.0f), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !enableFirstCamera) {
		state = 2;
		modelMatrixFighter01 = glm::translate(modelMatrixFighter01, glm::vec3(0.0, 0.0, 0.75 * fighter01Speed));
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !enableFirstCamera)
	{
		state = 1;
		modelMatrixFighter01 = glm::rotate(modelMatrixFighter01, glm::radians(1.0f), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !enableFirstCamera)
	{
		state = 0;
		modelMatrixFighter01 = glm::rotate(modelMatrixFighter01, glm::radians(-1.0f), glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !enableFirstCamera)
	{
		state = 2;
		modelMatrixFighter01 = glm::translate(modelMatrixFighter01, glm::vec3(0.0, 0.0, -0.75 * fighter01Speed));
	}

	glfwPollEvents();
	return continueApplication;
}

void applicationLoop() {

	bool psi = true;

	glm::mat4 view;
	glm::vec3 axis;
	glm::vec3 target;
	float angleTarget;

	modelMatrixFighter01 = glm::translate(modelMatrixFighter01, glm::vec3(70.8984375, 0, -2.34375));
	modelMatrixFighter01 = glm::rotate(modelMatrixFighter01, glm::radians(180.0f), glm::vec3(0, 1, 0));
	modelMatrixFighter02 = glm::translate(modelMatrixFighter02, glm::vec3(70.8984375, 0, -1.34375)); //875, 505,12
	modelMatrixFighter02 = glm::rotate(modelMatrixFighter02, glm::radians(180.0f), glm::vec3(0, 1, 0));
	modelMatrixFighter03 = glm::translate(modelMatrixFighter03, glm::vec3(70.8984375, 0, -1.34375)); //875, 505,12
	modelMatrixFighter03 = glm::rotate(modelMatrixFighter03, glm::radians(180.0f), glm::vec3(0, 1, 0));
	modelMatrixFighter04 = glm::translate(modelMatrixFighter04, glm::vec3(70.8984375, 0, -1.34375)); //875, 505,12
	modelMatrixFighter04 = glm::rotate(modelMatrixFighter04, glm::radians(180.0f), glm::vec3(0, 1, 0));
	modelMatrixPortal2 = glm::rotate(modelMatrixPortal2, glm::radians(160.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrixPortal2 = glm::translate(modelMatrixPortal2, glm::vec3(45.5078125f, 0.0f, 0.0f)); //745,40

	lastTime = TimeManager::Instance().GetTime();

	// Time for the particles animation
	currTimeParticlesAnimation = lastTime;
	lastTimeParticlesAnimation = lastTime;

	currTimeParticlesAnimationFire = lastTime;
	lastTimeParticlesAnimationFire = lastTime;

	glm::vec3 lightPos = glm::vec3(10.0, 10.0, 0.0);

	shadowBox = new ShadowBox(-lightPos, camera.get(), 15.0f, 0.1f, 45.0f);

	while (psi) {
		currTime = TimeManager::Instance().GetTime();
		if (currTime - lastTime < 0.016666667) {
			glfwPollEvents();
			continue;
		}
		lastTime = currTime;
		TimeManager::Instance().CalculateFrameRate(true);
		deltaTime = TimeManager::Instance().DeltaTime;
		psi = processInput(true);

		std::map<std::string, bool> collisionDetection;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(45.0f),
			(float)screenWidth / (float)screenHeight, 0.01f, 100.0f);

		axis = glm::axis(glm::quat_cast(modelMatrixFighter01));
		angleTarget = glm::angle(glm::quat_cast(modelMatrixFighter01));
		target = modelMatrixFighter01[3];

		/***************************************
		Despues de varias pruebas se encontro una forma de solucionar el bug que impide que la camara en tercera persona
		se coloque en la posicion adecuada cuando se rota respecto al objeto. Se sabe que al menos en este caso la camara
		se mueve a una velocidad de un radian por ciclo, así es como se definio para este proyecto,
		contando los radianes que se rotan hasta que se presenta el problema, se sabe que corresponde
		con los angulos que van de 0 a -90 y de 270 a 360.
		Además si se resta el angulo previo de un ciclo anterior menos el nuevo angulo obtenido en un ciclo actual cuando
		se gira hacia la derecha la resta es menor a 0 mientras que si se rota a la izquierda el resultado es mayor a 0, esta regla
		solo aplica para los angulos de 0 a -90 y 270 a 360. Con esto en cuenta se implemento un metodo que tambien funciona
		con la implementacion de colisiones.
		Por último verificando el codigo de el archivo ThirdPersonCamera.cpp se descubrio que el causante de este bug corresponde con
		el seno del angulo y se descubrio que si el seno se calcula con el angulo negativo se evita la presencia del bug.
		****************************************/
		
		angle = angle - angleTarget;
		if (state == 1) {
			if (angle > 0) {
				angle = angleTarget;
				angleTarget = -angleTarget;
				angleAux = angleTarget;
			}
			else{
				angle = angleTarget;
				angleAux = angleTarget;
			}	
			
		}
		else{ //if(state == 0){
			if (angle < 0) {
				angle = angleTarget;
				angleTarget = -angleTarget;
				angleAux = angleTarget;
			}
			else {
				angle = angleTarget;
				angleAux = angleTarget;
			}
		}
	
		camera->setCameraTarget(target);
		camera->setAngleTarget(angleTarget);
		camera->updateCamera();
		if (enableFirstCamera == true) {
			view = fpscamera->getViewMatrix();
			fighter01Speed = 0;
			fighter02Speed = 0;
			fighter03Speed = 0;
			fighter04Speed = 0;
		}
		else {
			view = camera->getViewMatrix();
			fighter01Speed = 1;
			fighter02Speed = 5.0f;
			fighter03Speed = 4.0f;
			fighter04Speed = 4.5f;
		}

		shadowBox->update(screenWidth, screenHeight);
		glm::vec3 centerBox = shadowBox->getCenter();

		// Projection light shadow mapping
		glm::mat4 lightProjection = glm::mat4(1.0f), lightView = glm::mat4(1.0f);
		glm::mat4 lightSpaceMatrix;

		lightProjection[0][0] = 2.0f / shadowBox->getWidth();
		lightProjection[1][1] = 2.0f / shadowBox->getHeight();
		lightProjection[2][2] = -2.0f / shadowBox->getLength();
		lightProjection[3][3] = 1.0f;

		lightView = glm::lookAt(centerBox, centerBox + glm::normalize(-lightPos), glm::vec3(0.0, 1.0, 0.0));

		lightSpaceMatrix = lightProjection * lightView;
		shaderDepth.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));

		// Settea la matriz de vista y projection al shader con solo color
		shader.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shader.setMatrix4("view", 1, false, glm::value_ptr(view));

		// Settea la matriz de vista y projection al shader con skybox
		shaderSkybox.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderSkybox.setMatrix4("view", 1, false,
			glm::value_ptr(glm::mat4(glm::mat3(view))));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderMulLighting.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderMulLighting.setMatrix4("view", 1, false,
			glm::value_ptr(view));
		shaderMulLighting.setMatrix4("lightSpaceMatrix", 1, false,
			glm::value_ptr(lightSpaceMatrix));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderTerrain.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderTerrain.setMatrix4("view", 1, false,
			glm::value_ptr(view));
		shaderTerrain.setMatrix4("lightSpaceMatrix", 1, false,
			glm::value_ptr(lightSpaceMatrix));
		// Settea la matriz de vista y projection al shader para el fuego
		shaderParticlesFire.setInt("Pass", 2);
		shaderParticlesFire.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderParticlesFire.setMatrix4("view", 1, false, glm::value_ptr(view));
		// Settea la matriz de vista y projection al shader para el fountain
		/*shaderParticlesFountain.setMatrix4("projection", 1, false,
			glm::value_ptr(projection));
		shaderParticlesFountain.setMatrix4("view", 1, false,
			glm::value_ptr(view));*/

		/******************************************
		* Propiedades de la neblina
		*******************************************/
		shaderMulLighting.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.5, 0.5, 0.4)));
		shaderTerrain.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.5, 0.5, 0.4)));
		shaderSkybox.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.5, 0.5, 0.4)));
		
		/*******************************************
		 * Propiedades Luz direccional
		 *******************************************/
		shaderMulLighting.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderMulLighting.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderMulLighting.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.707106781, -0.707106781, 0.0)));

		/*******************************************
		 * Propiedades Luz direccional Terrain
		 *******************************************/
		shaderTerrain.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderTerrain.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderTerrain.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
		shaderTerrain.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
		shaderTerrain.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.707106781, -0.707106781, 0.0)));

		/*******************************************
		 * Propiedades SpotLights
		 *******************************************/
		shaderMulLighting.setInt("spotLightCount", 0);
		shaderTerrain.setInt("spotLightCount", 0);

		/*******************************************
		 * Propiedades PointLights
		 *******************************************/
		shaderMulLighting.setInt("pointLightCount", 0);
		shaderTerrain.setInt("pointLightCount", 0);

		/*******************************************
		 * 1.- We render the depth buffer
		 *******************************************/
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render scene from light's point of view
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glCullFace(GL_FRONT);
		prepareDepthScene();
		renderScene(false);
		//glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/*******************************************
		 * Debug to view the texture view map
		 *******************************************/
		 // reset viewport
		 /*glViewport(0, 0, screenWidth, screenHeight);
		 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		 // render Depth map to quad for visual debugging
		 shaderViewDepth.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
		 shaderViewDepth.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
		 glActiveTexture(GL_TEXTURE0);
		 glBindTexture(GL_TEXTURE_2D, depthMap);
		 boxViewDepth.setScale(glm::vec3(2.0, 2.0, 1.0));
		 boxViewDepth.render();*/

		 /*******************************************
		  * 2.- We render the normal objects
		  *******************************************/
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prepareScene();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		shaderMulLighting.setInt("shadowMap", 10);
		shaderTerrain.setInt("shadowMap", 10);

		

		/*******************************************
		 * Skybox
		 *******************************************/
		GLint oldCullFaceMode;
		GLint oldDepthFuncMode;
		// deshabilita el modo del recorte de caras ocultas para ver las esfera desde adentro
		glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
		glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);
		shaderSkybox.setFloat("skybox", 0);
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE0);
		skyboxSphere.render();
		glCullFace(oldCullFaceMode);
		glDepthFunc(oldDepthFuncMode);
		renderScene();

		/*******************************************
		 * Debug to box light box
		 *******************************************/
		 /*glm::vec3 front = glm::normalize(-lightPos);
		 glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), front));
		 glm::vec3 up = glm::normalize(glm::cross(front, right));
		 glDisable(GL_CULL_FACE);
		 glm::mat4 boxViewTransform = glm::mat4(1.0f);
		 boxViewTransform = glm::translate(boxViewTransform, centerBox);
		 boxViewTransform[0] = glm::vec4(right, 0.0);
		 boxViewTransform[1] = glm::vec4(up, 0.0);
		 boxViewTransform[2] = glm::vec4(front, 0.0);
		 boxViewTransform = glm::scale(boxViewTransform, glm::vec3(shadowBox->getWidth(), shadowBox->getHeight(), shadowBox->getLength()));
		 boxLightViewBox.enableWireMode();
		 boxLightViewBox.render(boxViewTransform);
		 glEnable(GL_CULL_FACE);*/

		

		/*******************************************
		 * Creacion de colliders
		 * IMPORTANT do this before interpolations
		 *******************************************/
		 // Barrier1 colliders
		for (int i = 0; i < barrier1Position.size(); i++) {
			AbstractModel::OBB barrier1Collider;
			glm::mat4 modelMatrixColliderBarrier1 = glm::mat4(1.0);
			modelMatrixColliderBarrier1 = glm::translate(modelMatrixColliderBarrier1, barrier1Position[i]);
			modelMatrixColliderBarrier1 = glm::rotate(modelMatrixColliderBarrier1, glm::radians(barrier1Orientation[i]),
				glm::vec3(0, 1, 0));
			addOrUpdateColliders(collidersOBB, "barrier1-" + std::to_string(i), barrier1Collider, modelMatrixColliderBarrier1);
			// Set the orientation of collider before doing the scale
			barrier1Collider.u = glm::quat_cast(modelMatrixColliderBarrier1);
			modelMatrixColliderBarrier1 = glm::scale(modelMatrixColliderBarrier1, glm::vec3(0.4f, 0.1f, 0.025f));
			modelMatrixColliderBarrier1 = glm::translate(modelMatrixColliderBarrier1, modelBarrier1.getObb().c);
			barrier1Collider.c = glm::vec3(modelMatrixColliderBarrier1[3]);
			barrier1Collider.e = modelBarrier1.getObb().e * (glm::vec3(0.4f, 0.1f, 0.025f));
			std::get<0>(collidersOBB.find("barrier1-" + std::to_string(i))->second) = barrier1Collider;
		}
		for (int i = 0; i < barrier1Position2.size(); i++) {
			AbstractModel::OBB barrier1Collider;
			glm::mat4 modelMatrixColliderBarrier1 = glm::mat4(1.0);
			modelMatrixColliderBarrier1 = glm::translate(modelMatrixColliderBarrier1, barrier1Position2[i]);
			modelMatrixColliderBarrier1 = glm::rotate(modelMatrixColliderBarrier1, glm::radians(barrier1Orientation2[i]),
				glm::vec3(0, 1, 0));
			addOrUpdateColliders(collidersOBB, "barrier1I-" + std::to_string(i), barrier1Collider, modelMatrixColliderBarrier1);
			// Set the orientation of collider before doing the scale
			barrier1Collider.u = glm::quat_cast(modelMatrixColliderBarrier1);
			modelMatrixColliderBarrier1 = glm::scale(modelMatrixColliderBarrier1, glm::vec3(0.2f, 0.1f, 0.025f));
			modelMatrixColliderBarrier1 = glm::translate(modelMatrixColliderBarrier1, modelBarrier1.getObb().c);
			barrier1Collider.c = glm::vec3(modelMatrixColliderBarrier1[3]);
			barrier1Collider.e = modelBarrier1.getObb().e * (glm::vec3(0.2f, 0.1f, 0.025f));
			std::get<0>(collidersOBB.find("barrier1I-" + std::to_string(i))->second) = barrier1Collider;
		}

		// Collider del fighter
		glm::mat4 modelmatrixColliderFighter01 = glm::mat4(modelMatrixFighter01);
		AbstractModel::OBB fighter01Collider;
		// Set the orientation of collider before doing the scale
		fighter01Collider.u = glm::quat_cast(modelMatrixFighter01);
		//modelmatrixColliderFighter01 = glm::rotate(modelmatrixColliderFighter01, glm::radians(270.0f), glm::vec3(1, 0, 0));
		modelmatrixColliderFighter01 = glm::scale(modelmatrixColliderFighter01, glm::vec3(0.05, 0.05, 0.05));//glm::vec3(0.15, 0.15, 0.025));
		modelmatrixColliderFighter01 = glm::translate(modelmatrixColliderFighter01,
			glm::vec3(modelFighter01.getObb().c.x,
				modelFighter01.getObb().c.y,
				modelFighter01.getObb().c.z));
		fighter01Collider.c = glm::vec3(modelmatrixColliderFighter01[3]);
		fighter01Collider.e = modelFighter01.getObb().e * glm::vec3(0.05, 0.05, 0.05);//glm::vec3(0.15, 0.15, 0.05);
		addOrUpdateColliders(collidersOBB, "fighter01", fighter01Collider, modelMatrixFighter01);

		// Collider del portal
		AbstractModel::OBB portalCollider;
		glm::mat4 modelMatrixColliderPortal = glm::mat4(1.0);
		modelMatrixColliderPortal = glm::translate(modelMatrixColliderPortal, modelPortal.getPosition());
		modelMatrixColliderPortal = glm::rotate(modelMatrixColliderPortal, glm::radians(-80.0f), glm::vec3(0, 1, 0));
		addOrUpdateColliders(collidersOBB, "portal", portalCollider, modelMatrixColliderPortal);
		// Set the orientation of collider before doing the scale
		portalCollider.u = glm::quat_cast(modelMatrixColliderPortal);
		modelMatrixColliderPortal = glm::scale(modelMatrixColliderPortal, glm::vec3(0.1f, 0.1f, 0.1f));
		modelMatrixColliderPortal = glm::translate(modelMatrixColliderPortal, modelPortal.getObb().c);
		portalCollider.c = glm::vec3(modelMatrixColliderPortal[3]);
		portalCollider.e = modelPortal.getObb().e * glm::vec3(0.1f, 0.1f, 0.1f);
		addOrUpdateColliders(collidersOBB, "portal", portalCollider, modelMatrixColliderPortal);

		// Collider del portal 2
		AbstractModel::OBB portalCollider2;
		glm::mat4 modelMatrixColliderPortal2 = glm::mat4(modelMatrixPortal2);
		// Set the orientation of collider before doing the scale
		portalCollider2.u = glm::quat_cast(modelMatrixColliderPortal2);
		modelMatrixColliderPortal2 = glm::scale(modelMatrixColliderPortal2, glm::vec3(0.1f, 0.1f, 0.1f));
		modelMatrixColliderPortal2 = glm::translate(modelMatrixColliderPortal2, modelPortal2.getObb().c);
		portalCollider2.c = glm::vec3(modelMatrixColliderPortal2[3]);
		portalCollider2.e = modelPortal2.getObb().e * glm::vec3(0.1f, 0.1f, 0.1f);
		addOrUpdateColliders(collidersOBB, "portal2", portalCollider2, modelMatrixColliderPortal2);

		 /*******************************************
		  * Render de colliders
		  *******************************************/
		for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator it =
			collidersOBB.begin(); it != collidersOBB.end(); it++) {
			glm::mat4 matrixCollider = glm::mat4(1.0);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(it->second).c);
			matrixCollider = matrixCollider * glm::mat4(std::get<0>(it->second).u);
			matrixCollider = glm::scale(matrixCollider, std::get<0>(it->second).e * 2.0f);
			boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			boxCollider.enableWireMode();
			boxCollider.render(matrixCollider);
		}

		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it =
			collidersSBB.begin(); it != collidersSBB.end(); it++) {
			glm::mat4 matrixCollider = glm::mat4(1.0);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(it->second).c);
			matrixCollider = glm::scale(matrixCollider, glm::vec3(std::get<0>(it->second).ratio * 2.0f));
			sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			sphereCollider.enableWireMode();
			sphereCollider.render(matrixCollider);
		}

		// Esto es para ilustrar la transformacion inversa de los coliders
		/*glm::vec3 cinv = glm::inverse(mayowCollider.u) * glm::vec4(rockCollider.c, 1.0);
		glm::mat4 invColliderS = glm::mat4(1.0);
		invColliderS = glm::translate(invColliderS, cinv);
		invColliderS =  invColliderS * glm::mat4(mayowCollider.u);
		invColliderS = glm::scale(invColliderS, glm::vec3(rockCollider.ratio * 2.0, rockCollider.ratio * 2.0, rockCollider.ratio * 2.0));
		sphereCollider.setColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
		sphereCollider.enableWireMode();
		sphereCollider.render(invColliderS);
		glm::vec3 cinv2 = glm::inverse(mayowCollider.u) * glm::vec4(mayowCollider.c, 1.0);
		glm::mat4 invColliderB = glm::mat4(1.0);
		invColliderB = glm::translate(invColliderB, cinv2);
		invColliderB = glm::scale(invColliderB, mayowCollider.e * 2.0f);
		boxCollider.setColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
		boxCollider.enableWireMode();
		boxCollider.render(invColliderB);
		// Se regresa el color blanco
		sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
		boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));*/

		/*******************************************
		 * Test Colisions
		 *******************************************/
		for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4,
			glm::mat4> >::iterator it = collidersOBB.begin(); it != collidersOBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4,
				glm::mat4> >::iterator jt = collidersOBB.begin(); jt != collidersOBB.end(); jt++) {
				if (it != jt && testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Colision " << it->first << " with " << jt->first << std::endl;
					isCollision = true;
				}
			}
			for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4,
				glm::mat4> >::iterator jt = collidersSBB.begin(); jt != collidersSBB.end(); jt++) {
				if (testSphereOBox(std::get<0>(jt->second), std::get<0>(it->second))) {
					//std::cout << "Colision " << it->first << " with " << jt->first << std::endl;
					//std::cout << "Colision " << jt->first << " with " << it->first << std::endl;
					isCollision = true;
					addOrUpdateCollisionDetection(collisionDetection, jt->first, true);
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4,
			glm::mat4> >::iterator it = collidersSBB.begin(); it != collidersSBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4,
				glm::mat4> >::iterator jt = collidersSBB.begin(); jt != collidersSBB.end(); jt++) {
				if (it != jt && testSphereSphereIntersection(std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Colision " << it->first << " with " << jt->first << std::endl;
					isCollision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}


		for (std::map<std::string,
			std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it =
			collidersSBB.begin(); it != collidersSBB.end(); it++) {
			bool isCollision = false;
			std::map<std::string,
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator jt =
				collidersOBB.begin();
			for (; jt != collidersOBB.end(); jt++) {
				if (testSphereOBox(std::get<0>(it->second),
					std::get<0>(jt->second))) {
					//std::cout << "Colision " << it->first << " with "<< jt->first << std::endl;
					isCollision = true;
					addOrUpdateCollisionDetection(collisionDetection, jt->first, isCollision);
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		std::map<std::string, bool>::iterator colIt;
		for (colIt = collisionDetection.begin(); colIt != collisionDetection.end();
			colIt++) {
			std::map<std::string,
				std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it =
				collidersSBB.find(colIt->first);
			std::map<std::string,
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator jt =
				collidersOBB.find(colIt->first);
			if (it != collidersSBB.end()) {
				if (!colIt->second)
					addOrUpdateColliders(collidersSBB, it->first);
			}
			if (jt != collidersOBB.end()) {
				if (!colIt->second) {
					addOrUpdateColliders(collidersOBB, jt->first);
				}
				else {
					if (jt->first.compare("fighter01") == 0) {
							modelMatrixFighter01 = std::get<1>(jt->second);
						}
					if (jt->first.compare("portal") == 0) {
						modelMatrixFighter01 = glm::mat4(1.0);
						modelMatrixFighter01 = glm::translate(modelMatrixFighter01, glm::vec3(-67.7734375f, 0.0, 0.0f));
						modelMatrixFighter01[3][1] = terrain.getHeightTerrain(modelMatrixFighter01[3][0], modelMatrixFighter01[3][2]) + 1.0f;
					}
				}
			}
		}

		/******************************
		* Rutina de los NPC y calculo de posicion
		******************************/
		
		//Fighter01
		objetivoPos = pathNPC[checkpointFighter01]; 
		objetoPos = modelMatrixFighter01[3];
		distance = (objetoPos.x - objetivoPos.x) * (objetoPos.x - objetivoPos.x) + (objetoPos.z - objetivoPos.z) * (objetoPos.z - objetivoPos.z);
		if (distance < 50.0f) {
			checkpointFighter01++;
			if (checkpointFighter01 > pathNPC.size() - 1) {
				lapFighter01++;
				checkpointFighter01 = 0;
			}
		}
		else {
			distance = (-objetoPos.x - objetivoPos.x) * (-objetoPos.x - objetivoPos.x) + (-objetoPos.z - objetivoPos.z) * (-objetoPos.z - objetivoPos.z);
			if (distance < 50.0f) {
				checkpointFighter01++;
				if (checkpointFighter01 > pathNPC.size() - 1) {
					checkpointFighter01 = 0;
					lapFighter01++;
				}
			}
		}

		determinarPos();

		if (posFighter01 != posActual) {
			std::cout << "EL JUGADOR SE ENCUETRA EN LA POSICION: " << posFighter01 << std::endl;
			posActual = posFighter01;
		}

		//Fighter02
		objetivoPos = pathNPC[checkpointFighter02];
		objetoPos = modelMatrixFighter02[3];
		distance = (objetoPos.x - objetivoPos.x) * (objetoPos.x - objetivoPos.x) + (objetoPos.z - objetivoPos.z) * (objetoPos.z - objetivoPos.z);
		if (distance < 1.0f) {
			if (checkpointFighter02 == 5) {
				modelMatrixFighter02 = glm::mat4(1.0);
				modelMatrixFighter02 = glm::translate(modelMatrixFighter02, glm::vec3(-67.7734375f, 0.0, 0.0f)); //165, 512
				modelMatrixFighter02 = glm::rotate(modelMatrixFighter02, glm::radians(180.0f), glm::vec3(0, 1, 0));
				modelMatrixFighter02[3][1] = terrain.getHeightTerrain(modelMatrixFighter02[3][0], modelMatrixFighter02[3][2]) + 1.0f;
			}
			checkpointFighter02++;
			if (checkpointFighter02 > pathNPC.size() - 1) {
				lapFighter02++;
				checkpointFighter02 = 0;
			}
		}
		else {
			diferencia = objetoPos - objetivoPos;
			diferencia = diferencia / distance;
			modelMatrixFighter02 = glm::translate(modelMatrixFighter02, diferencia * fighter02Speed);
			modelMatrixFighter02[3][1] = terrain.getHeightTerrain(modelMatrixFighter02[3][0], modelMatrixFighter02[3][2]) + 1.0f;
		}
		//Fighter03
		objetivoPos = pathNPC[checkpointFighter03];
		objetoPos = modelMatrixFighter03[3];
		distance = (objetoPos.x - objetivoPos.x) * (objetoPos.x - objetivoPos.x) + (objetoPos.z - objetivoPos.z) * (objetoPos.z - objetivoPos.z);
		if (distance < 1.0f) {
			if (checkpointFighter03 == 5) {
				modelMatrixFighter03 = glm::mat4(1.0);
				modelMatrixFighter03 = glm::translate(modelMatrixFighter03, glm::vec3(-67.7734375f, 0.0, 0.0f)); //165, 512
				modelMatrixFighter03 = glm::rotate(modelMatrixFighter03, glm::radians(180.0f), glm::vec3(0, 1, 0));
				modelMatrixFighter03[3][1] = terrain.getHeightTerrain(modelMatrixFighter03[3][0], modelMatrixFighter03[3][2]) + 1.0f;
			}
			checkpointFighter03++;
			if (checkpointFighter03 > pathNPC.size() - 1) {
				lapFighter03++;
				checkpointFighter03 = 0;
			}
		}
		else {
			diferencia = objetoPos - objetivoPos;
			diferencia = diferencia / distance;
			modelMatrixFighter03 = glm::translate(modelMatrixFighter03, diferencia * fighter03Speed);
			modelMatrixFighter03[3][1] = terrain.getHeightTerrain(modelMatrixFighter03[3][0], modelMatrixFighter03[3][2]) + 1.0f;
		}
		//Fighter04
		objetivoPos = pathNPC[checkpointFighter04];
		objetoPos = modelMatrixFighter04[3];
		distance = (objetoPos.x - objetivoPos.x) * (objetoPos.x - objetivoPos.x) + (objetoPos.z - objetivoPos.z) * (objetoPos.z - objetivoPos.z);
		if (distance < 1.0f) {
			if (checkpointFighter04 == 5) {
				modelMatrixFighter04 = glm::mat4(1.0);
				modelMatrixFighter04 = glm::translate(modelMatrixFighter04, glm::vec3(-67.7734375f, 0.0, 0.0f)); //165, 512
				modelMatrixFighter04 = glm::rotate(modelMatrixFighter04, glm::radians(180.0f), glm::vec3(0, 1, 0));
				modelMatrixFighter04[3][1] = terrain.getHeightTerrain(modelMatrixFighter04[3][0], modelMatrixFighter04[3][2]) + 1.0f;
			}
			checkpointFighter04++;
			if (checkpointFighter04 > pathNPC.size() - 1) {
				lapFighter04++;
				checkpointFighter04 = 0;
			}
		}
		else {
			diferencia = objetoPos - objetivoPos;
			diferencia = diferencia / distance;
			modelMatrixFighter04 = glm::translate(modelMatrixFighter04, diferencia * fighter04Speed);
			modelMatrixFighter04[3][1] = terrain.getHeightTerrain(modelMatrixFighter04[3][0], modelMatrixFighter04[3][2]) + 1.0f;
		}

		glfwSwapBuffers(window);

		/****************************+
		 * Open AL sound data
		 ****************************/
		source0Pos[0] = modelMatrixFighter01[3].x;
		source0Pos[1] = modelMatrixFighter01[3].y;
		source0Pos[2] = modelMatrixFighter01[3].z;
		alSourcefv(source[0], AL_POSITION, source0Pos);

		source2Pos[0] = modelMatrixFighter02[3].x;
		source2Pos[1] = modelMatrixFighter02[3].y;
		source2Pos[2] = modelMatrixFighter02[3].z;
		alSourcefv(source[2], AL_POSITION, source2Pos);

		source3Pos[0] = modelMatrixFighter03[3].x;
		source3Pos[1] = modelMatrixFighter03[3].y;
		source3Pos[2] = modelMatrixFighter03[3].z;
		alSourcefv(source[3], AL_POSITION, source3Pos);

		source4Pos[0] = modelMatrixFighter04[3].x;
		source4Pos[1] = modelMatrixFighter04[3].y;
		source4Pos[2] = modelMatrixFighter04[3].z;
		alSourcefv(source[4], AL_POSITION, source4Pos);

		// Listener for the Thris person camera
		listenerPos[0] = modelMatrixFighter01[3].x;
		listenerPos[1] = modelMatrixFighter01[3].y;
		listenerPos[2] = modelMatrixFighter01[3].z;
		alListenerfv(AL_POSITION, listenerPos);

		glm::vec3 upModel = glm::normalize(modelMatrixFighter01[1]);
		glm::vec3 frontModel = glm::normalize(modelMatrixFighter01[2]);

		listenerOri[0] = frontModel.x;
		listenerOri[1] = frontModel.y;
		listenerOri[2] = frontModel.z;
		listenerOri[3] = upModel.x;
		listenerOri[4] = upModel.y;
		listenerOri[5] = upModel.z;

		// Listener for the First person camera
		/*listenerPos[0] = camera->getPosition().x;
		listenerPos[1] = camera->getPosition().y;
		listenerPos[2] = camera->getPosition().z;
		alListenerfv(AL_POSITION, listenerPos);
		listenerOri[0] = camera->getFront().x;
		listenerOri[1] = camera->getFront().y;
		listenerOri[2] = camera->getFront().z;
		listenerOri[3] = camera->getUp().x;
		listenerOri[4] = camera->getUp().y;
		listenerOri[5] = camera->getUp().z;*/
		alListenerfv(AL_ORIENTATION, listenerOri);

		for (unsigned int i = 0; i < sourcesPlay.size(); i++) {
			if (sourcesPlay[i]) {
				sourcesPlay[i] = false;
				alSourcePlay(source[i]);
			}
		}
	}
}

void prepareScene() {
	skyboxSphere.setShader(&shaderSkybox);

	terrain.setShader(&shaderTerrain);

	modelFighter01.setShader(&shaderMulLighting);

	modelFighter02.setShader(&shaderMulLighting);

	modelFighter03.setShader(&shaderMulLighting);

	modelFighter04.setShader(&shaderMulLighting);
}

void prepareDepthScene() {
	skyboxSphere.setShader(&shaderDepth);

	terrain.setShader(&shaderDepth);

	modelFighter01.setShader(&shaderDepth);

	modelFighter02.setShader(&shaderDepth);

	modelFighter03.setShader(&shaderDepth);

	modelFighter04.setShader(&shaderDepth);
}

void renderScene(bool renderParticles) {
	/*******************************************
	* Terrain Cesped
	*******************************************/
	glm::mat4 modelCesped = glm::mat4(1.0);
	modelCesped = glm::translate(modelCesped, glm::vec3(0.0, 0.0, 0.0));
	modelCesped = glm::scale(modelCesped, glm::vec3(200.0, 0.001, 200.0));
	// Se activa la textura del background
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBackgroundID);
	shaderTerrain.setInt("backgroundTexture", 0);
	// Se activa la textura de tierra
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureTerrainRID);
	shaderTerrain.setInt("rTexture", 1);
	// Se activa la textura de hierba
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureTerrainGID);
	shaderTerrain.setInt("gTexture", 2);
	// Se activa la textura del camino
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBID);
	shaderTerrain.setInt("bTexture", 3);
	// Se activa la textura del blend map
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBlendMapID);
	shaderTerrain.setInt("blendMapTexture", 4);
	shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(40, 40)));
	terrain.render();
	shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(0, 0)));
	glBindTexture(GL_TEXTURE_2D, 0);
	
	/*******************************************
	 * Custom objects obj
	 *******************************************/

	 // Render muros de contencion de la pista
	for (int i = 0; i < barrier1Position.size(); i++) {
		barrier1Position[i].y = terrain.getHeightTerrain(barrier1Position[i].x, barrier1Position[i].z);
		modelBarrier1.setPosition(barrier1Position[i]);
		modelBarrier1.setScale(glm::vec3(0.4f, 0.1f, 0.025f));
		modelBarrier1.setOrientation(glm::vec3(0.0f, barrier1Orientation[i], 0.0f));
		modelBarrier1.render();
	}
	for (int i = 0; i < barrier1Position2.size(); i++) {
		barrier1Position2[i].y = terrain.getHeightTerrain(barrier1Position2[i].x, barrier1Position2[i].z);
		modelBarrier1.setPosition(barrier1Position2[i]);
		modelBarrier1.setScale(glm::vec3(0.2f, 0.1f, 0.025f));
		modelBarrier1.setOrientation(glm::vec3(0.0f, barrier1Orientation2[i], 0.0f));
		modelBarrier1.render();
	}

	//Render portal
	float portalY = terrain.getHeightTerrain(modelPortal.getPosition().x, modelPortal.getPosition().z) + 30.0f;
	modelPortal.setPosition(glm::vec3(5.0f, portalY, -107.5f)); //540,40
	modelPortal.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
	modelPortal.setOrientation(glm::vec3(0.0f, -80.0f, 0.0f));
	modelPortal.render();

	//Render portal2
	modelMatrixPortal2[3][1] = terrain.getHeightTerrain(modelMatrixPortal2[3][0], modelMatrixPortal2[3][1]) + 30.0f;
	glm::mat4 modelMatrixPortal2Core = glm::mat4(modelMatrixPortal2);
	modelMatrixPortal2Core = glm::scale(modelMatrixPortal2Core, glm::vec3(0.1f, 0.1f, 0.1f));
	//modelPortal2.setScale(glm::vec3(0.1f, 0.1f, 0.1f));
	//portalY = terrain.getHeightTerrain(-67.7734375f, 10.46875f) + 30.0f;
	//modelPortal2.setPosition(glm::vec3(-67.7734375f, portalY, 10.46875f));//0.0f)); //540,40
	//modelPortal2.setOrientation(glm::vec3(0.0f, 160.0f, 0.0f));
	modelPortal2.render(modelMatrixPortal2Core);

	/**********************
	* Importante se actualiza la posicion de los objetos con transparencia
	***********************/
		//Se actualiza el fighter
	blendingSinOrden.find("fighter01")->second = glm::vec3(modelMatrixFighter01[3]);
	blendingSinOrden.find("fighter02")->second = glm::vec3(modelMatrixFighter02[3]);
	blendingSinOrden.find("fighter03")->second = glm::vec3(modelMatrixFighter03[3]);
	blendingSinOrden.find("fighter04")->second = glm::vec3(modelMatrixFighter04[3]);

	/*********************
	* Se ordena los objetos con el canal alfa
	**********************/
	std::map<float, std::pair<std::string, glm::vec3>> blendingOrdenado;
	std::map<std::string, glm::vec3>::iterator itBlend;
	for (itBlend = blendingSinOrden.begin(); itBlend != blendingSinOrden.end(); itBlend++) {
		float distanceFromView = glm::length(camera->getPosition() - itBlend->second);
		blendingOrdenado[distanceFromView] = std::make_pair(itBlend->first, itBlend->second);
	}

	/***********************
	* Render de las transparencias
	************************/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	for (std::map<float, std::pair<std::string, glm::vec3>>::reverse_iterator it = blendingOrdenado.rbegin();
		it != blendingOrdenado.rend(); it++) {
		if (it->second.first.compare("fighter01") == 0) {
			// Render for the fighter model
			modelMatrixFighter01[3][1] = terrain.getHeightTerrain(modelMatrixFighter01[3][0], modelMatrixFighter01[3][2]) + 1.0f;
			glm::mat4 modelMatrixFighter01Chasis = glm::mat4(modelMatrixFighter01);
			//modelMatrixFighter01Chasis = glm::rotate(modelMatrixFighter01Chasis, glm::radians(270.0f), glm::vec3(1, 0, 0));
			modelMatrixFighter01Chasis = glm::scale(modelMatrixFighter01Chasis, glm::vec3(0.05, 0.05, 0.05));//glm::vec3(0.15, 0.15, 0.025));//
			modelFighter01.setAnimationIndex(0);
			modelFighter01.render(modelMatrixFighter01Chasis);

		}
		if (it->second.first.compare("fighter02") == 0) {
			// Render for the fighter model
			modelMatrixFighter02[3][1] = terrain.getHeightTerrain(modelMatrixFighter02[3][0], modelMatrixFighter02[3][2]) + 1.0f;
			glm::mat4 modelMatrixFighter02Chasis = glm::mat4(modelMatrixFighter02);
			//modelMatrixFighter02Chasis = glm::rotate(modelMatrixFighter02Chasis, glm::radians(270.0f), glm::vec3(1, 0, 0));
			modelMatrixFighter02Chasis = glm::rotate(modelMatrixFighter02Chasis, glm::radians(orientationPathNPC[checkpointFighter02]), glm::vec3(0, 1, 0));
			modelMatrixFighter02Chasis = glm::scale(modelMatrixFighter02Chasis, glm::vec3(0.05, 0.05, 0.05)); //glm::vec3(0.15, 0.15, 0.025));//
			modelFighter02.setAnimationIndex(0);
			modelFighter02.render(modelMatrixFighter02Chasis);

		}
		if (it->second.first.compare("fighter03") == 0) {
			// Render for the fighter model
			modelMatrixFighter03[3][1] = terrain.getHeightTerrain(modelMatrixFighter03[3][0], modelMatrixFighter03[3][2]) + 1.0f;
			glm::mat4 modelMatrixFighter03Chasis = glm::mat4(modelMatrixFighter03);
			//modelMatrixFighter02Chasis = glm::rotate(modelMatrixFighter02Chasis, glm::radians(270.0f), glm::vec3(1, 0, 0));
			modelMatrixFighter03Chasis = glm::rotate(modelMatrixFighter03Chasis, glm::radians(orientationPathNPC[checkpointFighter03]), glm::vec3(0, 1, 0));
			modelMatrixFighter03Chasis = glm::scale(modelMatrixFighter03Chasis, glm::vec3(0.05, 0.05, 0.05)); //glm::vec3(0.15, 0.15, 0.025));//
			modelFighter03.setAnimationIndex(0);
			modelFighter03.render(modelMatrixFighter03Chasis);

		}
		if (it->second.first.compare("fighter04") == 0) {
			// Render for the fighter model
			modelMatrixFighter04[3][1] = terrain.getHeightTerrain(modelMatrixFighter04[3][0], modelMatrixFighter04[3][2]) + 1.0f;
			glm::mat4 modelMatrixFighter04Chasis = glm::mat4(modelMatrixFighter04);
			//modelMatrixFighter02Chasis = glm::rotate(modelMatrixFighter02Chasis, glm::radians(270.0f), glm::vec3(1, 0, 0));
			modelMatrixFighter04Chasis = glm::rotate(modelMatrixFighter04Chasis, glm::radians(orientationPathNPC[checkpointFighter04]), glm::vec3(0, 1, 0));
			modelMatrixFighter04Chasis = glm::scale(modelMatrixFighter04Chasis, glm::vec3(0.05, 0.05, 0.05)); //glm::vec3(0.15, 0.15, 0.025));//
			modelFighter04.setAnimationIndex(0);
			modelFighter04.render(modelMatrixFighter04Chasis);

		}
		/*
		else if (renderParticles && it->second.first.compare("fountain") == 0) {
			/**********
			 * Init Render particles systems
			 *
			glm::mat4 modelMatrixParticlesFountain = glm::mat4(1.0);
			modelMatrixParticlesFountain = glm::translate(modelMatrixParticlesFountain, glm::vec3(5.0f, 0.0f, -80.46875f));//it->second.second);
			modelMatrixParticlesFountain[3][1] = terrain.getHeightTerrain(modelMatrixParticlesFountain[3][0], modelMatrixParticlesFountain[3][2]) + 0.36 * 10.0;
			modelMatrixParticlesFountain = glm::scale(modelMatrixParticlesFountain, glm::vec3(3.0, 3.0, 3.0));
			currTimeParticlesAnimation = TimeManager::Instance().GetTime();
			if (currTimeParticlesAnimation - lastTimeParticlesAnimation > 10.0)
				lastTimeParticlesAnimation = currTimeParticlesAnimation;
			//glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			// Set the point size
			glPointSize(10.0f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureParticleFountainID);
			shaderParticlesFountain.turnOn();
			shaderParticlesFountain.setFloat("Time", float(currTimeParticlesAnimation - lastTimeParticlesAnimation));
			shaderParticlesFountain.setFloat("ParticleLifetime", 3.5f);
			shaderParticlesFountain.setInt("ParticleTex", 0);
			shaderParticlesFountain.setVectorFloat3("Gravity", glm::value_ptr(glm::vec3(0.0f, -0.3f, 0.0f)));
			shaderParticlesFountain.setMatrix4("model", 1, false, glm::value_ptr(modelMatrixParticlesFountain));
			glBindVertexArray(VAOParticles);
			glDrawArrays(GL_POINTS, 0, nParticles);
			glDepthMask(GL_TRUE);
			//glEnable(GL_DEPTH_TEST);
			shaderParticlesFountain.turnOff();
			/**********
			 * End Render particles systems
			 *
		}*/
		else if (renderParticles && it->second.first.compare("fire") == 0) {
			/**********
			 * Init Render particles systems
			 */
			lastTimeParticlesAnimationFire = currTimeParticlesAnimationFire;
			currTimeParticlesAnimationFire = TimeManager::Instance().GetTime();

			shaderParticlesFire.setInt("Pass", 1);
			shaderParticlesFire.setFloat("Time", currTimeParticlesAnimationFire);
			shaderParticlesFire.setFloat("DeltaT", currTimeParticlesAnimationFire - lastTimeParticlesAnimationFire);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D, texId);
			glEnable(GL_RASTERIZER_DISCARD);
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
			glBeginTransformFeedback(GL_POINTS);
			glBindVertexArray(particleArray[1 - drawBuf]);
			glVertexAttribDivisor(0, 0);
			glVertexAttribDivisor(1, 0);
			glVertexAttribDivisor(2, 0);
			glDrawArrays(GL_POINTS, 0, nParticlesFire);
			glEndTransformFeedback();
			glDisable(GL_RASTERIZER_DISCARD);

			shaderParticlesFire.setInt("Pass", 2);
			glm::mat4 modelFireParticles = glm::mat4(1.0);
			modelFireParticles = glm::translate(modelFireParticles, glm::vec3(15.234375f, 0.0f, -76.5625f));//it->second.second);//590,120
			modelFireParticles[3][1] = terrain.getHeightTerrain(modelFireParticles[3][0], modelFireParticles[3][2]);
			modelFireParticles = glm::rotate(modelFireParticles, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
			shaderParticlesFire.setMatrix4("model", 1, false, glm::value_ptr(modelFireParticles));

			shaderParticlesFire.turnOn();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureParticleFireID);
			glDepthMask(GL_FALSE);
			glBindVertexArray(particleArray[drawBuf]);
			glVertexAttribDivisor(0, 1);
			glVertexAttribDivisor(1, 1);
			glVertexAttribDivisor(2, 1);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticlesFire);
			glBindVertexArray(0);
			glDepthMask(GL_TRUE);
			drawBuf = 1 - drawBuf;
			shaderParticlesFire.turnOff();

			/****************************+
			 * Open AL sound data
			 */
			source1Pos[0] = modelFireParticles[3].x;
			source1Pos[1] = modelFireParticles[3].y;
			source1Pos[2] = modelFireParticles[3].z;
			alSourcefv(source[1], AL_POSITION, source1Pos);

			/**********
			 * End Render particles systems
			 */
		}
	}
	glEnable(GL_CULL_FACE);
	//glDisable(GL_BLEND);
}

void determinarPos() {
	
	if (lapFighter01 > lapFighter02) {
		if (lapFighter01 > lapFighter03) {
			/****************
			P1 > P2 P3 P4
			*****************/
			if (lapFighter01 > lapFighter04) {
				posFighter01 = 1;
			}
			else {
				/****************
				P1 > P2 P3 P4
				*****************/
				if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
					posFighter01 = 1;
				}
				/****************
				P4 > P1 > P2 P3
				*****************/
				else{
					posFighter01 = 2;
				}
			}
		}
		else {
			if (lapFighter03 == lapFighter03 && checkpointFighter01 > checkpointFighter03) {
				/****************
			    P1 > P2 P3 P4
			    *****************/
				if (lapFighter01 > lapFighter04) {
					posFighter01 = 1;
				}
				else {
					/****************
					P1 > P2 P3 P4
					*****************/
					if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
						posFighter01 = 1;
					}
					/****************
					P4 > P1 > P2 P3
					*****************/
					else {
						posFighter01 = 2;
					}
				}

			}
			else {
				/****************
			    P3 > P1 > P2 P4
			    *****************/
				if (lapFighter01 > lapFighter04) {
					posFighter01 = 2;
				}
				else {
					/****************
					P3 > P1 > P2 P4
					*****************/
					if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
						posFighter01 = 2;
					}
					/****************
					P4 P3 > P1 > P2
					*****************/
					else {
						posFighter01 = 3;
					}
				}
			}
		}
	}
	else {
		if (lapFighter01 == lapFighter02 && checkpointFighter01 > checkpointFighter02) {
			if (lapFighter01 > lapFighter03) {
				/****************
				P1 > P2 P3 P4
				*****************/
				if (lapFighter01 > lapFighter04) {
					posFighter01 = 1;
				}
				else {
					/****************
					P1 > P2 P3 P4
					*****************/
					if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
						posFighter01 = 1;
					}
					/****************
					P4 > P1 > P2 P3
					*****************/
					else {
						posFighter01 = 2;
					}
				}
			}
			else {
				if (lapFighter03 == lapFighter03 && checkpointFighter01 > checkpointFighter03) {
					/****************
					P1 > P2 P3 P4
					*****************/
					if (lapFighter01 > lapFighter04) {
						posFighter01 = 1;
					}
					else {
						/****************
						P1 > P2 P3 P4
						*****************/
						if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
							posFighter01 = 1;
						}
						/****************
						P4 > P1 > P2 P3
						*****************/
						else {
							posFighter01 = 2;
						}
					}

				}
				else {
					/****************
					P3 > P1 > P2 P4
					*****************/
					if (lapFighter01 > lapFighter04) {
						posFighter01 = 2;
					}
					else {
						/****************
						P3 > P1 > P2 P4
						*****************/
						if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
							posFighter01 = 2;
						}
						/****************
						P4 P3 > P1 > P2
						*****************/
						else {
							posFighter01 = 3;
						}
					}
				}
			}
		}
		else {
			if (lapFighter01 > lapFighter03) {
				/****************
				P2 > P1 > P3 P4
				*****************/
				if (lapFighter01 > lapFighter04) {
					posFighter01 = 2;
				}
				else {
					/****************
					P2 > P1 > P3 P4
					*****************/
					if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
						posFighter01 = 2;
					}
					/****************
					P2 P4 > P1 > P3
					*****************/
					else {
						posFighter01 = 3;
					}
				}
			}
			else {
				if (lapFighter03 == lapFighter03 && checkpointFighter01 > checkpointFighter03) {
					/****************
					P2 > P1 > P3 P4
					*****************/
					if (lapFighter01 > lapFighter04) {
						posFighter01 = 2;
					}
					else {
						/****************
						P2 > P1 > P3 P4
						*****************/
						if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
							posFighter01 = 2;
						}
						/****************
						P4 P2 > P1 > P3
						*****************/
						else {
							posFighter01 = 3;
						}
					}

				}
				else {
					/****************
					P3 P2 > P1 > P4
					*****************/
					if (lapFighter01 > lapFighter04) {
						posFighter01 = 3;
					}
					else {
						/****************
						P3 P2 > P1 > P4
						*****************/
						if (lapFighter01 == lapFighter04 && checkpointFighter01 > checkpointFighter04) {
							posFighter01 = 3;
						}
						/****************
						P4 P3 P2 > P1 
						*****************/
						else {
							posFighter01 = 4;
						}
					}
				}
			}
		}
	}
}

int main(int argc, char **argv) {
	init(800, 700, "Window GLFW", false);
	applicationLoop();
	destroy();
	return 1;
}