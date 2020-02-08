
#include <string.h>
#include <jansson.h>

#include "data-container.h"

/* demo data structure */

#define DATA_STRUCTURE \
    "[\n"\
	"{ \"type\": \"folder\", \"name\": \"Local_Action\", \"onlinepath\": \"local-action\", \"atime\": 1580428873, \"subnodes\": [\n"\
	    "{\"type\": \"folder\", \"name\": \"actionOn_ObsLightValue\", \"atime\": 1580428873, \"subnodes\": [\n"\
		"{\"type\": \"reference\", \"name\": \"recent\", \"reference\": 1, \"atime\": 1580428873, \"subnodes\": null},\n"\
		"{\"type\": \"folder\", \"name\": \"v1\", \"version\": 1, \"atime\": 1580428873, \"subnodes\": [\n"\
		    "{\"type\": \"file\", \"name\": \"actionOn_ObsLightValue\", \"atime\": 1580428873, \"subnodes\": null},\n"\
		    "{\"type\": \"file\", \"name\": \".options\", \"atime\": 1580428873, \"subnodes\": null}\n"\
		"]}\n"\
	    "]}\n"\
	" ]},\n"\
	"{ \"type\": \"folder\", \"name\": \"Cloud_Action\", \"onlinepath\": \"action\", \"atime\": 1580428873, \"subnodes\": [\n"\
	    "{\"type\": \"folder\", \"name\": \"TS_actionOn_ObsLightValueSuperCloudAction\", \"atime\": 1580428873, \"subnodes\": [\n"\
		"{\"type\": \"reference\", \"name\": \"recent\", \"reference\": 1, \"atime\": 1580428873, \"subnodes\": null},\n"\
		"{\"type\": \"folder\", \"name\": \"v1\", \"version\": 1, \"atime\": 1580428873, \"subnodes\": [\n"\
		    "{\"type\": \"file\", \"name\": \"actionOn_ObsLightValue\", \"atime\": 1580428873, \"subnodes\": null},\n"\
		    "{\"type\": \"file\", \"name\": \".options\", \"atime\": 1580428873, \"subnodes\": null}\n"\
		"]}\n"\
	    "]}\n"\
	" ]}\n"\
    "]\n"


json_t *setGetRoot(json_t * newRoot) {
    static json_t *root = NULL;
    if(newRoot != NULL) {
	root = newRoot;
    }
    return root;
}



int oct_setupDataStructure() {
    json_error_t error;
    json_t *root = NULL;
    root = json_loads(DATA_STRUCTURE, 0, &error);

    if(!root) {
	fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
	return 1;
    }
    setGetRoot(root);

    return 0;
}


// int oct_walkFolders(int (*callback)(struct Oct_DirLoaderRef_s *ref), void *ref) {
int oct_walkFolders(WalkFolders_Callback_t callback, struct Oct_DirLoaderRef_s *ref) {

    unsigned int i;
    json_t *root = setGetRoot(NULL);
    
    if(!root) {
	fprintf(stderr, "error: cant walk through folders if data is not loaded\n");
	return -1;
    }

    if(!json_is_array(root)) {
	fprintf(stderr, "error: data root is not an array\n");
//	json_decref(root);
	return -2;
    }

    for(i = 0; i < json_array_size(root); i++) {
	json_t *data, *type, *dir_name;

	data = json_array_get(root, i);
	if(!json_is_object(data)) {
	    fprintf(stderr, "error: commit data %d is not an object\n", i + 1);
//	    json_decref(root);
	    return -3;
	}

	type = json_object_get(data, "type");
	dir_name = json_object_get(data, "name");
	if(json_is_string(type) && json_is_string(dir_name)) {
	    if (strcmp(json_string_value(type), "folder") == 0) {
		
		ref->filename = json_string_value(dir_name);
		callback(ref);
//		printf("found folder: %s\n", json_string_value(dir_name));
	    }

	} 
    }
    return 0;
}


