#include <string.h>
#include <stdlib.h>
#include <object.h>
#include <image.h>
#include "project.h"


#define M_PI_12 (M_PI/12.0)

#define TILE_SLOPE (1 / sqrt(6))

#define FLAT 0
#define GENTLE (atan(TILE_SLOPE))
#define STEEP (atan(4 * TILE_SLOPE))
#define VERTICAL M_PI_2
#define FG_TRANSITION ((FLAT + GENTLE) / 2)
#define GS_TRANSITION ((GENTLE + STEEP) / 2)
#define SV_TRANSITION ((STEEP + VERTICAL) / 2)

#define GENTLE_DIAGONAL (atan(TILE_SLOPE * M_SQRT1_2))
#define STEEP_DIAGONAL (atan(4 * TILE_SLOPE * M_SQRT1_2))
#define FG_TRANSITION_DIAGONAL ((FLAT + GENTLE_DIAGONAL) / 2)

#define BANK M_PI_4
#define BANK_TRANSITION (M_PI_4 / 2)

#define CORKSCREW_RIGHT_YAW(angle) \
    (atan2(0.5 * (1 - cos(angle)), 1 - 0.5 * (1 - cos(angle))))
#define CORKSCREW_RIGHT_PITCH(angle) (-asin(-sin(angle) / sqrt(2.0)))
#define CORKSCREW_RIGHT_ROLL(angle) (-atan2(sin(angle) / sqrt(2.0), cos(angle)))

#define CORKSCREW_LEFT_YAW(angle) (-CORKSCREW_RIGHT_YAW(angle))
#define CORKSCREW_LEFT_PITCH(angle) (-CORKSCREW_RIGHT_PITCH(-angle))
#define CORKSCREW_LEFT_ROLL(angle) (-CORKSCREW_RIGHT_ROLL(angle))


void render_rotation(context_t* context,mesh_t* mesh,image_t* images,int num_frames,float pitch,float roll,float yaw)
{
	for(int i=0;i<num_frames;i++)
	{
	context_render_view(context,matrix_mult(rotate_y((2*i*M_PI)/num_frames),rotate_z(pitch)),images+i);
	}

}

void render_vehicle(context_t* context,mesh_t* mesh,image_t* images,int sprite_flags)
{
context_begin_render(context);
context_add_model(context,mesh,transform(matrix_identity(),vector3(0,0,0)),0); 
context_finalize_render(context);

int base_frame=0;

	if(sprite_flags&SPRITE_FLAT_SLOPE)
	{
	printf("Rendering flat sprites\n");
        render_rotation(context,mesh,images+base_frame,32,FLAT,0,0);
        base_frame+=32;
        }
	if(sprite_flags&SPRITE_GENTLE_SLOPE)
	{
	printf("Rendering gentle sprites\n");
	render_rotation(context,mesh,images+base_frame,4,FG_TRANSITION,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-FG_TRANSITION,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,32,GENTLE,0,0);
	base_frame+=32;
	render_rotation(context,mesh,images+base_frame,32,-GENTLE,0,0);
	base_frame+=32;
	}
	if(sprite_flags&SPRITE_STEEP_SLOPE)
	{
	printf("Rendering steep sprites\n");
	render_rotation(context,mesh,images+base_frame,8,GS_TRANSITION,0,0);
	base_frame+=8;
	render_rotation(context,mesh,images+base_frame,8,-GS_TRANSITION,0,0);
	base_frame+=8;
	render_rotation(context,mesh,images+base_frame,32,STEEP,0,0);
	base_frame+=32;
	render_rotation(context,mesh,images+base_frame,32,-STEEP,0,0);
	base_frame+=32;
	}
	if(sprite_flags&SPRITE_VERTICAL_SLOPE)
	{
	printf("Rendering vertical sprites\n");
	render_rotation(context,mesh,images+base_frame,4,SV_TRANSITION,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-SV_TRANSITION,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,32,VERTICAL,0,0);
	base_frame+=32;
	render_rotation(context,mesh,images+base_frame,32,-VERTICAL,0,0);
	base_frame+=32;
	render_rotation(context,mesh,images+base_frame,4,VERTICAL+M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-VERTICAL-M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,VERTICAL+2*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-VERTICAL-2*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,VERTICAL+3*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-VERTICAL-3*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,VERTICAL+4*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-VERTICAL-4*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,VERTICAL+5*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-VERTICAL-5*M_PI_12,0,0);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,M_PI,0,0);
	base_frame+=4;
	}
	if(sprite_flags&SPRITE_DIAGONAL_SLOPE)
	{
	printf("Rendering diagonal sprites\n");
	render_rotation(context,mesh,images+base_frame,4,FG_TRANSITION_DIAGONAL,0,M_PI_4);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-FG_TRANSITION_DIAGONAL,0,M_PI_4);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,GENTLE_DIAGONAL,0,M_PI_4);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-GENTLE_DIAGONAL,0,M_PI_4);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,STEEP_DIAGONAL,0,M_PI_4);
	base_frame+=4;
	render_rotation(context,mesh,images+base_frame,4,-STEEP_DIAGONAL,0,M_PI_4);
	base_frame+=4;
	}

context_end_render(context);
}


