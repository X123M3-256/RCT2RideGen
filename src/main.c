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


int load_mesh(mesh_t* model,json_t* mesh)
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
printf("Error: Mesh path is not a string\n");
return 1;
}

int load_vector(vector3_t* vector,json_t* json,char* name)
{
json_t* array=json_object_get(json,name);
	if(array==NULL||!json_is_array(array))
	{
	printf("Property \"%s\" not found or is not an array\n",name);
	return 1;
	}

int size=json_array_size(array);
	if(size!=3)
	{
	printf("Vector must have 3 components\n");
	return 1;
	}

json_t* x=json_array_get(array,0);
json_t* y=json_array_get(array,1);
json_t* z=json_array_get(array,2);

	if(!json_is_number(x)||!json_is_number(y)||!json_is_number(z))
	{
	printf("Vector components must be numeric\n");
	return 1;
	}
vector->x=json_number_value(x);
vector->y=json_number_value(y);
vector->z=json_number_value(z);
return 0;
}

int load_model(model_t* model,json_t* json,int num_meshes)
{
	if(model==NULL||!json_is_object(json))
	{
	printf("Property \"model\" not found or is not an object\n");
	return 1;
	}

json_t* mesh=json_object_get(json,"mesh_index");
	if(mesh==NULL||!json_is_integer(mesh))
	{
	printf("Error: Property \"mesh_index\" not found or is not an integer\n");
	return 1;
	}

model->mesh_index=json_integer_value(mesh);
	if(model->mesh_index>=num_meshes||model->mesh_index<0)
	{
	printf("Mesh index %d is out of bounds\n",model->mesh_index);
	return 1;
	}

	if(load_vector(&(model->position),json,"position"))return 1;
	if(load_vector(&(model->orientation),json,"orientation"))return 1;
return 0;
}

int load_lights(light_t** lights_array,int* lights_count,json_t* json)
{
int num_lights=json_array_size(json);
light_t* lights=calloc(num_lights,sizeof(light_t));
	for(int i=0;i<num_lights;i++)
	{
	json_t* light=json_array_get(json,i);
	assert(light!=NULL);
		if(!json_is_object(light))
		{
		printf("Warning: Light array contains an element which is not an object - ignoring\n");
		continue;
		}
	
	json_t* type=json_object_get(light,"type");
		if(type==NULL||!json_is_string(type))
		{
		printf("Error: Property \"type\" not found or is not a string\n");
		return 1;
		}
	const char* type_value=json_string_value(type);
		if(strcmp(type_value,"diffuse")==0)lights[i].type=LIGHT_DIFFUSE;
		else if(strcmp(type_value,"specular")==0)lights[i].type=LIGHT_SPECULAR;
		else
		{
		printf("Unrecognized light type \"%s\"\n",type);
		free(lights);
		}
	json_t* shadow=json_object_get(light,"shadow");
		if(shadow==NULL||!json_is_boolean(shadow))
		{
		printf("Error: Property \"shadow\" not found or is not a boolean\n");
		return 1;
		}
		if(json_boolean_value(shadow)==JSON_TRUE)lights[i].shadow=1;
		else lights[i].shadow=0;

		if(load_vector(&(lights[i].direction),light,"direction"))return 1;
	lights[i].direction=vector3_normalize(lights[i].direction);
	
	json_t* strength=json_object_get(light,"strength");
		if(strength==NULL||!json_is_number(strength))
		{
		printf("Error: Property \"strength\" not found or is not a number\n");
		return 1;
		}
	lights[i].intensity=json_number_value(strength);
	}
*lights_count=num_lights;
*lights_array=lights;
return 0;
}


