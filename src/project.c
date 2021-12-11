#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <jansson.h>
//#include <zip.h>
#include <image.h>
#include "project.h"


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

json_t* json_image(const char* path,int x,int y)
{
json_t* image=json_object();
json_object_set_new(image,"path",json_string(path));
json_object_set_new(image,"x",json_integer(x));
json_object_set_new(image,"y",json_integer(y));
return image;
}


void render_rotation(context_t* context,int num_frames,float pitch,float roll,float yaw,json_t* images,const char* output_directory)
{
int base_index=json_array_size(images);
	for(int i=0;i<num_frames;i++)
	{
	image_t image;
	context_render_view(context,matrix_mult(rotate_y(yaw+(2*i*M_PI)/num_frames),matrix_mult(rotate_z(pitch),rotate_x(roll))),&image);
	
	char path[256];
	sprintf(path,"object/images/%d.png",base_index+i);
	
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
		exit(1);
		}	
	//Write image JSON
	sprintf(path,"images/%d.png",base_index+i);
	json_array_append_new(images,json_image(path,image.x_offset,image.y_offset));
	//printf("Written %s\n",path);	
	image_destroy(&image);
	}

}

void render_vehicle(context_t* context,mesh_t* meshes,vehicle_t* vehicle,int sprite_flags,json_t* images,const char* output_directory,model_t* peep)
{
	if(sprite_flags&SPRITE_FLAT_SLOPE)
	{
	printf("Rendering flat sprites\n");
        render_rotation(context,32,FLAT,0,0,images,output_directory);
        }
	if(sprite_flags&SPRITE_GENTLE_SLOPE)
	{
	printf("Rendering gentle sprites\n");
	render_rotation(context,4,FG_TRANSITION,0,0,images,output_directory);
	render_rotation(context,4,-FG_TRANSITION,0,0,images,output_directory);
	render_rotation(context,32,GENTLE,0,0,images,output_directory);
	render_rotation(context,32,-GENTLE,0,0,images,output_directory);
	}
	if(sprite_flags&SPRITE_STEEP_SLOPE)
	{
	printf("Rendering steep sprites\n");
	render_rotation(context,8,GS_TRANSITION,0,0,images,output_directory);
	render_rotation(context,8,-GS_TRANSITION,0,0,images,output_directory);
	render_rotation(context,32,STEEP,0,0,images,output_directory);
	render_rotation(context,32,-STEEP,0,0,images,output_directory);
	}
	if(sprite_flags&SPRITE_VERTICAL_SLOPE)
	{
	printf("Rendering vertical sprites\n");
	render_rotation(context,4,SV_TRANSITION,0,0,images,output_directory);
	render_rotation(context,4,-SV_TRANSITION,0,0,images,output_directory);
	render_rotation(context,32,VERTICAL,0,0,images,output_directory);
	render_rotation(context,32,-VERTICAL,0,0,images,output_directory);
	render_rotation(context,4,VERTICAL+M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,-VERTICAL-M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,VERTICAL+2*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,-VERTICAL-2*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,VERTICAL+3*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,-VERTICAL-3*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,VERTICAL+4*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,-VERTICAL-4*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,VERTICAL+5*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,-VERTICAL-5*M_PI_12,0,0,images,output_directory);
	render_rotation(context,4,M_PI,0,0,images,output_directory);
	}
	if(sprite_flags&SPRITE_DIAGONAL_SLOPE)
	{
	printf("Rendering diagonal sprites\n");
	render_rotation(context,4,FG_TRANSITION_DIAGONAL,0,M_PI_4,images,output_directory);
	render_rotation(context,4,-FG_TRANSITION_DIAGONAL,0,M_PI_4,images,output_directory);
	render_rotation(context,4,GENTLE_DIAGONAL,0,M_PI_4,images,output_directory);
	render_rotation(context,4,-GENTLE_DIAGONAL,0,M_PI_4,images,output_directory);
	render_rotation(context,4,STEEP_DIAGONAL,0,M_PI_4,images,output_directory);
	render_rotation(context,4,-STEEP_DIAGONAL,0,M_PI_4,images,output_directory);
	}
        if (sprite_flags & SPRITE_BANKING)
	{
	printf("Rendering banked sprites\n");
	render_rotation(context,8,FLAT,BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,8,FLAT,-BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,32,FLAT,BANK,0,images,output_directory);
	render_rotation(context,32,FLAT,-BANK,0,images,output_directory);
        }
        if (sprite_flags & SPRITE_INLINE_TWIST)
	{
	printf("Rendering inline twist sprites\n");
	render_rotation(context,4,FLAT,3.0 * M_PI_8,0,images,output_directory);
	render_rotation(context,4,FLAT,-3.0 * M_PI_8,0,images,output_directory);
	render_rotation(context,4,FLAT,M_PI_2,0,images,output_directory);
	render_rotation(context,4,FLAT,-M_PI_2,0,images,output_directory);
	render_rotation(context,4,FLAT,5.0 * M_PI_8,0,images,output_directory);
	render_rotation(context,4,FLAT,-5.0 * M_PI_8,0,images,output_directory);
	render_rotation(context,4,FLAT,3.0 * M_PI_4,0,images,output_directory);
	render_rotation(context,4,FLAT,-3.0 * M_PI_4,0,images,output_directory);
	render_rotation(context,4,FLAT,7.0 * M_PI_8,0,images,output_directory);
	render_rotation(context,4,FLAT,-7.0 * M_PI_8,0,images,output_directory);
        }
        if (sprite_flags & SPRITE_SLOPE_BANK_TRANSITION)
	{
	printf("Rendering slope-bank transition sprites\n");
	render_rotation(context,32,FG_TRANSITION,BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,32,FG_TRANSITION,-BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,32,-FG_TRANSITION,BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,32,-FG_TRANSITION,-BANK_TRANSITION,0,images,output_directory);
        }
        if (sprite_flags & SPRITE_DIAGONAL_BANK_TRANSITION)
	{
	printf("Rendering diagonal slope-bank transition sprites\n");
	render_rotation(context,4,GENTLE_DIAGONAL,BANK_TRANSITION,M_PI_4,images,output_directory);
	render_rotation(context,4,GENTLE_DIAGONAL,-BANK_TRANSITION,M_PI_4,images,output_directory);
	render_rotation(context,4,-GENTLE_DIAGONAL,BANK_TRANSITION,M_PI_4,images,output_directory);
	render_rotation(context,4,-GENTLE_DIAGONAL,-BANK_TRANSITION,M_PI_4,images,output_directory);
        }
        if (sprite_flags & SPRITE_SLOPED_BANK_TRANSITION)
	{
	printf("Rendering diagonal sloped bank transition sprites\n");
	render_rotation(context,4,GENTLE,BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,4,GENTLE,-BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,4,-GENTLE,BANK_TRANSITION,0,images,output_directory);
	render_rotation(context,4,-GENTLE,-BANK_TRANSITION,0,images,output_directory);
        }
        if (sprite_flags & SPRITE_SLOPED_BANKED_TURN)
	{
	printf("Rendering sloped banked sprites\n");
	render_rotation(context,32,GENTLE,BANK,0,images,output_directory);
	render_rotation(context,32,GENTLE,-BANK,0,images,output_directory);
	render_rotation(context,32,-GENTLE,BANK,0,images,output_directory);
	render_rotation(context,32,-GENTLE,-BANK,0,images,output_directory);
        }
        if (sprite_flags & SPRITE_BANKED_SLOPE_TRANSITION)
	{
	printf("Rendering banked slope transition sprites\n");
	render_rotation(context,4,FG_TRANSITION,BANK,0,images,output_directory);
	render_rotation(context,4,FG_TRANSITION,-BANK,0,images,output_directory);
	render_rotation(context,4,-FG_TRANSITION,BANK,0,images,output_directory);
	render_rotation(context,4,-FG_TRANSITION,-BANK,0,images,output_directory);
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
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_1),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_2),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_3),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_4),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_5),images,output_directory);

	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_1),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_1),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_2),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_2),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_3),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_3),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_4),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_4),images,output_directory);
	render_rotation(context,4,CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_5),CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_5),images,output_directory);

	// Half corkscrew left
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_1),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_1),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_1),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_2),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_2),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_2),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_3),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_3),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_3),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_4),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_4),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_4),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_5),CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_5),CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_5),images,output_directory);

	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_1),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_1),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_1),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_2),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_2),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_2),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_3),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_3),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_3),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_4),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_4),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_4),images,output_directory);
	render_rotation(context,4,CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_5),CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_5),CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_5),images,output_directory);
        }
