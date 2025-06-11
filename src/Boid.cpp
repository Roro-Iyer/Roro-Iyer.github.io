#include "Boid.h"

#include <random>
#include <iostream>
#include <list>
#include <../glm/gtc/type_ptr.hpp>
#include <../glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

vec3 position;
vec3 velocity;
vec3 acceleration;
float borderz;
float borderz2;
float maxforce;
float maxspeed;
float positionz;
float celing;
float wall;
float wall1;
float floor1;
int ID;

    float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Boid::Boid(float x, float y, float z, float m, float n, int h){
    ID = h;
    acceleration = vec3(0.0, 0.0, 0.0);
    float angleH = randFloat(0.0f, 2* 3.1415);
    float angleW = randFloat(0.0f, 3.1415);
    velocity = vec3(0.01*cos(angleH)*cos(angleW),0.01*sin(angleH),0.01*cos(angleH)*cos((3.1415/2)-angleW));
    float randz = randFloat(-100.0f, 100.0f);
    float randx = randFloat(-0.5f, 0.5f);
    position = vec3(x + randx,y + randx,z - randz);
    borderz = m;
    borderz2 = n;
    maxspeed = 0.00001;
    maxforce = 0.03;

    celing = 3.5;
    floor1 = -1;
    wall = -2.75;
    wall1 = 2.25;
}

void Boid::run(list<Boid*> boids, float cpositionz){
    positionz = cpositionz;
    Boid::flock(boids);
    Boid::update();
    Boid::border();
}

void Boid::applyForce(vec3 force){
    acceleration += force;
}

void Boid::flock(list<Boid*> boids){
    
    Boid::applyForce(vec3(0,0,0.001));

    if (position.y >= celing-0.5){
        Boid::applyForce(vec3(0,-0.0001,0.0));
    }
    if (position.y <= floor1 + 0.5){
        Boid::applyForce(vec3(0,0.0001,0.0));
    }
    if (position.x >= wall1 - 0.5){
        Boid::applyForce(vec3(-0.001,0.0,0.0));
    }
    if (position.x <= wall + 0.5){
        Boid::applyForce(vec3(0.001,0.0,0.0));
    }
    if (positionz - 2.0 < position.z){
        if (position.x <= 0.75 && position.x >= -0.75){
            if (position.y <= 1 && position.y >= -1){
                Boid::applyForce(vec3(0.01 * position.x,0.0,0.0));
            }
        }
    }
}

void Boid::update(){
    velocity = velocity + acceleration;
    position = position + (0.25f*velocity);
    acceleration = vec3(0.0, 0.0, 0.0);
}

void Boid::border(){
    if (position.z >= (borderz2 + positionz)){
        position.x = 0;
        position.y = 0;
        float randz = randFloat(-10.0f, 0.0f);
        position.z = borderz + positionz + randz;
        float angleH = randFloat(0.0f, 3.1415);
        float angleW = randFloat(0.0f, 2*3.1415);
        velocity = vec3(0.01*cos(angleH)*cos(angleW),0.01*sin(angleH),0.01*cos(angleH)*cos((3.1415/2)-angleW));
    }

    if (position.y >= celing){
        position.y = celing;
    }
    if (position.y <= floor1){
        position.y = floor1;
    }
    if (position.x >= wall1){
        position.x = wall1;
    }
    if (position.x <= wall){
        position.x = wall;
    }
}