int project_export(project_t* project,context_t* context)
{
ride_t ride;

ride.track_pieces=0xFFFFFFFFFFFFFFFFULL;
ride.flags=14336;
ride.zero_cars=project->zero_cars;
ride.preview_index=0;
ride.track_types[0]=project->track_type;
ride.track_types[1]=0xFF;
ride.track_types[2]=0xFF;
ride.excitement=0;
ride.intensity=0;
ride.nausea=0;
ride.max_height=0;
ride.categories[0]=CATEGORY_ROLLERCOASTER;
ride.categories[1]=0xFF;
ride.min_cars_per_train=project->min_cars_per_train;
ride.max_cars_per_train=project->max_cars_per_train;
ride.flat_ride_cars=0xFF;
ride.tab_vehicle=0;
ride.default_vehicle=project->configuration[CAR_INDEX_DEFAULT];
ride.front_vehicle=project->configuration[CAR_INDEX_FRONT];
ride.second_vehicle=project->configuration[CAR_INDEX_SECOND];
ride.rear_vehicle=project->configuration[CAR_INDEX_REAR];
ride.third_vehicle=project->configuration[CAR_INDEX_THIRD];
ride.shop_item=0xFF;
ride.shop_item_secondary=0xFF;


color_scheme_t colors={{0,0,0}};
ride.default_colors=&colors;
ride.num_default_colors=1;


string_table_init(&(ride.name));
ride.name.strings[LANGUAGE_ENGLISH_UK]=project->name;
string_table_init(&(ride.description));
ride.description.strings[LANGUAGE_ENGLISH_UK]=project->description;
string_table_init(&(ride.capacity));
ride.capacity.strings[LANGUAGE_ENGLISH_UK]=project->capacity;


memset(ride.vehicles,0,4*sizeof(ride_vehicle_t));

	for(int i=0;i<project->num_vehicles;i++)
	{
	ride_vehicle_t* vehicle=ride.vehicles+i;
	vehicle->rotation_frame_mask=31; 
	vehicle->spacing=(project->vehicles[i].spacing*262144)/TILE_SIZE;               
	vehicle->friction=project->vehicles[i].mass;              
	vehicle->tab_height=0;            
	vehicle->num_seats=0;             
	vehicle->sprites=project->vehicles[i].sprite_flags;               
	vehicle->sprite_width=0;        //THESE ARE THE PROBLEM
	vehicle->sprite_height_negative=0;//BYTES DO NOT WRITE
	vehicle->sprite_height_positive=0;//FOR TRACKED RIDES
	vehicle->vehicle_animation=0;  
	vehicle->flags=ENABLE_ADDITIONAL_COLOR_1|ENABLE_ADDITIONAL_COLOR_2;
	vehicle->num_rows=0;
	vehicle->spin_inertia=0;
	vehicle->spin_friction=0;
	vehicle->running_sound=RUNNING_SOUND_STEEL_SMOOTH;
	vehicle->var_58=0;
	vehicle->secondary_sound=SECONDARY_SOUND_SCREAMS_1;
	vehicle->var_5A=0;
	vehicle->powered_acceleration=0; 
	vehicle->powered_max_speed=0;    
	vehicle->car_visual=0;           
	vehicle->effect_visual=1;        
	vehicle->z_value=5;              
	vehicle->special_frames=0;
	vehicle->peep_positions=NULL;
	vehicle->num_peep_positions=0;
	}

ride.sprites.images=malloc(project->num_sprites*sizeof(image_t));
ride.sprites.num_images=project->num_sprites;

	for(int i=0;i<3;i++)
	{
	image_new(ride.sprites.images+i,1,1,0,0,0x5);
	}

int current_sprite=3;
	for(int i=0;i<project->num_vehicles;i++)
	{
	printf("Vehicle %d\n",i);
	render_vehicle(context,&(project->vehicles[i].mesh),ride.sprites.images+current_sprite,project->vehicles[i].sprite_flags);
	current_sprite+=project->vehicles[i].num_sprites;
	}

/*
	for(int i=0;i<ride.sprites.num_images;i++)
	{
	char filename[255];
	sprintf(filename,"sprites/sprite_%d.png",i);
	FILE* file=fopen(filename,"w");
	image_write_png(ride.sprites.images+i,file);
	fclose(file);
	}
*/

object_t object;
object.flags=0;
object.checksum=0x20434D52;
memset(object.name,' ',8);
memcpy(object.name,project->filename,strlen((char*)project->filename));


uint8_t* data;uint32_t length;
error_t error=ride_encode(&ride,ENCODING_RLE,&(object.chunk));
    if(error!=ERROR_NONE)
    {
    printf("Error: %s\n",error_string(error));
    }
char output_path[512];
sprintf(output_path,"/Users/ec2618/Library/Application Support/OpenRCT2/object/%s.DAT",project->filename);
printf("%s\n",output_path);
FILE* file=fopen(output_path,"w");
    if(file==NULL)
    {
    printf("Could not open file for writing\n");
    return 1;
    }
error=object_write(&object,file);

    if(error!=ERROR_NONE)
    {
    printf("Could not write file\n");
    object_destroy(&object);
    return 1;
    }
object_destroy(&object);
return 0;
}
