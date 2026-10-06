#ifndef PHYSICS_H
#define PHYSICS_H

#ifndef MAX_PARTICLES
#define MAX_PARTICLES 500
#endif
#ifndef MAX_CONSTRAINTS
#define MAX_CONSTRAINTS 1000
#endif
#ifndef TIMESTEP
#define TIMESTEP (1.0f/60.0f)
#endif
#ifndef MAX_ITERATIONS
#define MAX_ITERATIONS 10
#endif
#ifndef CONSTRAINT_ITERATIONS
#define CONSTRAINT_ITERATIONS 4
#endif
#ifndef MAX_CONSTRAINT_CORRECTION
#define MAX_CONSTRAINT_CORRECTION 4
#endif

#define MAX_CONSTRAINT_CORRECTION_SQ (MAX_CONSTRAINT_CORRECTION * MAX_CONSTRAINT_CORRECTION)

#ifndef GRAVITY
#define GRAVITY 500.0f
#endif

#define DT_SQUARED (TIMESTEP*TIMESTEP)
#define DT_GRAVITY (DT_SQUARED*GRAVITY)

#define C_PULL_ONLY 1
#define C_PUSH_ONLY 2
#define C_EQUAL_DISTANCE 3

#ifndef WORLD_MIN_X
#define WORLD_MIN_X 0.0f
#endif
#ifndef WORLD_MAX_X
#define WORLD_MAX_X (TILE_SIZE*MAP_WIDTH)
#endif
#ifndef WORLD_MIN_Y
#define WORLD_MIN_Y 0.0f
#endif
#ifndef WORLD_MAX_Y
#define WORLD_MAX_Y (TILE_SIZE*MAP_HEIGHT)
#endif

extern float posX[MAX_PARTICLES];
extern float posY[MAX_PARTICLES];
extern float oldX[MAX_PARTICLES];
extern float oldY[MAX_PARTICLES];
extern float invMass[MAX_PARTICLES];
extern int particleConstraintCount[MAX_PARTICLES];
extern float particleLife[MAX_PARTICLES];
extern float particleRadius[MAX_PARTICLES];
extern float particleSkin[MAX_PARTICLES];
extern int totalParticles;

extern int constraintIdxA[MAX_CONSTRAINTS];
extern int constraintIdxB[MAX_CONSTRAINTS];
extern float constraintRestLength[MAX_CONSTRAINTS];
extern float constraintStiffness[MAX_CONSTRAINTS];
extern float constraintHardness[MAX_CONSTRAINTS];
extern int constraintType[MAX_CONSTRAINTS];
extern float constraintMaxStretchSq[MAX_CONSTRAINTS];
extern float constraintMinStretchSq[MAX_CONSTRAINTS];
extern int constraintColor[MAX_CONSTRAINTS];
extern int totalConstraints;

extern bool predict_ground_impact(int pId);

extern void physicsSimulation(int iterations);
extern int create_particle(float x, float y, float inv_mass, float radius, float restitution);
extern int create_constraint(int type, int idxA, int idxB, float stiffness);
extern int create_constraint_len(int type, int idxA, int idxB, float stiffness, float len);

extern void spawn_physics_box(float x, float y, float size, float mass);

#endif