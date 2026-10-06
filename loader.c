#include <stdio.h>
#include <stdbool.h>
#include "loader.h"

unsigned char physicsTilemap[MAP_SIZE];

bool LoadPhysicsLayer(const char* filename, unsigned char* target_buffer) {
    // 1. Open file stream in RAW BINARY READ mode
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Engine Error: Failed to open map file: %s\n", filename);
        return false;
    }

    // 2. Read the full memory grid in a single lightning-fast operation
    // fread parameters: (dest_ptr, size_of_each_item, total_items_to_read, file_stream)
    size_t bytes_read = fread(target_buffer, sizeof(unsigned char), MAP_SIZE, file);

    // 3. Always clean up and close your file handles
    fclose(file);
		
    // 4. Verify we actually read a full screen layer worth of bytes
    if (bytes_read != MAP_SIZE) {
        printf("Engine Warning: Expected %d bytes from %s, but only read %zu.\n", 
               MAP_SIZE, filename, bytes_read);
        return false;
    }
	
    return true;
}

bool LoadMap(const char* filename) {
	return LoadPhysicsLayer(filename, physicsTilemap);
}