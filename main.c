#include "include/raylib.h"
#include "main.h"
#include "physics.h"
#include "loader.h"

float dt_acc = 0;
Camera2D camera = { 0 };
int mouse;
int m_c;

int body;
int b2;
int b3;


Vector2 v1;
Vector2 v2;
Vector2 v3;

int main(void) {
    // Initialize the window context (Width, Height, Title)
    InitWindow(WORLD_MAX_X, WORLD_MAX_Y, "Verlet Simulation in C");
	
	SetExitKey(0);
    // Set the game to run at 60 frames-per-second
    SetTargetFPS(60);
	
	camera.target = (Vector2){ WORLD_MAX_X/2, WORLD_MAX_Y/2 }; // What the camera is looking at (e.g., your physics body)
	camera.offset = (Vector2){ WORLD_MAX_X/2, WORLD_MAX_Y/2 };   // The screen center pivot point (Screen Width/2, Height/2)
	camera.rotation = 0.0f;                        // Camera rotation in degrees
	camera.zoom = 1.0f;
	
	mouse = create_particle(0, 0, 0, 0, 0);
	m_c = create_constraint(3, mouse, mouse, 1.0f);
	constraintMaxStretchSq[m_c] = -1;
	
	init_world();


    // Main game loop
    while (!WindowShouldClose()) { // Detect window close button
        // Update variables or game logic here
		dt_acc += GetFrameTime();
		update();
        render();
    }

    // De-initialize and close OpenGL context
    CloseWindow();

    return 0;
}

void rigid_quad(void) {
	create_particle(10,10,10,2,0);
	create_particle(10,10,10,5,0);
	create_particle(10,10,10,2,0.5f);
	create_particle(10,10,10,5,0.5f);
	
	int head = create_particle(10,10,10,5,0);
	int c = create_particle(20,10,5,3,0);
	
	int b1 = create_particle(30,10,1,3,0);
	body = create_particle(40,10,1,3,0);
	int b3 = create_particle(50,10,1,3,0);

	int t1 = create_particle(60,10,20,2,0);
	int t2 = create_particle(70,10,20,2,0);
	int t3 = create_particle(80,10,20,2,0);
	
	int k1 = create_particle(20,20,1,2,0);
	int k2 = create_particle(20,20,1,2,0);
	int k3 = create_particle(40,20,1,2,0);
	int k4 = create_particle(40,20,1,2,0);
	
	int f1 = create_particle(30,30,1,2,0);
	int f2 = create_particle(30,30,1,2,0);
	int f3 = create_particle(50,30,1,2,0);
	int f4 = create_particle(50,30,1,2,0);
	
	create_constraint_len(C_EQUAL_DISTANCE,head,c,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,c,b1,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,b1,body,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,body,b3,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,b3,t1,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,t1,t2,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,t2,t3,0.8f,10);
	
	create_constraint_len(C_EQUAL_DISTANCE,b1,k1,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,b1,k2,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,b3,k3,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,b3,k4,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,f1,k1,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,f2,k2,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,f3,k3,0.8f,10);
	create_constraint_len(C_EQUAL_DISTANCE,f4,k4,0.8f,10);
	
	create_constraint_len(C_EQUAL_DISTANCE,f1,b1,0.7f,17);
	create_constraint_len(C_EQUAL_DISTANCE,f2,b1,0.7f,17);
	create_constraint_len(C_EQUAL_DISTANCE,f3,b3,0.7f,17);
	create_constraint_len(C_EQUAL_DISTANCE,f4,b3,0.7f,17);
	
	create_constraint_len(C_PUSH_ONLY,f1,b3,0.9f,17);
	create_constraint_len(C_PUSH_ONLY,f2,b3,0.9f,17);
	create_constraint_len(C_PUSH_ONLY,f3,b1,0.9f,17);
	create_constraint_len(C_PUSH_ONLY,f4,b1,0.9f,17);
	
	create_constraint_len(C_PUSH_ONLY,f1,head,0.9f,30);
	create_constraint_len(C_PUSH_ONLY,f2,head,0.9f,30);
	
	create_constraint_len(C_PUSH_ONLY,head,b1,0.7f,19);
	create_constraint_len(C_PUSH_ONLY,c,body,0.7f,19);
	create_constraint_len(C_PUSH_ONLY,b1,b3,0.7f,19);
	create_constraint_len(C_PUSH_ONLY,body,t1,0.7f,19);
	create_constraint_len(C_PUSH_ONLY,b3,t2,0.7f,19);
	create_constraint_len(C_PUSH_ONLY,t1,t3,0.7f,19);
}

void init_world(void) {
	
	LoadMap("Tiled/p_start_map.bin");
	
	body = create_particle(80,80,1,10,0.5f);
	b2 = create_particle(80,80,10,10,0.5f);
	b3 = create_particle(80,80,10,10,0.5f);
	
	create_constraint_len(C_PULL_ONLY,body,b2,0.8f,10);
	create_constraint_len(C_PULL_ONLY,b2,b3,0.8f,10);

}

Vector2 screen_mouse;
Vector2 world_mouse;