int load_project(project_t* project,json_t* json)
{
json_t* id=json_object_get(json,"id");
	if(id==NULL||!json_is_string(id))
	{
	printf("Error: No property \"id\" found\n");
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
json_t* ride_type=json_object_get(json,"ride_type");
	if(ride_type!=NULL&&json_is_string(ride_type))project->ride_type=strdup(json_string_value(ride_type));
	else
	{
	printf("Error: Property \"ride_type\" not found or is not a string\n");
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





//Load meshes
json_t* meshes=json_object_get(json,"meshes");
	if(meshes==NULL)printf("Error: Property \"meshes\" does not exist or is not an array\n");
project->num_meshes=json_array_size(meshes);
	for(int i=0;i<project->num_meshes;i++)
	{
	json_t* mesh=json_array_get(meshes,i);
	assert(mesh!=NULL);
		if(load_mesh(project->meshes+i,mesh))
		{
			for(int j=0;j<i;j++)mesh_destroy(project->meshes+i);
		return 1;
		}
	}


//Load vehicles
json_t* vehicles=json_object_get(json,"vehicles");
	if(vehicles==NULL||!json_is_array(vehicles))printf("Error: Property \"vehicles\" does not exist or is not an array\n");
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
	
	json_t* draw_order=json_object_get(vehicle,"draw_order");
		if(draw_order!=NULL&&json_is_integer(draw_order))project->vehicles[i].draw_order=json_integer_value(draw_order);
		else
		{
		printf("Error: Property \"draw_order\" not found or is not a integer\n");
		return 1;
		}
	//Load car model
		if(load_model(&(project->vehicles[i].model),json_object_get(vehicle,"model"),project->num_meshes))
		{
		return 1;
		}
			
	//Load rider models
	json_t* riders=json_object_get(vehicle,"riders");
		if(riders!=NULL&&json_is_array(riders))//TODO fail if riders exists but is not array
		{
		json_t* num_riders=json_object_get(vehicle,"capacity");
			if(num_riders==NULL||!json_is_integer(num_riders))
			{
			printf("Error: Property \"capacity\" not found or is not an integer\n");
			return 1;
			}
		project->vehicles[i].num_riders=json_integer_value(num_riders);


		project->vehicles[i].num_rider_models=json_array_size(riders);
			for(int j=0;j<project->vehicles[i].num_rider_models;j++)
			{
				if(load_model(project->vehicles[i].riders+j,json_array_get(riders,j),project->num_meshes))return 1;

			}
		
		}
		else
		{
		project->vehicles[i].num_riders=0;
		project->vehicles[i].num_rider_models=0;
		}

	project->vehicles[i].sprite_flags=SPRITE_FLAT_SLOPE|SPRITE_GENTLE_SLOPE|SPRITE_STEEP_SLOPE|SPRITE_VERTICAL_SLOPE|SPRITE_DIAGONAL_SLOPE|SPRITE_BANKING|SPRITE_INLINE_TWIST|SPRITE_SLOPE_BANK_TRANSITION|SPRITE_DIAGONAL_BANK_TRANSITION|SPRITE_SLOPED_BANK_TRANSITION|SPRITE_SLOPED_BANKED_TURN|SPRITE_BANKED_SLOPE_TRANSITION|SPRITE_CORKSCREW;
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
	printf("Error: Property \"output_directory\" not found or is not a string\n");
	return 1;
	}

//light_t* lights;
//int num_lights;
json_t* light_array=json_object_get(project_json,"lights");
	if(light_array==NULL||!json_is_array(light_array))
	{
	printf("Error: Property \"lights\" not found or is not an array\n");
	return 1;
	}

//	if(load_lights(&lights,&num_lights,light_array))return 1;
	if(load_project(&project,project_json))return 1;

context_t context;


int num_lights=8;
light_t lights[8]={
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.1},//Bottom
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,0.5,-1.0)),0.6},//Front right
{LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1.65,-1)),0.8},//Main spec
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,1.7,-1)),0.8},//Main diffuse
{LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.45},//Top
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.85,1.0)),0.475},//Front left
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.75,0.4,-1.0)),0.6},//Top right
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.25,0)),0.5},//Back left
};

context_init(&context,lights,num_lights,palette_rct2(),TILE_SIZE);



	if(project_export(&project,&context,json_string_value(output_directory)))return 1;

context_destroy(&context);
return 0;
}
