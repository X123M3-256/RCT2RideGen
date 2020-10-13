#include<stdint.h>
#include<renderer.h>

#define TILE_SIZE 3.3

typedef struct
{
mesh_t mesh;
uint32_t mass;
uint32_tsprite_flags;
uint32_t num_sprites;
float spacing;
}vehicle_t;

typedef struct
{
uint8_t* filename;
uint32_t checksum;
uint8_t* name;
uint8_t* description;
uint8_t* capacity;
uint8_t track_type;
uint8_t zero_cars;
uint8_t min_cars_per_train;
uint8_t max_cars_per_train;
uint8_t configuration[5];
uint32_t num_sprites;
uint32_t num_vehicles;
vehicle_t vehicles[4];
}project_t;

int project_export(project_t* project,context_t* context);
