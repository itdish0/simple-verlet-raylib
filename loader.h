#ifndef LOADER_H
#define LOADER_H

#define MAP_WIDTH 30
#define MAP_HEIGHT 20
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)

#define TILE_SIZE 32

#define TOP_TILE_MASK 0x08
#define LEFT_TILE_MASK 0x04
#define BOTTOM_TILE_MASK 0x02
#define RIGHT_TILE_MASK 0x01


#define TILE_EMPTY 0x00
#define TILE_FULL 0x0F
#define TILE_BR (BOTTOM_TILE_MASK|RIGHT_TILE_MASK)
#define TILE_BL (BOTTOM_TILE_MASK|LEFT_TILE_MASK)
#define TILE_TL (TOP_TILE_MASK|LEFT_TILE_MASK)
#define TILE_TR (TOP_TILE_MASK|RIGHT_TILE_MASK)

extern unsigned char physicsTilemap[MAP_SIZE];
extern bool LoadPhysicsLayer(const char* filename, unsigned char* target_buffer);
extern bool LoadMap(const char* filename);
#endif