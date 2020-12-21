#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <jansson.h>
#include "project.h"

#define SQRT_2 1.4142135623731
#define SQRT1_2 0.707106781
#define SQRT_3 1.73205080757
#define SQRT_6 2.44948974278



int count_vehicle_sprites(uint16_t sprites)
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


int load_model(mesh_t* model,json_t* json,const char* name)
{
json_t* mesh=json_object_get(json,name);
	if(mesh!=NULL)
	{
		if(json_is_string(mesh))
		{
		if(mesh_load(model,json_string_value(mesh)))
			{
			printf("Failed to load model \"%s\"\n",json_string_value(mesh));
			return 1;
			}
		return 0;
		}
	printf("Error: Property \"%s\" is not a string\n",name);
	return 1;
	}
printf("Error: Property \"%s\" not found\n",name);
return 2;
}

int load_project(project_t* project,json_t* json)
{
json_t* id=json_object_get(json,"id");
	if(id==NULL||!json_is_string(id))
	{
	printf("Error: No property \"filename\" found\n");
	return 1;
	}
project->id=(uint8_t*)strdup(json_string_value(id));

json_t* name=json_object_get(json,"name");
	if(name==NULL||!json_is_string(name))
	{
	printf("Error: No property \"name\" found\n");
	return 1;
	}
project->name=(uint8_t*)strdup(json_string_value(name));

json_t* description=json_object_get(json,"description");
	if(description==NULL||!json_is_string(description))
	{
	printf("Error: No property \"description\" found\n");
	return 1;
	}
project->description=(uint8_t*)strdup(json_string_value(description));

json_t* capacity=json_object_get(json,"capacity");
	if(capacity==NULL||!json_is_string(capacity))
	{
	printf("Error: No property \"capacity\" found\n");
	return 1;
	}
project->capacity=(uint8_t*)strdup(json_string_value(capacity));

//TODO check if track type is valid
json_t* track_type=json_object_get(json,"track_type");
	if(track_type!=NULL&&json_is_string(track_type))project->track_type=strdup(json_string_value(track_type));
	else
	{
	printf("Error: Property \"track_type\" not found or is not a string\n");
	return 1;
	}

json_t* zero_cars=json_object_get(json,"zero_cars");
	if(zero_cars!=NULL&&json_is_integer(zero_cars))project->zero_cars=json_integer_value(zero_cars);
	else
	{
	printf("Error: Property \"zero_cars\" not found or is not a integer\n");
	return 1;
	}

json_t* min_cars_per_train=json_object_get(json,"min_cars_per_train");
	if(min_cars_per_train!=NULL&&json_is_integer(min_cars_per_train))project->min_cars_per_train=json_integer_value(min_cars_per_train);
	else
	{
	printf("Error: Property \"min_cars_per_train\" not found or is not a integer\n");
	return 1;
	}

json_t* max_cars_per_train=json_object_get(json,"max_cars_per_train");
	if(max_cars_per_train!=NULL&&json_is_integer(max_cars_per_train))project->max_cars_per_train=json_integer_value(max_cars_per_train);
	else
	{
	printf("Error: Property \"max_cars_per_train\" not found or is not a integer\n");
	return 1;
	}

json_t* configuration=json_object_get(json,"configuration");

	if(configuration==NULL||!json_is_object(configuration))
	{
	printf("Error: Property \"configuration\" not found or is not a integer\n");
	return 1;
	}
memset(project->configuration,0xFF,5);
	
json_t* default_car=json_object_get(configuration,"default");
	if(default_car!=NULL&&json_is_integer(default_car))project->configuration[CAR_INDEX_DEFAULT]=json_integer_value(default_car);
	else
	{
	printf("Error: Property \"default\" not found or is not a integer\n");
	return 1;
	}
		
json_t* front_car=json_object_get(configuration,"front");
	if(front_car!=NULL&&json_is_integer(front_car))project->configuration[CAR_INDEX_FRONT]=json_integer_value(front_car);

json_t* second_car=json_object_get(configuration,"second");
	if(second_car!=NULL&&json_is_integer(second_car))project->configuration[CAR_INDEX_SECOND]=json_integer_value(second_car);

json_t* third_car=json_object_get(configuration,"third");
	if(third_car!=NULL&&json_is_integer(third_car))project->configuration[CAR_INDEX_THIRD]=json_integer_value(third_car);

json_t* rear_car=json_object_get(configuration,"rear");
	if(rear_car!=NULL&&json_is_integer(rear_car))project->configuration[CAR_INDEX_REAR]=json_integer_value(rear_car);



json_t* vehicles=json_object_get(json,"vehicles");
	if(vehicles==NULL)printf("Error: Property \"vehicles\" does not exist or is not an array\n");

project->num_sprites=3;

project->num_vehicles=json_array_size(vehicles);

	for(int i=0;i<project->num_vehicles;i++)
	{
	json_t* vehicle=json_array_get(vehicles,i);
	assert(vehicle!=NULL);
		if(!json_is_object(vehicle))
		{
		printf("Warning: Vehicle array contains an element which is not an object - ignoring\n");
		continue;
		}
					
	json_t* spacing=json_object_get(vehicle,"spacing");
		if(spacing!=NULL&&json_is_number(spacing))project->vehicles[i].spacing=json_number_value(spacing);
		else
		{
		printf("Error: Property \"spacing\" not found or is not a number\n");
		return 1;
		}
	
	json_t* mass=json_object_get(vehicle,"mass");
		if(mass!=NULL&&json_is_integer(mass))project->vehicles[i].mass=json_integer_value(mass);
		else
		{
		printf("Error: Property \"mass\" not found or is not a integer\n");
		return 1;
		}
	
		if(load_model(&(project->vehicles[i].mesh),vehicle,"model"))
		{
			for(int j=0;j<i;j++)mesh_destroy(&(project->vehicles[i].mesh));
		return 1;
		}
	project->vehicles[i].sprite_flags=SPRITE_FLAT_SLOPE|SPRITE_GENTLE_SLOPE|SPRITE_STEEP_SLOPE;
	project->vehicles[i].num_sprites=count_vehicle_sprites(project->vehicles[i].sprite_flags);
	project->num_sprites+=project->vehicles[i].num_sprites;
	}

return 0;
}

int main(int argc,char** argv)
{
project_t project;
	
	if(argc!=2)
	{
	printf("Usage: makeride <file>\n");
	return 1;
	}

json_error_t error;
json_t* project_json=json_load_file(argv[1],0,&error);
	if(project_json==NULL)
	{
	printf("Error: %s at line %d column %d\n",error.text,error.line,error.column);
	return 1;
	}

json_t* output_directory=json_object_get(project_json,"output_directory");
	if(output_directory==NULL||!json_is_string(output_directory))
	{
	printf("Error: No property \"output_directory\" found\n");
	return 1;
	}

	if(load_project(&project,project_json))return 1;


context_t context;
light_t lights[9]={
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.25},
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.32},
	{LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1,-1)),1.0},
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.65,-1)),0.8},
	{LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.174},
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.15},
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,1.0,1.0)),0.2},
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.65,0.816,-0.65000000)),0.25},
	{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,-1.0)),0.25},
};
context_init(&context,lights,9,palette_rct2(),TILE_SIZE);

	if(project_export(&project,&context,json_string_value(output_directory)))return 1;

context_destroy(&context);
return 0;
}
