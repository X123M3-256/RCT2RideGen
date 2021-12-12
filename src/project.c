#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <jansson.h>
//#include <zip.h>
#include <image.h>
#include "project.h"
#include "pack.h"

#define M_PI_8 (M_PI/8.0)
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

json_t* json_image(const char* path,int x,int y,int src_x,int src_y,int src_width,int src_height)
{
json_t* image=json_object();
json_object_set_new(image,"path",json_string(path));
json_object_set_new(image,"x",json_integer(x));
json_object_set_new(image,"y",json_integer(y));
assert(src_width!=0&&src_height!=0);

	if(src_x>=0)json_object_set_new(image,"srcX",json_integer(src_x));
	if(src_y>=0)json_object_set_new(image,"srcY",json_integer(src_y));
	if(src_width>0)json_object_set_new(image,"srcWidth",json_integer(src_width));
	if(src_height>0)json_object_set_new(image,"srcHeight",json_integer(src_height));

json_object_set_new(image,"palette",json_string("keep"));
return image;
}



int count_sprites_from_flags(uint16_t sprites)
{
int count=0;
	if(sprites&SPRITE_FLAT_SLOPE)count+=32;
	if(sprites&SPRITE_GENTLE_SLOPE)count+=72;
	if(sprites&SPRITE_STEEP_SLOPE)count+=80;
	if(sprites&SPRITE_VERTICAL_SLOPE)count+=116;
	if(sprites&SPRITE_DIAGONAL_SLOPE)count+=24;
	if(sprites&SPRITE_BANKING)count+=80;
	if(sprites&SPRITE_INLINE_TWIST)count+=40;
	if(sprites&SPRITE_SLOPE_BANK_TRANSITION)count+=128;
	if(sprites&SPRITE_DIAGONAL_BANK_TRANSITION)count+=16;
	if(sprites&SPRITE_SLOPED_BANK_TRANSITION)count+=16;
	if(sprites&SPRITE_SLOPED_BANKED_TURN)count+=128;
	if(sprites&SPRITE_BANKED_SLOPE_TRANSITION)count+=16;
	if(sprites&SPRITE_CORKSCREW)count+=80;
	if(sprites&SPRITE_RESTRAINT_ANIMATION)count+=12;
return count;
}

void render_rotation(context_t* context,int num_frames,float pitch,float roll,float yaw,image_t* images)
{
	for(int i=0;i<num_frames;i++)
	{
	context_render_view(context,matrix_mult(rotate_y(yaw+(2*i*M_PI)/num_frames),matrix_mult(rotate_z(pitch),rotate_x(roll))),images+i);
	}

}

