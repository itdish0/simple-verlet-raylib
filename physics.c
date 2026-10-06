#include "physics.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdbool.h>

#include "loader.h"
#include <xmmintrin.h> // Required for _mm_rsqrt_ss

// Define it as 'static inline' so the compiler embeds it directly
// inside physics.c functions without any function call overhead.
static inline float fast_rsqrt(float val) {
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(val)));
}

float posX[MAX_PARTICLES];
float posY[MAX_PARTICLES];
float oldX[MAX_PARTICLES];
float oldY[MAX_PARTICLES];
float invMass[MAX_PARTICLES];
float particleRestitution[MAX_PARTICLES];
int particleConstraintCount[MAX_PARTICLES];
float particleRadius[MAX_PARTICLES];
float particleLife[MAX_PARTICLES];
float particleSkin[MAX_PARTICLES];
int totalParticles = 0;

int constraintIdxA[MAX_CONSTRAINTS];
int constraintIdxB[MAX_CONSTRAINTS];
float constraintRestLength[MAX_CONSTRAINTS];
float constraintRestSq[MAX_CONSTRAINTS];
float constraintMaxStretchSq[MAX_CONSTRAINTS];
float constraintMinStretchSq[MAX_CONSTRAINTS];
float constraintStiffness[MAX_CONSTRAINTS];
float constraintHardness[MAX_CONSTRAINTS];
int constraintType[MAX_CONSTRAINTS];
int constraintColor[MAX_CONSTRAINTS];
int totalConstraints = 0;


int create_particle(float x, float y, float inv_mass, float r, float restitution) {
	
    int new_idx = -1;
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (particleLife[i] <= 0.0f) {
            new_idx = i;
            break;
        }
    }

    if (new_idx == -1) return -1;
	if (new_idx + 1 > totalParticles) totalParticles = new_idx + 1;

    posX[new_idx] = x;
    posY[new_idx] = y;
    oldX[new_idx] = x;
    oldY[new_idx] = y;
    invMass[new_idx] = inv_mass;
    particleRadius[new_idx] = r;
    particleRestitution[new_idx] = restitution;
    particleLife[new_idx] = 1.0f;
	particleSkin[new_idx] = -1;

    return new_idx;
}

int create_constraint(int type, int idxA, int idxB, float stiffness) {
    float dx = posX[idxB] - posX[idxA];
    float dy = posY[idxB] - posY[idxA];
    float dist = sqrtf(dx * dx + dy * dy);

    return create_constraint_len(type, idxA, idxB, stiffness, dist);
}

int create_constraint_len(int type, int idxA, int idxB, float stiffness, float len) {
    int new_idx = -1;
    for (int i = 0; i < MAX_CONSTRAINTS; ++i) {
        if (constraintType[i] <= 0) {
            new_idx = i;
            break;
        }
    }

    if (new_idx == -1) return -1;
	
	if (new_idx + 1 > totalConstraints) totalConstraints = new_idx + 1;

    constraintType[new_idx] = type;
    constraintIdxA[new_idx] = idxA;
    constraintIdxB[new_idx] = idxB;
    constraintRestLength[new_idx] = len;
    constraintRestSq[new_idx] = len * len;
    constraintStiffness[new_idx] = stiffness;
	constraintMaxStretchSq[new_idx] = 2.0f* len*len;
	constraintColor[new_idx] = 1;

    return new_idx;
}

