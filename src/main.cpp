/*
 * Program 4 example with diffuse and spline camera PRESS 'g'
 * CSC 471 Cal Poly Z. Wood + S. Sueda + I. Dunn (spline D. McGirr)
 */

#include <iostream>
#include <Windows.h>
#include <chrono>
#include <glad/glad.h>
extern "C++" {
#include <../bass24/c/bass.h>
}

#include "mmsystem.h"
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include "Bezier.h"
#include "Boid.h"
#include "Spline.h"

#pragma comment(lib, "winmm.lib")

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>


// value_ptr for glm
#include <../glm/gtc/type_ptr.hpp>
#include <../glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong
	std::shared_ptr<Program> prog;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	//our geometry
	shared_ptr<Shape> sphere;

	shared_ptr<Shape> theBunny;

	shared_ptr<Shape> floorshapes;
	shared_ptr<Shape> doorshapes;

	vector<shared_ptr<Shape>> floor;
	vector<shared_ptr<Shape>> door;

	list<Boid*> boids;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> texture4;	

	//global data (larger program should be encapsulated)
	vec3 gMin;
	float gRot = 0;
	float gCamH = 0;
	float gCamW = -3.1415/2;
	float forward = 0;
	float sideways = 0;
	//animation data
	float lightTrans = 0;
	float gTrans = -3;
	float sTheta = 0;
	float sTheta2 = 0;
	float sTheta3 = 0;
	float sTheta4 = 0;
	int mat = 0;
	float eTheta = 0;
	float hTheta = 0;
	bool first = true;
	bool goCamera = false;
	bool charCam = true;
	bool moving = false;
	bool movingside = false;
	bool footplaying;
	float positionx = 0,positionz = 0,positiony = 0;
	double posX, posY;
	double newposX, newposY;
	vec3 direction;
	vec3 g_eye = vec3(0, 1, 0);
	vec3 direction1;
	float speed = 0.0;
	float speed2 = 0.0;
	bool speed1 = false;
	Spline splinepath[2];
	float lightz = 0.0;
	float light1z = -70.0;
	float doorz = 0.0;
	float cpositionx = 0.0;
	float cpositiony = 0.7;
	float cpositionz = -1.0;
	float lightint = 1.0;
	float backgroundz = 0.0;

	struct PointLight {
		vec3 position;  

		float constant;
		float linear;
		float quadratic;
	}; 

	PointLight light1;
	PointLight light2;
	float lightArray[60*3];

	bool sounds = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//update global camera rotate

		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			speed1 = true;
		}
		if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			speed1 = false;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			movingside = true;
			sideways = -0.05;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			movingside = true;
			sideways = +0.05;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			movingside = false;
			sideways = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			movingside = false;
			sideways = 0;
		}
		//update camera height
		if (key == GLFW_KEY_S && action == GLFW_PRESS){
			moving = true;
			forward = -0.05;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS){
			moving = true;
			forward = 0.05;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE){
			moving = false;
			forward = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE){
			moving = false;
			forward = 0;
		}

		if (key == GLFW_KEY_Q && action == GLFW_PRESS){
			lightz -= 2.0;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS){
			lightz += 2.0;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS){
			if (mat == 0){
				mat = 1;
			}
			else{
				mat = 0;
			}
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_G && action == GLFW_PRESS) {
			charCam = !charCam;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{	
		glfwGetCursorPos(window, &newposX, &newposY);
		if (action == GLFW_PRESS)
		{
			cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
			cout << "Angle X " << gCamW <<  " Angle Y " << gCamH << endl;
		}
	}

	
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		
	}
	

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightPos");
		prog->addUniform("lightPos1");
		prog->addUniform("lightPos2");
		prog->addUniform("lightPos3");
		prog->addUniform("lightPos4");
		prog->addUniform("lightArray");
		prog->addUniform("lightI");
		prog->addUniform("lightI1");
		prog->addUniform("constant");
		prog->addUniform("linear");
		prog->addUniform("quadratic");
		prog->addUniform("constant1");
		prog->addUniform("linear1");
		prog->addUniform("quadratic1");
		prog->addUniform("camPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatAmb");
		texProg->addUniform("MatDif");
		texProg->addUniform("MatSpec");
		texProg->addUniform("MatShine");;
		texProg->addUniform("lightPos");
		texProg->addUniform("lightPos1");
		texProg->addUniform("lightPos2");
		texProg->addUniform("lightPos3");
		texProg->addUniform("lightPos4");
		texProg->addUniform("lightArray");
		texProg->addUniform("lightI");
		texProg->addUniform("lightI1");
		texProg->addUniform("constant");
		texProg->addUniform("linear");
		texProg->addUniform("quadratic");
		texProg->addUniform("constant1");
		texProg->addUniform("linear1");
		texProg->addUniform("quadratic1");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		//read in a load the texture
		texture0 = make_shared<Texture>();
  		texture0->setFilename(resourceDirectory + "/dirtywall.jpg");
  		texture0->init();
  		texture0->setUnit(0);
  		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  		texture1 = make_shared<Texture>();
  		texture1->setFilename(resourceDirectory + "/city.jpg");
  		texture1->init();
  		texture1->setUnit(1);
  		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture2 = make_shared<Texture>();
  		texture2->setFilename(resourceDirectory + "/door.png");
  		texture2->init();
  		texture2->setUnit(1);
  		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture4 = make_shared<Texture>();
  		texture4->setFilename(resourceDirectory + "/walls.jpg");
  		texture4->init();
  		texture4->setUnit(1);
  		texture4->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		
		splinepath[0] = Spline(glm::vec3(-8,3,-8), glm::vec3(-6,3,0), glm::vec3(6, 3, 0), glm::vec3(8,3,-8), 10);
       	splinepath[1] = Spline(glm::vec3(8,3,-8), glm::vec3(8,5,-16), glm::vec3(0, 5, -16), glm::vec3(-8,5,-8), 10);
    
	}

	

	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = sphere->min.x;
		gMin.y = sphere->min.y;

		// Initialize bunny mesh.
		vector<tinyobj::shape_t> TOshapesB;
 		vector<tinyobj::material_t> objMaterialsB;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {	
			theBunny = make_shared<Shape>();
			theBunny->createShape(TOshapesB[0]);
			theBunny->measure();
			theBunny->init();
		}

		vector<tinyobj::shape_t> TOshapes4;
 		rc = tinyobj::LoadObj(TOshapes4, objMaterials, errStr, (resourceDirectory + "/dummy.obj").c_str());
		
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			for (tinyobj::shape_t shape: TOshapes4){
				floorshapes = make_shared<Shape>();
				floorshapes->createShape(shape);
				floorshapes->measure();
				floorshapes->init();
				floor.push_back(floorshapes);
			}
		}

		vector<tinyobj::shape_t> TOshapes5;
 		rc = tinyobj::LoadObj(TOshapes5, objMaterials, errStr, (resourceDirectory + "/Door.obj").c_str());
		
		if (!rc) {
			cerr << errStr << endl;
		} else {
			//for now all our shapes will not have textures - change in later labs
			for (tinyobj::shape_t shape: TOshapes5){
				doorshapes = make_shared<Shape>();
				doorshapes->createShape(shape);
				doorshapes->measure();
				doorshapes->init();
				door.push_back(doorshapes);
			}
		}

		initGround();
		boidsSetup();
	}

	void initGround() {

		float g_groundSize = 20;
		float g_groundY = -0.25;

  		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
      		0, 0, // back
      		0, 1,
      		1, 1,
      		1, 0 };

      	unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	g_GiboLen = 6;
      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndTexBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
      }

      //code to draw the ground plane
     void drawGround(shared_ptr<Program> curS, float groundz, float groundy, float groundx, float rotx, float roty, float rotz, int tex) {
     	curS->bind();
     	glBindVertexArray(GroundVertexArrayID);
		if (tex == 0){
			texture0->bind(curS->getUniform("Texture0"));
		}
		else if (tex == 1){
			texture4->bind(curS->getUniform("Texture0"));
		}
		//draw the ground plane 
  		SetModel(vec3(groundx, groundy, groundz), rotz, roty, rotx, 1, curS);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
  		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->unbind();
     }

	//directly pass quad for the ground to the GPU

      //code to draw the ground plane

     //helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
    		break;
    		case 1: // 
    			glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.038, 0.1);
    			glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
    			glUniform3f(curS->getUniform("MatSpec"), 0.3, 0.2, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 4.0);
    		break;
    		case 2: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
			case 3: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.24725, 0.1995, 0.0745);
    			glUniform3f(curS->getUniform("MatDif"), 0.714, 0.4284, 0.18144);
    			glUniform3f(curS->getUniform("MatSpec"), 0.116228, 0.116228, 0.416228);
    			glUniform1f(curS->getUniform("MatShine"), 6.8);
    		break;
			case 4: //
    			glUniform3f(curS->getUniform("MatAmb"), 1, 1, 1);
    			glUniform3f(curS->getUniform("MatDif"), 1, 1, 1);
    			glUniform3f(curS->getUniform("MatSpec"), 0, 0, 0);
    			glUniform1f(curS->getUniform("MatShine"), 1);
    		break;
			case 5: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.19225, 0.19225, 0.19225);
    			glUniform3f(curS->getUniform("MatDif"), 0.50754, 0.50754, 0.50754);
    			glUniform3f(curS->getUniform("MatSpec"), 0.508273, 0.508273, 0.508273);
    			glUniform1f(curS->getUniform("MatShine"), 51.2);
    		break;
			case 6: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.19225, 0.19225, 0.19225);
    			glUniform3f(curS->getUniform("MatDif"), 0.40754, 0.40754, 0.40754);
    			glUniform3f(curS->getUniform("MatSpec"), 0.308273, 0.308273, 0.308273);
    			glUniform1f(curS->getUniform("MatShine"), 31.2);
    		break;
			case 7: //
    			glUniform3f(curS->getUniform("MatAmb"), 0, 0, 0);
    			glUniform3f(curS->getUniform("MatDif"), 0, 0, 0);
    			glUniform3f(curS->getUniform("MatSpec"), 0, 0, 0);
    			glUniform1f(curS->getUniform("MatShine"), 1);
    		break;
  		}
	}

	/* helper function to set model trasnforms */
	void SetModel(vec3 trans, float rotZ, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 RotZ = glm::rotate( glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*RotZ*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void SetModel(vec3 trans, float rotZ, float rotY, float rotX, vec3 sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
		mat4 RotZ = glm::rotate( glm::mat4(1.0f), rotZ, vec3(0, 0, 1));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), sc);
  		mat4 ctm = Trans*RotX*RotY*RotZ*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}
	void mscPlay(){
		PlaySound(0, 0, 0);
		PlaySoundA((LPCSTR) "C:/Users/rohan/OneDrive/Desktop/CSC 471/Lab7_texture_s25/src/Ending Song.wav", NULL, SND_FILENAME | SND_ASYNC);
	}

	void sfxPlay(){
		PlaySoundA((LPCSTR) "C:/Users/rohan/OneDrive/Desktop/CSC 471/Lab7_texture_s25/src/footstep.wav", NULL, SND_FILENAME | SND_ASYNC);
		footplaying = true;
	}

	void boidsSetup(){
		for (int i = 0; i < 60; i++){
			Boid* x = new Boid(0 + (0.1*i),0 - (0.1*i), cpositionz-50, -50, 5, i);
			boids.push_front(x);
		}

	}

	void boidsUpdate(shared_ptr<Program> prog){
		int count = 0;
		for (Boid* b : boids){
			b->run(boids, cpositionz);
			SetModel(b->position ,0 ,0 ,0 ,0.01 ,prog);
			lightArray[(count*3)] = b->position.x;
			lightArray[(count*3)+1] = b->position.y;
			lightArray[(count*3)+2] = b->position.z;
			sphere->draw(prog);
			count++;
		}
	}

   	
   	void drawHierModel(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
   		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(cpositionx, cpositiony + ((sTheta2/32) + (1/32)), cpositionz));
			Model->rotate(-0.2, vec3(1, 0, 0));
			Model->scale(vec3(0.4, 0.4, 0.4));
			/* draw top cube - aka head */
			Model->pushMatrix();
				Model->translate(vec3(0, 1.4, 0));
				Model->scale(vec3(0.45, 0.45, 0.45));
				setModel(prog, Model);
				sphere->draw(prog);
			Model->popMatrix();
			//draw the torso with these transforms
			Model->pushMatrix();
			  Model->translate(vec3(0, 0.4, 0));
			  Model->scale(vec3(0.65, 0.5, 0.4));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(0, -0.5, 0));
			  Model->scale(vec3(0.4, 1.2, 0.4));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
			// draw the upper 'arm' - relative 
			//note you must change this to include 3 components!
			Model->pushMatrix();
			  

			//Right Arm Component
			  Model->translate(vec3(0.7, 0.5, 0));
			  //rotate shoulder joint
			  Model->rotate(-3.14/2 + 0.3, vec3(0, 0, 1));
			  Model->rotate(sTheta/3 + 0.1, vec3(0, 1, 0));
			  //move to shoulder joint
			  Model->translate(vec3(0.7, 0, 0));
			  	Model->pushMatrix();
				  Model->translate(vec3(0.5, 0, 0));
				  Model->rotate(0, vec3(0, 0, 1));
				  Model->rotate(sTheta/4 + 0.75, vec3(0, 1, 0));
				  Model->translate(vec3(0.5, 0, 0));
				  	Model->pushMatrix();
					  Model->translate(vec3(0.3, 0, 0));
					  Model->rotate(0, vec3(0, 0, 1));
					  Model->translate(vec3(0.3, 0, 0));
					  Model->scale(vec3(0.25, 0.25, 0.25));
					  setModel(prog, Model);
			  	  	  sphere->draw(prog);
				  Model->popMatrix();
				  Model->scale(vec3(0.4, 0.25, 0.25));
			  	  setModel(prog, Model);
			  	  sphere->draw(prog);
			  	Model->popMatrix();
			  Model->scale(vec3(0.7, 0.30, 0.30));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();

			//Left Arm Component
			  Model->translate(vec3(-0.7, 0.5, 0));
			  Model->rotate(3.14/2 - 0.3, vec3(0, 0, 1));
			  Model->rotate((sTheta/3) - 0.1, vec3(0, 1, 0));
			  Model->translate(vec3(-0.7, 0, 0));
			  	Model->pushMatrix();
				  Model->translate(vec3(-0.5, 0, 0));
				  Model->rotate(0, vec3(0, 0, 1));
				  Model->rotate((sTheta*0.25) - 0.75, vec3(0, 1, 0));
				  Model->translate(vec3(-0.5, 0, 0));
				  	Model->pushMatrix();
					  Model->translate(vec3(-0.3, 0, 0));
					  Model->rotate(0, vec3(0, 0, 1));
					  Model->translate(vec3(-0.3, 0, 0));
					  Model->scale(vec3(0.25, 0.25, 0.25));
					  setModel(prog, Model);
			  	  	  sphere->draw(prog);
				  Model->popMatrix();
				  Model->scale(vec3(0.4, 0.25, 0.25));
			  	  setModel(prog, Model);
			  	  sphere->draw(prog);
			  	Model->popMatrix();
			  Model->scale(vec3(0.7, 0.30, 0.30));
			  setModel(prog, Model);
			  sphere->draw(prog);
			  
			Model->popMatrix();
			Model->pushMatrix();

			//Left Leg Component
			Model->translate(vec3(-0.5, -1.7, 0));
			Model->rotate(3.1415/2, vec3(0, 0, 1));
			Model->rotate(-(sTheta*0.75) - 0.5, vec3(0, 1, 0));
			Model->translate(vec3(-0.6, 0, 0));
				Model->pushMatrix();
				Model->translate(vec3(-1.0, 0, 0));
				Model->rotate((sTheta*0.5) + 0.75, vec3(0, 1, 0));
				Model->translate(vec3(-1.0, 0, 0));
					Model->pushMatrix();
					Model->translate(vec3(-0.4, 0, 0));
					Model->rotate(0.0, vec3(0, 0, 1));
					Model->translate(vec3(-0.4, 0, 0));
					Model->scale(vec3(0.25, 0.2, 0.2));
					setModel(prog, Model);
						sphere->draw(prog);
				Model->popMatrix();
				Model->scale(vec3(1.0, 0.30, 0.30));
					setModel(prog, Model);
					sphere->draw(prog);
				Model->popMatrix();
			Model->scale(vec3(1.25, 0.40, 0.40));
			setModel(prog, Model);
			sphere->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();


		//Right Leg Component
		Model->translate(vec3(0.5, -1.7, 0));
			Model->rotate(3.1415/2, vec3(0, 0, 1));
			Model->rotate((sTheta*0.75) - 0.5, vec3(0, 1, 0));
			Model->translate(vec3(-0.6, 0, 0));
				Model->pushMatrix();
				Model->translate(vec3(-1.0, 0, 0));
				Model->rotate(-(sTheta*0.5) + 0.75, vec3(0, 1, 0));
				Model->translate(vec3(-1.0, 0, 0));
					Model->pushMatrix();
					Model->translate(vec3(-0.4, 0, 0));
					Model->rotate(0.0, vec3(0, 0, 1));
					Model->translate(vec3(-0.4, 0, 0));
					Model->scale(vec3(0.35, 0.2, 0.2));
					setModel(prog, Model);
						sphere->draw(prog);
				Model->popMatrix();
				Model->scale(vec3(1.0, 0.30, 0.30));
					setModel(prog, Model);
					sphere->draw(prog);
				Model->popMatrix();
			Model->scale(vec3(1.25, 0.40, 0.40));
			setModel(prog, Model);
			sphere->draw(prog);
			Model->popMatrix();
			Model->popMatrix();

   	}

	void updateUsingCameraPath(float frametime)  {

   	  if (goCamera) {
       if (!splinepath[0].isDone()){
       		splinepath[0].update(frametime);
            g_eye = splinepath[0].getPosition();
        } else {
            splinepath[1].update(frametime);
            g_eye = splinepath[1].getPosition();
        }
      }
   	}

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
		View->loadIdentity();
		//camera up and down
		if (glfwRawMouseMotionSupported()){
    		glfwSetInputMode(windowManager->getHandle(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		glfwGetCursorPos(windowManager->getHandle(), &newposX, &newposY);

		if (first == false){
			gCamW -= (posX-newposX)/180;
			gCamH += (posY-newposY)/180;
		}
		
		if (gCamH > 1){
			gCamH = 1;
		}
		if (gCamH < -1){
			gCamH = -1;
		}
		if (moving = false){
			forward = 0;
		}
		else{
			forward = forward*1.01;
			if (forward >= 0.1){
				forward = 0.1;
			}
		}

		if (movingside = false){
			sideways = 0;
		}
		else{
			sideways = sideways*1.01;
			if (sideways >= 0.1){
				sideways = 0.1;
			}
		}
		
		direction = vec3(forward*cos(gCamH)*cos(gCamW),forward*sin(gCamH),forward*cos(gCamH)*cos((3.1415/2)-gCamW));
		direction1 = vec3(sideways*cos(gCamH)*cos(gCamW+(3.1415/2)),sideways*sin(gCamH),sideways*cos(gCamH)*cos((3.1415/2)-(gCamW+(3.1415/2))));
		positiony = 1.5;
		positionx = positionx + direction.x + direction1.x;
		positionz = positionz + direction.z + direction1.z;
		if (charCam){
			positionx = cpositionx;
			positiony = cpositiony;
			positionz = cpositionz;
			if (gCamH > 0.4){
				gCamH = 0.4;
			}
			if (gCamH < -0.4){
				gCamH = -0.4;
			}
			if (gCamW > 0.4){
				gCamW = 0.4;
			}
			if (gCamW < -0.4){
				gCamW = -0.4;
			}
			View->lookAt(vec3(positionx - 4.0*cos(gCamH)*cos(gCamW-(3.1415/2)),positiony - 4.0*sin(gCamH),positionz - 4.0*cos(gCamH)*cos((3.1415/2)-(gCamW-(3.1415/2)))),vec3(cpositionx,cpositiony,cpositionz),vec3(0,1,0));
		}
		else if (goCamera){
			positionx = g_eye.x;
			positiony = g_eye.y;
			positionz = g_eye.z;
			View->lookAt(vec3(positionx, positiony, positionz),vec3( positionx + 1*cos(gCamH)*cos(gCamW),positiony + 1*sin(gCamH),positionz + 1*cos(gCamH)*cos((3.1415/2)-gCamW)),vec3(0,1,0));
		}
		else{
			View->lookAt(vec3(positionx, positiony, positionz),vec3(positionx + 1*cos(gCamH)*cos(gCamW),positiony + 1*sin(gCamH),positionz + 1*cos(gCamH)*cos((3.1415/2)-gCamW)),vec3(0,1,0));

		}
		posX = newposX;
		posY = newposY;


		updateUsingCameraPath(frametime);
		// Draw the scene
		
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog->getUniform("camPos"), positionx, positiony, positionz);
		light1.position = vec3(0, 3.5, lightz);
		light1.constant = 1.0f;
		light1.linear = 0.14f;
		light1.quadratic = 0.07f;
		light2.position = vec3(0, 1.0, light1z + cpositionz - 5);
		light2.constant = 1.0f;
		light2.linear = 1.0f;
		light2.quadratic = 2.8f;
		glUniform3f(prog->getUniform("lightPos"), light1.position.x, light1.position.y, light1.position.z);
		glUniform3f(prog->getUniform("lightPos1"), light1.position.x, light1.position.y, light1.position.z-2.0);
		glUniform3f(prog->getUniform("lightPos2"), light1.position.x, light1.position.y, light1.position.z-4.0);
		glUniform3f(prog->getUniform("lightPos4"), light2.position.x, light2.position.y, light2.position.z);
		glUniform1f(prog->getUniform("lightI"), 1.0 - lightint);
		glUniform1f(prog->getUniform("lightI1"), lightint);
		glUniform3f(prog->getUniform("lightPos3"), 0.0, -5.0, 5.0); //directional light
		glUniform1f(prog->getUniform("constant"), light1.constant);
		glUniform1f(prog->getUniform("linear"), light1.linear);
		glUniform1f(prog->getUniform("quadratic"), light1.quadratic);
		glUniform1f(prog->getUniform("constant1"), light2.constant);
		glUniform1f(prog->getUniform("linear1"), light2.linear);
		glUniform1f(prog->getUniform("quadratic1"), light2.quadratic);

		glUniform3fv(prog->getUniform("lightArray"), 60, lightArray);

		SetMaterial(prog, 4);
		boidsUpdate(prog);
		SetModel(vec3(0, -1.25, -5), 1.5708, 1.5708*2, 1.5708, 0.02, prog);
		
		SetMaterial(prog, 2);
		for (shared_ptr<Shape> floor1: floor){
			//floor1->draw(prog);
		}

		//SetModel(vec3(0, -1.25, -5), 1.5708, 0, 0, (0.04,0.04,0.02), prog);
		Model->pushMatrix();
		Model->loadIdentity();
		Model->translate(vec3(0, -1.25, -5));
		Model->rotate(1.5708, vec3(0, 0, 1));
		Model->scale(vec3(0.001,0.02,0.02),1);
		setModel(prog, Model);
		Model->popMatrix();
		SetMaterial(prog, 3);
		for (shared_ptr<Shape> floor1: floor){
			//floor1->draw(prog);
		}
		SetMaterial(prog, 3);
		drawHierModel(Model ,prog);
		
		/*
		SetMaterial(prog, 6);
		for (int i = 0; i < 10; i++){
			SetModel(vec3(3, -1.25, 0-(i*4)), 0, 1.5708, 0, 0.02, prog);
			for (shared_ptr<Shape> door1: door){
			door1->draw(prog);
		}
		}

		for (int i = 0; i < 10; i++){
			SetModel(vec3(-3, -1.25, 0-(i*4)), 0, -1.5708, 0, 0.02, prog);
			for (shared_ptr<Shape> door1: door){
			door1->draw(prog);
		}
		}*/
	

		SetMaterial(prog, 7);
		SetModel(vec3(0, 1, light1z + cpositionz), 0, sTheta4, sTheta4, 1.5 - 1.0*(cpositionz/50), prog);
		theBunny->draw(prog);

		SetMaterial(prog, 4);
		SetModel(vec3(0, 1, light1z + cpositionz - 5), 0, 0, 0, 7, prog);
		theBunny->draw(prog);


		SetMaterial(prog, 4);
		for (int i = 0; i < 20; i++){
			SetModel(vec3(0, 3.5, lightz-(i*2)), 0, 0, 0, 0.1, prog);
			if (i > 2){
				SetMaterial(prog, 7);
			}
			sphere->draw(prog);
		}


		prog->unbind();

		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(texProg->getUniform("lightPos"), light1.position.x, light1.position.y, light1.position.z);
		glUniform3f(texProg->getUniform("lightPos1"), light1.position.x, light1.position.y, light1.position.z-2.0);
		glUniform3f(texProg->getUniform("lightPos2"), light1.position.x, light1.position.y, light1.position.z-4.0);
		glUniform3f(texProg->getUniform("lightPos4"), light2.position.x, light2.position.y, light2.position.z);
		glUniform1f(texProg->getUniform("lightI"), 1.0 - lightint);
		glUniform1f(texProg->getUniform("lightI1"), lightint);
		glUniform3f(texProg->getUniform("lightPos3"), 0.0, -5.0, 5.0); //directional light
		glUniform1f(texProg->getUniform("constant"), light1.constant);
		glUniform1f(texProg->getUniform("linear"), light1.linear);
		glUniform1f(texProg->getUniform("quadratic"), light1.quadratic);
		glUniform1f(texProg->getUniform("constant1"), light2.constant);
		glUniform1f(texProg->getUniform("linear1"), light2.linear);
		glUniform1f(texProg->getUniform("quadratic1"), light2.quadratic);
		glUniform1f(texProg->getUniform("MatShine"), 27.9);
		glUniform1i(texProg->getUniform("flip"), 1);
		glUniform3fv(texProg->getUniform("lightArray"), 60, lightArray);
		texture0->bind(texProg->getUniform("Texture0"));
		
		// draw the array of bunnies
		Model->pushMatrix();

		float dScale = 1.0/(theBunny->max.x-theBunny->min.x);
		float sp = 9.0;
		float off = 5;
		  for (int i =0; i < 3; i++) {
		  	for (int j=0; j < 3; j++) {
			  Model->pushMatrix();
				Model->translate(vec3(off+sp*i, -0.5, off+sp*j));
				Model->scale(vec3(dScale));
				//SetMaterial(texProg, (i+j)%3);
				glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				theBunny->draw(texProg);
			  Model->popMatrix();
			}
		  }
		Model->popMatrix();

		texture2->bind(texProg->getUniform("Texture0"));
		

		for (int i = 0; i < 17; i++){
			SetModel(vec3(-3, -1.25, doorz-(i*4)), 0, -1.5708, 0, 0.02, texProg);
			for (shared_ptr<Shape> door1: door){
			door1->draw(texProg);
		}
		}

		for (int i = 0; i < 17; i++){
			SetModel(vec3(3, -1.25, doorz-(i*4)), 0, -3.1415*(1.5), 0, 0.02, texProg);
			for (shared_ptr<Shape> door1: door){
			door1->draw(texProg);
		}
		}

		SetMaterial(texProg, 5);
		//draw big background sphere
		glUniform1i(texProg->getUniform("flip"), 0);
		texture1->bind(texProg->getUniform("Texture0"));
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(positionx,0,positionz));
			Model->scale(vec3(75.0));
			setModel(texProg, Model);
			sphere->draw(texProg);
		Model->popMatrix();
		
		glUniform1i(texProg->getUniform("flip"), 1);
		drawGround(texProg, backgroundz + 0, -1, 0, 0, 0, 0, 1);
		drawGround(texProg,  backgroundz + -40, -1, 0, 0, 0, 0, 1);
		drawGround(texProg,  backgroundz + -80, -1, 0, 0, 0, 0, 1);
		drawGround(texProg,  backgroundz + -120, -1, 0, 0, 0, 0, 1);

		drawGround(texProg, backgroundz + 0, 4, 0, 0, 0, 0, 1);
		drawGround(texProg,  backgroundz + -40, 4, 0, 0, 0, 0, 1);
		drawGround(texProg,  backgroundz + -80, 4, 0, 0, 0, 0, 1);
		drawGround(texProg,  backgroundz + -120, 4, 0, 0, 0, 0, 1);

		
		drawGround(texProg, backgroundz + 0, 0, -3 - 0.35, 0, 0, 3.1415/2, 0);
		drawGround(texProg, backgroundz - 40.0, 0, -3 - 0.35, 0, 0, 3.1415/2, 0);
		drawGround(texProg, backgroundz - 80.0, 0, -3 - 0.35, 0, 0, 3.1415/2, 0);
		drawGround(texProg, backgroundz - 120.0, 0, -3 - 0.35, 0, 0, 3.1415/2, 0);

		drawGround(texProg, backgroundz + 0, 0, 3 - 0.15, 0, 0, 3.1415/2, 0);
		drawGround(texProg, backgroundz - 40.0, 0, 3 - 0.15, 0, 0, 3.1415/2, 0);
		drawGround(texProg, backgroundz - 80.0, 0, 3 - 0.15, 0, 0, 3.1415/2, 0);
		drawGround(texProg, backgroundz - 120.0, 0, 3 - 0.15, 0, 0, 3.1415/2, 0);
		//animation update example

		if (speed1){
			speed += 0.015;
			sTheta = sin(2*speed);
			sTheta2 = sin(4*speed);
			sTheta3 = sin(2*speed + 0.50);
			cpositionz = cpositionz - std::max(0.005f, abs(sTheta3)/80);
		}
		if (sounds){
			if (sTheta > 0.99 || sTheta < -0.99){
				if (!footplaying){
					sfxPlay();
				}
			}
			else{
				footplaying = false;
			}
		}
		sTheta4 = 0.1*glfwGetTime();
		if (cpositionz <= (lightz-4.0)){
			lightz -= 2.0;
			lightint = 0.0;
		}

		if (lightint < 1.0){
			lightint += 0.05;
		}
		else{
			lightint = 1.0;
		}

		if (cpositionz <= (doorz-4.0)){
			doorz -= 4.0;
		}

		if (cpositionz <= (backgroundz - 40.0)){
			backgroundz -= 40.0;
		}

		eTheta = std::max(0.0f, (float)sin(glfwGetTime()));
		hTheta = std::max(0.0f, (float)cos(glfwGetTime()));

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();
		first = false;

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{

		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;
		// Render scene.
		application->render(deltaTime);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