void render_vehicle(context_t* context,project_t* project,int i,image_t* images)
{
int sprite_flags=project->vehicles[i].sprite_flags;

int base=0;
	if(sprite_flags&SPRITE_FLAT_SLOPE)
	{
	printf("Rendering flat sprites\n");
        render_rotation(context,32,FLAT,0,0,images+base);
	base+=32;
        }
	if(sprite_flags&SPRITE_GENTLE_SLOPE)
	{
	printf("Rendering gentle sprites\n");
	render_rotation(context,4,FG_TRANSITION,0,0,images+base);
	base+=4;
	render_rotation(context,4,-FG_TRANSITION,0,0,images+base);
	base+=4;
	render_rotation(context,32,GENTLE,0,0,images+base);
	base+=32;
	render_rotation(context,32,-GENTLE,0,0,images+base);
	base+=32;
	}
	if(sprite_flags&SPRITE_STEEP_SLOPE)
	{
	printf("Rendering steep sprites\n");
	render_rotation(context,8,GS_TRANSITION,0,0,images+base);
	base+=8;
	render_rotation(context,8,-GS_TRANSITION,0,0,images+base);
	base+=8;
	render_rotation(context,32,STEEP,0,0,images+base);
	base+=32;
	render_rotation(context,32,-STEEP,0,0,images+base);
	base+=32;
	}
	if(sprite_flags&SPRITE_VERTICAL_SLOPE)
	{
	printf("Rendering vertical sprites\n");
	render_rotation(context,4,SV_TRANSITION,0,0,images+base);
	base+=4;
	render_rotation(context,4,-SV_TRANSITION,0,0,images+base);
	base+=4;
	render_rotation(context,32,VERTICAL,0,0,images+base);
	base+=32;
	render_rotation(context,32,-VERTICAL,0,0,images+base);
	base+=32;
	render_rotation(context,4,VERTICAL+M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,-VERTICAL-M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,VERTICAL+2*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,-VERTICAL-2*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,VERTICAL+3*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,-VERTICAL-3*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,VERTICAL+4*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,-VERTICAL-4*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,VERTICAL+5*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,-VERTICAL-5*M_PI_12,0,0,images+base);
	base+=4;
	render_rotation(context,4,M_PI,0,0,images+base);
	base+=4;
	}
	if(sprite_flags&SPRITE_DIAGONAL_SLOPE)
	{
	printf("Rendering diagonal sprites\n");
	render_rotation(context,4,FG_TRANSITION_DIAGONAL,0,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,-FG_TRANSITION_DIAGONAL,0,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,GENTLE_DIAGONAL,0,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,-GENTLE_DIAGONAL,0,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,STEEP_DIAGONAL,0,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,-STEEP_DIAGONAL,0,M_PI_4,images+base);
	base+=4;
	}
        if (sprite_flags & SPRITE_BANKING)
	{
	printf("Rendering banked sprites\n");
	render_rotation(context,8,FLAT,BANK_TRANSITION,0,images+base);
	base+=8;
	render_rotation(context,8,FLAT,-BANK_TRANSITION,0,images+base);
	base+=8;
	render_rotation(context,32,FLAT,BANK,0,images+base);
	base+=32;
	render_rotation(context,32,FLAT,-BANK,0,images+base);
	base+=32;
        }
        if (sprite_flags & SPRITE_INLINE_TWIST)
	{
	printf("Rendering inline twist sprites\n");
	render_rotation(context,4,FLAT,3.0 * M_PI_8,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,-3.0 * M_PI_8,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,M_PI_2,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,-M_PI_2,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,5.0 * M_PI_8,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,-5.0 * M_PI_8,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,3.0 * M_PI_4,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,-3.0 * M_PI_4,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,7.0 * M_PI_8,0,images+base);
	base+=4;
	render_rotation(context,4,FLAT,-7.0 * M_PI_8,0,images+base);
	base+=4;
        }
        if (sprite_flags & SPRITE_SLOPE_BANK_TRANSITION)
	{
	printf("Rendering slope-bank transition sprites\n");
	render_rotation(context,32,FG_TRANSITION,BANK_TRANSITION,0,images+base);
	base+=32;
	render_rotation(context,32,FG_TRANSITION,-BANK_TRANSITION,0,images+base);
	base+=32;
	render_rotation(context,32,-FG_TRANSITION,BANK_TRANSITION,0,images+base);
	base+=32;
	render_rotation(context,32,-FG_TRANSITION,-BANK_TRANSITION,0,images+base);
	base+=32;
        }
        if (sprite_flags & SPRITE_DIAGONAL_BANK_TRANSITION)
	{
	printf("Rendering diagonal slope-bank transition sprites\n");
	render_rotation(context,4,GENTLE_DIAGONAL,BANK_TRANSITION,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,GENTLE_DIAGONAL,-BANK_TRANSITION,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,-GENTLE_DIAGONAL,BANK_TRANSITION,M_PI_4,images+base);
	base+=4;
	render_rotation(context,4,-GENTLE_DIAGONAL,-BANK_TRANSITION,M_PI_4,images+base);
	base+=4;
        }
        if (sprite_flags & SPRITE_SLOPED_BANK_TRANSITION)
	{
	printf("Rendering diagonal sloped bank transition sprites\n");
	render_rotation(context,4,GENTLE,BANK_TRANSITION,0,images+base);
	base+=4;
	render_rotation(context,4,GENTLE,-BANK_TRANSITION,0,images+base);
	base+=4;
	render_rotation(context,4,-GENTLE,BANK_TRANSITION,0,images+base);
	base+=4;
	render_rotation(context,4,-GENTLE,-BANK_TRANSITION,0,images+base);
	base+=4;
        }
        if (sprite_flags & SPRITE_SLOPED_BANKED_TURN)
	{
	printf("Rendering sloped banked sprites\n");
	render_rotation(context,32,GENTLE,BANK,0,images+base);
	base+=32;
	render_rotation(context,32,GENTLE,-BANK,0,images+base);
	base+=32;
	render_rotation(context,32,-GENTLE,BANK,0,images+base);
	base+=32;
	render_rotation(context,32,-GENTLE,-BANK,0,images+base);
	base+=32;
        }
        if (sprite_flags & SPRITE_BANKED_SLOPE_TRANSITION)
	{
	printf("Rendering banked slope transition sprites\n");
	render_rotation(context,4,FG_TRANSITION,BANK,0,images+base);
	base+=4;
	render_rotation(context,4,FG_TRANSITION,-BANK,0,images+base);
	base+=4;
	render_rotation(context,4,-FG_TRANSITION,BANK,0,images+base);
	base+=4;
	render_rotation(context,4,-FG_TRANSITION,-BANK,0,images+base);
	base+=4;
        }
        if (sprite_flags & SPRITE_CORKSCREW)
	{
	printf("Rendering corkscrew sprites\n");
	#define CORKSCREW_ANGLE_1 2.0 * M_PI_12
	#define CORKSCREW_ANGLE_2 4.0 * M_PI_12
	#define CORKSCREW_ANGLE_3 M_PI_2
	#define CORKSCREW_ANGLE_4 8.0 * M_PI_12
	#define CORKSCREW_ANGLE_5 10.0 * M_PI_12

	// Corkscrew right
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_1),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_2),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_3),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_4),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_5),images+base);
	base+=4;

	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_1),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_2),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_3),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_4),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_5),images+base);
	base+=4;

	// Half corkscrew left
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_1),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_1),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_1),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_2),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_2),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_2),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_3),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_3),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_3),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_4),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_4),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_4),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_5),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_5),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_5),images+base);
	base+=4;

	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_1),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_1),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_1),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_2),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_2),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_2),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_3),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_3),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_3),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_4),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_4),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_4),images+base);
	base+=4;
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_5),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_5),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_5),images+base);
	base+=4;
        }
}