context_end_render(context);
}

int project_export(project_t* project,context_t* context,const char* output_directory)
{
//Create working directory
	if(mkdir("object", 0700)==-1)
	{
	printf("Failed to create working directory: %s\n",strerror(errno));
	return 1;
	}
//Create image directory
	if(!mkdir("object/images", 0700)==-1)
	{
	printf("Failed to create image directory: %s\n",strerror(errno));
	return 1;
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
json_t* images=json_array();
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
	json_array_append_new(images,json_image(path,image.x_offset,image.y_offset));
	}

	for(int i=0;i<project->num_vehicles;i++)
	{
	//Render vehicle
	printf("Rendering car sprites\n");
	context_begin_render(context);
	context_add_model(context,project->meshes+project->vehicles[i].model.mesh_index,transform(matrix_identity(),project->vehicles[i].model.position),0); 
	context_finalize_render(context);
	render_vehicle(context,project->meshes,project->vehicles+i,project->vehicles[i].sprite_flags,images,output_directory,NULL);
	
		for(int j=0;j<project->vehicles[i].num_rider_models;j++)
		{
		printf("Rendering peep sprites %d\n",j);
		context_begin_render(context);
		context_add_model(context,project->meshes+project->vehicles[i].model.mesh_index,transform(matrix_identity(),project->vehicles[i].model.position),1);
			for(int k=0;k<j;k++)context_add_model(context,project->meshes+project->vehicles[i].riders[k].mesh_index,transform(matrix_identity(),project->vehicles[i].riders[k].position),1);
		context_add_model(context,project->meshes+project->vehicles[i].riders[j].mesh_index,transform(matrix_identity(),project->vehicles[i].riders[j].position),0);
		context_finalize_render(context);
		render_vehicle(context,project->meshes,project->vehicles+i,project->vehicles[i].sprite_flags,images,output_directory,project->vehicles[i].riders+j);
		}
	}

json_object_set_new(json,"images",images);

json_dump_file(json,"object/object.json",JSON_INDENT(4));


//Make zip file
char zip_cmd[256];
sprintf(zip_cmd,"cd object&&zip %s%s.parkobj object.json images/*.png",output_directory,project->id);

system(zip_cmd);//Will fail if id contains special characters

/*
int err=0;
struct zip_error error;
zip_error_init(&error);

zip_source_t *src=zip_source_file_create(zip_path,0,-1,&error);
	if(src==NULL)
	{
	printf("Failed to open zip archive %s: %s\n",zip_path,zip_error_strerror(&error));
        return 1;
    	}

zip_t* zip=zip_open_from_source(src,ZIP_CREATE|ZIP_TRUNCATE,&error);
	if(zip==NULL)
	{
        zip_source_free(src);
	printf("Failed to open zip archive %s: %s\n",zip_path,zip_error_strerror(&error));
        return 1;
   	}

/*
zip_t* zip=zip_open(zip_path,ZIP_CREATE|ZIP_TRUNCATE,&err);
	
	if(zip==NULL)
	{
	zip_error_t ze;
	zip_error_init_with_code(&ze,err);
	printf("Failed to open zip archive %s: %s\n",zip_path,zip_error_strerror(&ze));
	return 1;
	}
	if(zip_dir_add(zip,"images",ZIP_FL_ENC_UTF_8)==-1)
	{
	printf("Failed to add images directory: %s\n",zip_strerror(zip));
	}

	if(zip_close(zip)==-1)
	{
	printf("Failed to write zip file: %s\n",zip_strerror(zip));
	}
printf("Done\n");
*/

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

return 0;
}