void update(void) {
	screen_mouse = GetMousePosition();
	world_mouse = GetScreenToWorld2D(screen_mouse, camera);
	posX[mouse] = world_mouse.x;
	posY[mouse] = world_mouse.y;
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    
		float min_dsq = -1;
		int sel_idx = -1;
		int m_x = world_mouse.x;
		int m_y = world_mouse.y;
		for (int i = 0; i < totalParticles; ++i) {
			if (i == mouse) continue;
			int d_x = posX[i]-m_x;
			int d_y = posY[i]-m_y;
			int d_sq = d_x*d_x + d_y*d_y;
			if (min_dsq == -1 || d_sq < min_dsq) {
				min_dsq = d_sq;
				sel_idx = i;
			}
		}
		if (sel_idx != -1 && min_dsq < MAX_GRAB_LENGTH_SQ) {
			constraintIdxB[m_c] = sel_idx;
			constraintType[m_c] = C_EQUAL_DISTANCE;
		}
	}
	if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
		constraintIdxB[m_c] = mouse;
	}
	
	if (predict_ground_impact(body)+predict_ground_impact(b2)+predict_ground_impact(b3) >=2) {

		if (IsKeyDown(KEY_D)) {
			posX[body] += 0.5f;
		}

		if (IsKeyDown(KEY_A)) {
			posX[body] -= 0.5f;
		}

		if (IsKeyDown(KEY_W)) {
			posY[body] -= 5.0f;
		}

	} else {
		if (IsKeyDown(KEY_A)) {
			posX[body] -= 0.1f;
		}

		if (IsKeyDown(KEY_D)) {
			posX[body] += 0.1f;
		}
	}
	
	if (IsKeyDown(KEY_S)) {
		posY[body] += 1.0f;
	}
	
	int iterations = 0;
	while (iterations < MAX_ITERATIONS && dt_acc >= TIMESTEP) {
		++iterations;
		dt_acc -= TIMESTEP;
	}
	physicsSimulation(iterations);
}

void render(void) {
	//camera.target = (Vector2){ posX[0], posY[0] };
    BeginDrawing();
		ClearBackground(BLACK);
		BeginMode2D(camera);

#ifdef DEBUG
		for (int idx = 0; idx < MAP_SIZE; ++idx) {
			unsigned char type = physicsTilemap[idx];
			int x = idx%MAP_WIDTH;
			int y = idx/MAP_WIDTH;
			switch (type) {
				case TILE_FULL:
					DrawRectangle(TILE_SIZE*x, TILE_SIZE*y, TILE_SIZE, TILE_SIZE, WHITE);
					DrawRectangleLines(TILE_SIZE*x, TILE_SIZE*y, TILE_SIZE, TILE_SIZE, BLACK);
					break;

				case TILE_BR: 
					v3.x = TILE_SIZE*x;
					v2.x = TILE_SIZE*(x+1);
					v1.x = TILE_SIZE*(x+1);
					v2.y = TILE_SIZE*y;
					v3.y = TILE_SIZE*(y+1);
					v1.y = TILE_SIZE*(y+1);
					DrawTriangle(v1, v2, v3, WHITE);
					DrawTriangleLines(v1, v2, v3, BLACK);
					break;
				
				case TILE_BL: 
					v3.x = TILE_SIZE*x;
					v2.x = TILE_SIZE*x;
					v1.x = TILE_SIZE*(x+1);
					v2.y = TILE_SIZE*y;
					v3.y = TILE_SIZE*(y+1);
					v1.y = TILE_SIZE*(y+1);
					DrawTriangle(v1, v2, v3, WHITE);
					DrawTriangleLines(v1, v2, v3, BLACK);
					break;
				
				case TILE_TL: 
					v3.x = TILE_SIZE*x;
					v2.x = TILE_SIZE*x;
					v1.x = TILE_SIZE*(x+1);
					v2.y = TILE_SIZE*y;
					v3.y = TILE_SIZE*(y+1);
					v1.y = TILE_SIZE*y;
					DrawTriangle(v1, v2, v3, WHITE);
					DrawTriangleLines(v1, v2, v3, BLACK);
					break;
					
				case TILE_TR: 
					v3.x = TILE_SIZE*(x+1);
					v2.x = TILE_SIZE*x;
					v1.x = TILE_SIZE*(x+1);
					v2.y = TILE_SIZE*y;
					v3.y = TILE_SIZE*(y+1);
					v1.y = TILE_SIZE*y;
					DrawTriangle(v1, v2, v3, WHITE);
					DrawTriangleLines(v1, v2, v3, BLACK);
					break;
				
			}
		}
		
		for (int idx = 0; idx < totalParticles; ++idx) {
			if (particleLife[idx] <= 0.0f) continue;
			DrawCircle((int)posX[idx], (int)posY[idx], particleRadius[idx], predict_ground_impact(idx) ? RED:GREEN);
		}

		for (int idx = 0; idx < totalConstraints; ++idx) {
			if (constraintType[idx] <= 0) continue;
			
			int idxA = constraintIdxA[idx];
			int idxB = constraintIdxB[idx];
			int c_col = constraintColor[idx];
			if (c_col > 0) {
				DrawLine((int)posX[idxA], (int)posY[idxA], 
						(int)posX[idxB], (int)posY[idxB], WHITE);
			}
		}
#endif
		
		
		
		EndMode2D();

#ifdef DEBUG
		DrawFPS(10, 10);
#endif

	EndDrawing();
}