int project_export(project_t* project,context_t* context,const char* output_directory)
{
//Create working directory
	if(mkdir("object", 0700)==-1)
	{
//	printf("Failed to create working directory: %s\n",strerror(errno));
//	return 1;
	}
//Create image directory
	if(!mkdir("object/images", 0700)==-1)
	{
//	printf("Failed to create image directory: %s\n",strerror(errno));
//	return 1;
	}

//Create JSON file
json_t* json=json_object();
json_object_set_new(json,"id",json_string(project->id));
//json_object_set_new(json,"originalId",json_string("09F8EB00|#VEKSD  |00000000"));
json_object_set_new(json,"version",json_string("1.0"));
//json_object_set_new(json,"sourceGame",json_string("custom"));

json_t* authors=json_array();
json_array_append_new(authors,json_string("Edward Calver"));
json_object_set_new(json,"authors",authors);

json_object_set_new(json,"objectType",json_string("ride"));

//Ride header
json_t* properties=json_object();
json_t* types=json_array();
json_array_append_new(types,json_string(project->ride_type));
json_object_set_new(properties,"type",types);
json_object_set_new(properties,"category",json_string("rollercoaster"));
//json_object_set_new(properties,"noInversions",json_false());
json_object_set_new(properties,"minCarsPerTrain",json_integer(4));
json_object_set_new(properties,"maxCarsPerTrain",json_integer(7));
//json_object_set_new(properties,"numEmptyCars",json_integer(0));
//json_object_set_new(properties,"tabCar",json_integer(0));
json_object_set_new(properties,"defaultCar",json_integer(project->configuration[CAR_INDEX_DEFAULT]));
	if(project->configuration[CAR_INDEX_FRONT]!=0xFF)json_object_set_new(properties,"headCars",json_integer(project->configuration[CAR_INDEX_FRONT]));//TODO support multiple head cars
	if(project->configuration[CAR_INDEX_REAR]!=0xFF)json_object_set_new(properties,"headCars",json_integer(project->configuration[CAR_INDEX_REAR]));
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
	json_object_set_new(car,"numSeats",json_integer(project->vehicles[i].num_riders));
	json_object_set_new(car,"numSeatRows",json_integer(project->vehicles[i].num_rider_models));
	json_object_set_new(car,"frictionSoundId",json_integer(1));
	json_object_set_new(car,"soundRange",json_integer(1));
	json_object_set_new(car,"drawOrder",json_integer(project->vehicles[i].draw_order));
	json_t* frames=json_object();
		if(project->vehicles[i].sprite_flags&SPRITE_FLAT_SLOPE)json_object_set_new(frames,"flat",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_GENTLE_SLOPE)json_object_set_new(frames,"gentleSlopes",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_STEEP_SLOPE)json_object_set_new(frames,"steepSlopes",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_VERTICAL_SLOPE)json_object_set_new(frames,"verticalSlopes",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_DIAGONAL_SLOPE)json_object_set_new(frames,"diagonalSlopes",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_BANKING)json_object_set_new(frames,"flatBanked",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_INLINE_TWIST)json_object_set_new(frames,"inlineTwists",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_SLOPE_BANK_TRANSITION)json_object_set_new(frames,"flatToGentleSlopeBankedTransitions",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_DIAGONAL_BANK_TRANSITION)json_object_set_new(frames,"diagonalGentleSlopeBankedTransitions",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_SLOPED_BANK_TRANSITION)json_object_set_new(frames,"gentleSlopeBankedTransitions",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_SLOPED_BANKED_TURN)json_object_set_new(frames,"gentleSlopeBankedTurns",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_BANKED_SLOPE_TRANSITION)json_object_set_new(frames,"flatToGentleSlopeWhileBankedTransitions",json_true());
		if(project->vehicles[i].sprite_flags&SPRITE_CORKSCREW)json_object_set_new(frames,"corkscrews",json_true());
	json_object_set_new(car,"frames",frames);
	json_object_set_new(car,"VEHICLE_ENTRY_FLAG_ENABLE_ADDITIONAL_COLOUR_1",json_true());
	json_object_set_new(car,"VEHICLE_ENTRY_FLAG_ENABLE_ADDITIONAL_COLOUR_2",json_true());
	json_object_set_new(car,"loadingPositions",json_array());
	json_array_append_new(cars,car);
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

//Render sprites
json_t* images_json=json_array();
	for(int i=0;i<3;i++)
	{
	char path[256];
	sprintf(path,"object/images/%d.png",i);
	image_t image;
	image_new(&image,1,1,0,0,0);	
	//Write image file	
	FILE* file=fopen(path,"w");
		if(file)
		{
		image_write_png(&image,file);
		fclose(file);
		}
		else
		{
		printf("Failed to write file %s\n",path);
		return 1;
		}	
	//Write image JSON
	sprintf(path,"images/%d.png",i);
	json_array_append_new(images_json,json_image(path,image.x_offset,image.y_offset,-1,-1,-1,-1));
	}

	for(int i=0;i<project->num_vehicles;i++)
	{
	int num_car_images=count_sprites_from_flags(project->vehicles[i].sprite_flags);
	int num_images=num_car_images*(1+project->vehicles[i].num_rider_models);
	image_t* images=calloc(num_images,sizeof(image_t));
	
	//Render vehicle
	printf("Rendering car sprites\n");
	context_begin_render(context);
	context_add_model(context,project->meshes+project->vehicles[i].model.mesh_index,transform(matrix_identity(),project->vehicles[i].model.position),0); 
	context_finalize_render(context);
	render_vehicle(context,project,i,images);
	context_end_render(context);

		for(int j=0;j<project->vehicles[i].num_rider_models;j++)
		{
		printf("Rendering peep sprites %d\n",j);
		context_begin_render(context);
		context_add_model(context,project->meshes+project->vehicles[i].model.mesh_index,transform(matrix_identity(),project->vehicles[i].model.position),1);
			for(int k=0;k<j;k++)context_add_model(context,project->meshes+project->vehicles[i].riders[k].mesh_index,transform(matrix_identity(),project->vehicles[i].riders[k].position),1);
		context_add_model(context,project->meshes+project->vehicles[i].riders[j].mesh_index,transform(matrix_identity(),project->vehicles[i].riders[j].position),0);
		context_finalize_render(context);
		render_vehicle(context,project,i,images+(j+1)*num_car_images);
		context_end_render(context);
		}

	//Pack images into atlas
	image_t atlas;
	int* x_coords=calloc(num_images,sizeof(int));
	int* y_coords=calloc(num_images,sizeof(int));
	pack_images(images,num_images,&atlas,x_coords,y_coords);
	//Write image json
	char path[256];
	sprintf(path,"images/car_%d.png",i);
		for(int i=0;i<num_images;i++)
		{
		json_array_append_new(images_json,json_image(path,images[i].x_offset,images[i].y_offset,x_coords[i],y_coords[i],images[i].width,images[i].height));
		}
	//Write image file	
	sprintf(path,"object/images/car_%d.png",i);
	FILE* file=fopen(path,"w");
		if(file)
		{
		image_write_png(&atlas,file);
		fclose(file);
		}
		else
		{
		printf("Failed to write file %s\n",path);
		exit(1);
		}
		for(int i=0;i<num_images;i++)image_destroy(images+i);	
	free(images);
	image_destroy(&atlas);
	}

json_object_set_new(json,"images",images_json);

json_dump_file(json,"object/object.json",JSON_INDENT(4));


//Make zip file
char zip_cmd[256];
sprintf(zip_cmd,"cd object&&zip %s%s.parkobj object.json images/*.png",output_directory,project->id);

system(zip_cmd);//Will fail if id contains special characters

/*
//Delete temporary files
struct dirent* dent;
DIR* dir=opendir("object/images");
	if(dir!=NULL)
	{
		while((dent=readdir(dir))!=NULL)
		{
			if(dent->d_type==8)
			{
			char path[256];
			sprintf(path,"object/images/%s",dent->d_name);
			remove(path);
			}
		}
	}
closedir(dir);
remove("object/images");
remove("object/object.json");
remove("object");
*/
return 0;
}