bool predict_ground_impact(int idx) {
	if (invMass[idx] == 0) return false;
	if (particleLife[idx] <= 0.0f) return false;
	float ox = posX[idx];
	float oy = posY[idx];
	float px = posX[idx] + (ox - oldX[idx]) * 0.99f;
	float py = posY[idx] + (oy - oldY[idx]) * 0.99f + DT_GRAVITY;
	
	float r = particleRadius[idx];
	float r_sq = r*r;
	float old_vx = px - ox;
	float old_vy = py - oy;

	float min_x_i = (px-r)/TILE_SIZE;
	float max_x_i = (px+r)/TILE_SIZE;
	float min_y_i = (py-r)/TILE_SIZE;
	float max_y_i = (py+r)/TILE_SIZE;
	
	if (min_x_i < 0) min_x_i = 0;
	if (max_x_i >= MAP_WIDTH) max_x_i = MAP_WIDTH - 1;
	if (min_y_i < 0) min_y_i = 0;
	if (max_y_i >= MAP_HEIGHT) max_y_i = MAP_HEIGHT - 1;

	// loop through (possibly) intersecting tiles

	for (int iy = min_y_i; iy <= max_y_i; ++iy) {
		for (int ix = min_x_i; ix <= max_x_i; ++ix) {
			// grab current tile - no duh
			unsigned char tile = physicsTilemap[iy * MAP_WIDTH + ix];
			if (tile == TILE_EMPTY) continue;
			float tx = ix * TILE_SIZE;
			float ty = iy * TILE_SIZE;
			float dtx = fabsf(tx + TILE_SIZE/2 - px) - TILE_SIZE/2;
			float dty = fabsf(ty + TILE_SIZE/2 - py) - TILE_SIZE/2;
			if (dty > 0 && dtx > 0) {
				float d_t_sq = dty*dty + dtx*dtx;
				if (d_t_sq > r_sq) continue;
			}
	
			// check standard floor
			
			if (old_vy > 0 && oy-r < ty && py+r > ty) {
				if (iy != 0) {
					unsigned char border_tile = physicsTilemap[(iy-1) * MAP_WIDTH + ix];
					if (tile&TOP_TILE_MASK && !(border_tile&BOTTOM_TILE_MASK)) {
						return true;
					}
				}
			}
			
			
			// Also the slopes... :P
			if (tile == TILE_BR) {
				if (old_vx < 0 && old_vy < 0) continue;
				float d_t_o = (tx + ty + TILE_SIZE - ox - oy)*M_SQRT1_2;
				float d_t_p = (tx + ty + TILE_SIZE - px - py)*M_SQRT1_2;
				if (d_t_o > 0 && d_t_p < r) {
					return true;
				}
			}
			if (tile == TILE_BL) {
				if (old_vx > 0 && old_vy < 0) continue;
				float d_t_o = (ox + ty - tx - oy)*M_SQRT1_2;
				float d_t_p = (px + ty - tx - py)*M_SQRT1_2;
				if (d_t_o > 0 && d_t_p < r) {
					return true;
				}
			}
		}
	}
	
	float limit_max_y = WORLD_MAX_Y - r;
	if (py > limit_max_y) {
		return true;
	}
	return false;
}

void spawn_physics_box(float x, float y, float size, float mass) {
    float half = size * 0.5f;
    float point_inv_mass = 1.0; // Distribute mass

    // 1. Spawn 4 Particles (Top-Left, Top-Right, Bottom-Right, Bottom-Left)
    int tl = create_particle(x - half, y - half, point_inv_mass, 8.0f, 0.5f);
    int tr = create_particle(x + half, y - half, point_inv_mass, 8.0f, 0.5f);
    int br = create_particle(x + half, y + half, point_inv_mass, 8.0f, 0.5f);
    int bl = create_particle(x - half, y + half, point_inv_mass, 8.0f, 0.5f);
	
    if (tl == -1 || tr == -1 || br == -1 || bl == -1) return;

    // 2. Spawn 4 Outer Edge Constraints (Stiffness = 1.0 for a solid body)
    create_constraint(C_EQUAL_DISTANCE, tl, tr, 1.0f); // Top
    create_constraint(C_EQUAL_DISTANCE, tr, br, 1.0f); // Right
    create_constraint(C_EQUAL_DISTANCE, br, bl, 1.0f); // Bottom
    create_constraint(C_EQUAL_DISTANCE, bl, tl, 1.0f); // Left

    // 3. Spawn 2 Diagonal Shear Constraints (Crucial for rigid shape preservation)
    create_constraint(C_EQUAL_DISTANCE, tl, br, 1.0f); // Diagonal 1
    create_constraint(C_EQUAL_DISTANCE, tr, bl, 1.0f); // Diagonal 2
}

