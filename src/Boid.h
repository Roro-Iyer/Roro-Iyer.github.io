#ifndef BOID_H
#define BOID_H

#include <random>
#include <list>
#include <../glm/gtc/type_ptr.hpp>
#include <../glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

class Boid
{
    public:
        vec3 position;
        vec3 velocity;
        vec3 acceleration;
        float borderx;
        float bordery;
        float maxforce;
        float maxspeed;
        int ID;
        Boid(float x, float y, float z, float m, float n, int h);
        void run(list<Boid*> boids, float cpositionz);
        void applyForce(vec3 force);
        void flock(list<Boid*> boids);
        void update();
        void border();
        vec3 seek(vec3 target);
        vec3 align(list<Boid*> boids);
        vec3 cohesion(list<Boid*> boids);
        vec3 separate(list<Boid*> boids);
};

#endif