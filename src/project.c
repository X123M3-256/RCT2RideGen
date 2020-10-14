#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <image.h>
#include <jansson.h>
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
json_t* json=json_object();
json_object_set_new(json,"id",json_string(project->filename));
json_object_set_new(json,"originalID",json_string("00000005|#RMCT1  |00000000"));
json_object_set_new(json,"version",json_string("1.0"));
json_object_set_new(json,"sourceGame",json_string("custom"));

json_t* authors=json_array();
json_array_append_new(authors,json_string("Edward Calver"));
json_object_set_new(json,"authors",authors);

json_object_set_new(json,"objectType",json_string("ride"));

//Ride header
json_t* properties=json_object();
json_t* types=json_array();
json_array_append_new(types,json_string("junior_rc"));
json_object_set_new(properties,"type",types);
//json_object_set_new(properties,"noInversions",json_false());
json_object_set_new(properties,"minCarsPerTrain",json_integer(4));
json_object_set_new(properties,"maxCarsPerTrain",json_integer(7));
//json_object_set_new(properties,"numEmptyCars",json_integer(0));
//json_object_set_new(properties,"tabCar",json_integer(0));
//json_object_set_new(properties,"headCars",json_integer(0));
//json_object_set_new(properties,"tailCars",json_integer(0));
//json_object_set_new(properties,"ratingMultiplier",json_integer(0));
json_object_set_new(properties,"buildMenuPriority",json_integer(1));
json_t* car_color_presets=json_array();
json_t* car_color_preset=json_array();
json_array_append_new(car_color_preset,json_string("black"));
json_array_append_new(car_color_preset,json_string("black"));
json_array_append_new(car_color_preset,json_string("black"));
json_array_append_new(car_color_presets,car_color_preset);
json_object_set_new(properties,"carColours",car_color_presets);

json_t* cars=json_array();
	for(int i=0;i<project->num_vehicles;i++)
	{
	json_t* car=json_object();
	json_object_set_new(car,"rotationFrameMask",json_integer(31));
	json_object_set_new(car,"spacing",json_integer((project->vehicles[i].spacing*262144)/TILE_SIZE));
	json_object_set_new(car,"mass",json_integer(project->vehicles[i].mass));
	json_object_set_new(car,"numSeats",json_integer(0));
	json_object_set_new(car,"numSeatRows",json_integer(0));
	json_object_set_new(car,"frictionSoundId",json_integer(1));
	json_object_set_new(car,"soundRange",json_integer(1));
	json_object_set_new(car,"drawOrder",json_integer(6));
	json_t* frames=json_object();
		if(project->vehicles[i].sprite_flags&SPRITE_FLAT_SLOPE)json_object_set_new(frames,"flat",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_GENTLE_SLOPE)json_object_set_new(frames,"gentleSlopes",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_STEEP_SLOPE)json_object_set_new(frames,"steepSlopes",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_DIAGONAL_SLOPE)json_object_set_new(frames,"diagonalSlopes",json_true());
	json_object_set_new(car,"frames",frames);
	json_object_set_new(car,"VEHICLE_ENTRY_FLAG_ENABLE_ADDITIONAL_COLOUR_1",json_true());
	json_object_set_new(car,"VEHICLE_ENTRY_FLAG_ENABLE_ADDITIONAL_COLOUR_2",json_true());
	json_object_set_new(car,"loadingPositions",json_array());
	}
json_object_set_new(properties,"cars",cars);
json_object_set_new(json,"properties",properties);

//String tables
json_t* strings=json_object();
json_t* name=json_object();
json_object_set_new(name,"en-GB",json_string(project->name));
json_object_set_new(strings,"name",name);
json_t* description=json_object();
json_object_set_new(description,"en-GB",json_string(project->description));
json_object_set_new(strings,"description",description);
json_t* capacity=json_object();
json_object_set_new(capacity,"en-GB",json_string(project->capacity));
json_object_set_new(strings,"capacity",capacity);
json_object_set_new(json,"strings",strings);

char fullname[256];
sprintf(fullname,"%s.json",project->filename);
json_dump_file(json,fullname,JSON_INDENT(4));


/*
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
*/
return 0;
}