void physicsSimulation(int iterations) {
	if (iterations > MAX_ITERATIONS) {
		iterations = MAX_ITERATIONS;
	}
	for (int i = 0; i < iterations; ++i) {
		for (int idx = 0; idx < totalParticles; ++idx) {
			if (invMass[idx] == 0) continue;
			if (particleLife[idx] <= 0.0f) continue;
			float tempX = posX[idx];
			float tempY = posY[idx];
			posX[idx] += (tempX - oldX[idx]) * 0.99f;
			posY[idx] += (tempY - oldY[idx]) * 0.99f + DT_GRAVITY;
			oldX[idx] = tempX;
			oldY[idx] = tempY;
		}
		for (int j = 0; j < CONSTRAINT_ITERATIONS; ++j) {
			for (int idx = 0; idx < totalConstraints; ++idx) {
				int cType = constraintType[idx];
				if (cType <= 0) continue;
				int idxA = constraintIdxA[idx];
				int idxB = constraintIdxB[idx];
				
				float imA = invMass[idxA];
				float imB = invMass[idxB];
				
				float c_imass = imA + imB;
				if (c_imass == 0) continue;
				float c_mass = 1.0f/c_imass;
				
				float d_x = posX[idxB]-posX[idxA];
				float d_y = posY[idxB]-posY[idxA];
				float d_sq = d_x*d_x + d_y*d_y;
				
				if (d_sq < 0.001f) continue;
				if (d_sq > constraintMaxStretchSq[idx] && constraintMaxStretchSq[idx] != -1) {
					//constraintType[idx] = 0;
					//continue;
				}
				if (d_sq < constraintMinStretchSq[idx]) {
					//constraintType[idx] = 0;
					//continue;
				}
				
				float cLen = constraintRestLength[idx];
				float cLen_sq = constraintRestSq[idx];
				if (cType == C_PUSH_ONLY && d_sq > cLen_sq) continue;
				if (cType == C_PULL_ONLY && d_sq < cLen_sq) continue;
				if (fabsf(cLen_sq - d_sq) < 0.001f) continue;
				
				float inv_dist = fast_rsqrt(d_sq);
				float percent = (1.0f - cLen*inv_dist) * constraintStiffness[idx];
				
				// constraint max pull
				
				//float c_sq = percent * percent * d_sq;
				
				//if (c_sq > MAX_CONSTRAINT_CORRECTION_SQ) {
				//	percent = MAX_CONSTRAINT_CORRECTION * inv_dist;
				//}
				
				float c_x = d_x * percent;
				float c_y = d_y * percent;
				
				if (imA != 0) {
					float ratioA = imA*c_mass;
					posX[idxA] += c_x * ratioA;
					posY[idxA] += c_y * ratioA;
				}
				if (imB != 0) {
					float ratioB = imB*c_mass;
					posX[idxB] -= c_x * ratioB;
					posY[idxB] -= c_y * ratioB;
				}
			}
			for (int idx = 0; idx < totalParticles; ++idx) {
				if (invMass[idx] == 0) continue;
				if (particleLife[idx] <= 0.0f) continue;

				float r = particleRadius[idx];
				float r_sq = r*r;
				float px = posX[idx];
				float py = posY[idx];
				float ox = oldX[idx];
				float oy = oldY[idx];
				float old_vx = px - ox;
				float old_vy = py - oy;
				float bounce = particleRestitution[idx];

				float min_x_i = (px-r)/TILE_SIZE;
				float max_x_i = (px+r)/TILE_SIZE;
				float min_y_i = (py-r)/TILE_SIZE;
				float max_y_i = (py+r)/TILE_SIZE;
				
				if (min_x_i < 0) min_x_i = 0;
				if (max_x_i >= MAP_WIDTH) max_x_i = MAP_WIDTH - 1;
				if (min_y_i < 0) min_y_i = 0;
				if (max_y_i >= MAP_HEIGHT) max_y_i = MAP_HEIGHT - 1;

				// loop through (possibly) intersecting tiles
			
				for (int iy = min_y_i; iy <= max_y_i; ++iy) {
					for (int ix = min_x_i; ix <= max_x_i; ++ix) {
						// grab current tile - no duh
						unsigned char tile = physicsTilemap[iy * MAP_WIDTH + ix];
						if (tile == TILE_EMPTY) continue;
						float tx = ix * TILE_SIZE;
						float ty = iy * TILE_SIZE;
						float dtx = fabsf(tx + TILE_SIZE/2 - px) - TILE_SIZE/2;
						float dty = fabsf(ty + TILE_SIZE/2 - py) - TILE_SIZE/2;
						if (dty > 0 && dtx > 0) {
							float d_t_sq = dty*dty + dtx*dtx;
							if (d_t_sq > r_sq) continue;
						}
						
						old_vx = px - ox;
						old_vy = py - oy;
				
						// ok this bit is more interesting, basically check if old<line & new>line,
						// also collide with left only if tile beside doesn't have right collision
						
						
						if (old_vx > 0 && ox-r < tx && px+r > tx) {
							if (ix != 0) {
								unsigned char border_tile = physicsTilemap[iy * MAP_WIDTH + ix - 1];
								if (tile&LEFT_TILE_MASK && !(border_tile&RIGHT_TILE_MASK)) {
									float penetration = px + r - tx;
									px = tx - r - penetration * bounce;
									if (bounce != 0.0f) ox = px + (old_vx * bounce);
									old_vy *= 0.8f;
									oy = py - old_vy;
									
								}
							}
						}
						if (old_vx < 0 && ox+r > tx+TILE_SIZE && px-r < tx+TILE_SIZE) { 
							if (ix != MAP_WIDTH - 1) {
								unsigned char border_tile = physicsTilemap[iy * MAP_WIDTH + ix + 1];
								if (tile&RIGHT_TILE_MASK && !(border_tile&LEFT_TILE_MASK)) {
									float penetration = tx + TILE_SIZE + r - px;
									px = tx + TILE_SIZE + r + penetration * bounce;
									if (bounce != 0.0f) ox = px + (old_vx * bounce);
									old_vy *= 0.8f;
									oy = py - old_vy;
									
								}
							}
						}
						if (old_vy > 0 && oy-r < ty && py+r > ty) {
							if (iy != 0) {
								unsigned char border_tile = physicsTilemap[(iy-1) * MAP_WIDTH + ix];
								if (tile&TOP_TILE_MASK && !(border_tile&BOTTOM_TILE_MASK)) {
									float penetration = py + r - ty;
									py = ty - r - penetration * bounce;
									if (bounce != 0.0f) oy = py + (old_vy * bounce);
									old_vx *= 0.8f;
									ox = px - old_vx;
									
								}
							}
						}
						if (old_vy < 0 && oy+r > ty+TILE_SIZE && py-r < ty+TILE_SIZE) { 
							if (iy != MAP_HEIGHT - 1) {
								unsigned char border_tile = physicsTilemap[(iy+1) * MAP_WIDTH + ix];
								if (tile&BOTTOM_TILE_MASK && !(border_tile&TOP_TILE_MASK)) {
									float penetration = ty + TILE_SIZE + r - py;
									py = ty + TILE_SIZE + r + penetration * bounce;
									if (bounce != 0.0f) oy = py + (old_vy * bounce);
									old_vx *= 0.8f;
									ox = px - old_vx;
									
								}
							}
						}
						
						// Also the slopes... :P
						
						float v_dd = old_vx + old_vy;
						float v_du = old_vx - old_vy;
						
						if (tile == TILE_BR) {
							if (v_dd < 0) continue;
							float d_t_o = (tx + ty + TILE_SIZE - ox - oy)*M_SQRT1_2;
							float d_t_p = (tx + ty + TILE_SIZE - px - py)*M_SQRT1_2;
							if (d_t_o > 0 && d_t_p < r) {
								float penetration = (r - d_t_p)*M_SQRT1_2;
								px -= penetration * (1 + bounce);
								py -= penetration * (1 + bounce);
								
								if (bounce != 0.0f) {
									v_dd *= -bounce;
									v_du *= 0.8f;
									
									old_vx = (v_dd+v_du)/2;
									old_vy = (v_dd-v_du)/2;
									
									ox = px - old_vx;
									oy = py - old_vy;
								}
							}
						}
						if (tile == TILE_BL) {
							if (v_du > 0) continue;
							float d_t_o = (ox + ty - tx - oy)*M_SQRT1_2;
							float d_t_p = (px + ty - tx - py)*M_SQRT1_2;
							if (d_t_o > 0 && d_t_p < r) {
								float penetration = (r - d_t_p)*M_SQRT1_2;
								px += penetration * (1 + bounce);
								py -= penetration * (1 + bounce);
								
								if (bounce != 0.0f) {
									v_du *= -bounce;
									v_dd *= 0.8f;
									
									old_vx = (v_dd+v_du)/2;
									old_vy = (v_dd-v_du)/2;
									
									ox = px - old_vx;
									oy = py - old_vy;
								}
							}
						}
						if (tile == TILE_TR) {
							if (v_du < 0) continue;
							float d_t_o = (tx + oy - ox - ty)*M_SQRT1_2;
							float d_t_p = (tx + py - px - ty)*M_SQRT1_2;
							if (d_t_o > 0 && d_t_p < r) {
								float penetration = (r - d_t_p)*M_SQRT1_2;
								px -= penetration * (1 + bounce);
								py += penetration * (1 + bounce);
								
								
								if (bounce != 0.0f) {
									v_du *= -bounce;
									v_dd *= 0.8f;
									
									old_vx = (v_dd+v_du)/2;
									old_vy = (v_dd-v_du)/2;
									
									ox = px - old_vx;
									oy = py - old_vy;
								}
								
							}
						}
						if (tile == TILE_TL) {
							if (v_dd > 0) continue;
							float d_t_o = (ox + oy - TILE_SIZE - tx - ty)*M_SQRT1_2;
							float d_t_p = (px + py - TILE_SIZE - tx - ty)*M_SQRT1_2;
							if (d_t_o > 0 && d_t_p < r) {
								float penetration = (r - d_t_p)*M_SQRT1_2;
								px += penetration * (1 + bounce);
								py += penetration * (1 + bounce);
								
								
								if (bounce != 0.0f) {
									v_dd *= -bounce;
									v_du *= 0.8f;
									
									old_vx = (v_dd+v_du)/2;
									old_vy = (v_dd-v_du)/2;
									
									ox = px - old_vx;
									oy = py - old_vy;
								}
								
							}
						}
					}
				}
				
				posX[idx]=px;
				posY[idx]=py;
				oldX[idx]=ox;
				oldY[idx]=oy;
				old_vx = px - ox;
				old_vy = py - oy;

				float limit_min_x = WORLD_MIN_X + r;
				float limit_max_x = WORLD_MAX_X - r;

				if (px < limit_min_x) {
					float penetration = limit_min_x - px;
					
					posX[idx] = limit_min_x + penetration * bounce;
					if (bounce != 0.0f) oldX[idx] = posX[idx] + (old_vx * bounce);
					
					oldY[idx] = py - (old_vy * 0.8f);
				}
				else if (px > limit_max_x) {
					float penetration = px - limit_max_x;
					
					posX[idx] = limit_max_x - penetration * bounce;
					if (bounce != 0.0f) oldX[idx] = posX[idx] + (old_vx * bounce);
					
					oldY[idx] = py - (old_vy * 0.8f);
				}

				float limit_min_y = WORLD_MIN_Y + r;
				float limit_max_y = WORLD_MAX_Y - r;

				if (py < limit_min_y) {
					float penetration = limit_min_y - py;
					
					posY[idx] = limit_min_y + penetration * bounce;
					
					if (bounce != 0.0f) oldY[idx] = posY[idx] + (old_vy * bounce);
					
					oldX[idx] = px - (old_vx * 0.8f);
				} 
				else if (py > limit_max_y) {
					float penetration = py - limit_max_y;
					
					posY[idx] = limit_max_y - penetration * bounce;
					if (bounce != 0.0f) oldY[idx] = posY[idx] + (old_vy * bounce);
					
					oldX[idx] = px - (old_vx * 0.8f);
				}
			}
		}
	}
}