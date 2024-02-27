/*
 * This file provides just some hardcoded definitions and information to have 
 * a mocup running
 *
 */

#ifndef HAVE_MOCUP_H
#define HAVE_MOCUP_H 1

#warning "Using MOCUP Setup !"

#define MOC_DATA_STRUCTURE \
    "{\n"\
	"\"Local_Action\" : { \"type\": \"folder\", \"onlinepath\": \"local-action\", \"atime\": 1580428000, \n"\
	    "\"actionOn_ObsLightValue\": {\"type\": \"folder\", \"onlinepath\": \"actionOn_ObsLightValue\",  \"atime\": 1580428001, \n"\
		"\"recent\": {\"type\": \"reference\", \"reference\": \"v1\", \"atime\": 1580428002 },\n"\
		"\"v1\": {\"type\": \"folder\", \"version\": 1, \"atime\": 1580428003, \n"\
		    "\"actionOn_ObsLightValue.js\" : {\"type\": \"file\", \"atime\": 1580428004, \"size\" : 123},\n"\
		    "\".options\" : {\"type\": \"file\", \"atime\": 1580428005, \"size\" : 123}\n"\
		"}\n"\
	    "}\n"\
	"},\n"\
	"\"Cloud_Action\" : { \"type\": \"folder\", \"onlinepath\": \"cloud-action\", \"atime\": 1580428006, \n"\
	    "\"TS_actionOn_ObsLightValueSuperCloudAction\": {\"type\": \"folder\", \"onlinepath\": \"TS_actionOn_ObsLightValueSuperCloudAction\",  \"atime\": 1580428007, \n"\
		"\"recent\": {\"type\": \"reference\", \"reference\": \"v2\", \"atime\": 1580428009 },\n"\
		"\"v1\": {\"type\": \"folder\", \"version\": 1, \"atime\": 1580428010, \n"\
		    "\"TS_actionOn_ObsLightValueSuperCloudAction.js\" : {\"type\": \"file\", \"atime\": 1580428011, \"size\" : 123},\n"\
		    "\".options\" : {\"type\": \"file\", \"atime\": 1580428012, \"size\" : 123 }\n"\
		"},\n"\
		"\"v2\": {\"type\": \"folder\", \"version\": 2, \"atime\": 1580428013, \n"\
		    "\"TS_actionOn_ObsLightValueSuperCloudAction.js\" : {\"type\": \"file\", \"atime\": 1580428014, \"size\" : 123 },\n"\
		    "\".options\" : {\"type\": \"file\", \"atime\": 1580428015, \"size\" : 123 }\n"\
		"}\n"\
	    "}\n"\
	"}\n"\
    "}\n"


#define MOC_FILE \
"function(event) {\n"\
"\tvar light = event.value;\n"\
"\tvar light_threshold = Datahub.read('/virtual/light_threshold/value', 0);\n"\
"\tvar led_on = true;\n"\
"\tif (light >= light_threshold.value)  {\n"\
"        led_on = false;\n"\
"\t}\n"\
"\n"\
"\treturn {\n"\
"    \t\"dh://io/IOT0_GPIO1/value\":[!led_on],\n"\
"\t\t\"cl://\":['LED is ON: '+led_on]\n"\
"\n"\
"   \t}"\
"\n"\
"}\n"


#define MOC_OPTION \
"{\n"\
"   observation : 'obs_light_value',\n"\
"   enabled: true\n"\
"}\n"


#define MOCFILES_ATTR \
	if(strcmp(path, filepath) == 0) { \
	    stbuf->st_size = strlen(filecontent); \
	} \
	if(strcmp(path, filepath1) == 0) { \
	    stbuf->st_size = strlen(filecontent1); \
	} \

#define MOCUP_FILE_LOAD(moc_path, moc_filepath, moc_filecontent, moc_offset, moc_size, moc_buffer) \
    if (strcmp(moc_path, moc_filepath) == 0) { \
	size_t len = strlen(moc_filecontent); \
	if (moc_offset >= len) { \
    	    return 0; \
	} \
	if (moc_offset + moc_size > len) { \
    	    memcpy(moc_buffer, moc_filecontent + moc_offset, len - moc_offset); \
    	    return len - moc_offset; \
	} \
	memcpy( moc_buffer, moc_filecontent + moc_offset, moc_size); \
	return moc_size; \
    }

#define MOC_DEMO_FILES \
static const char *filepath = "/Local_Action/actionOn_ObsLightValue/v1/actionOn_ObsLightValue.js"; \
static const char *filecontent = MOC_FILE; \
static const char *filepath1 = "/Local_Action/actionOn_ObsLightValue/v1/.options"; \
static const char *filecontent1 = MOC_OPTION; 

#endif
