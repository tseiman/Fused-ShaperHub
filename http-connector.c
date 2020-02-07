#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#define BASEURL "http://localhost:3000/"
#define MAX_PATH_LEN 512

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))


struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
 


int checkPathLen(char *str) {
    if(strnlen(str,MAX_PATH_LEN - STRLEN(BASEURL)) < 1) {
	fprintf(stderr, "Empty string given, cant proceed: %s\n", str);
	return -1;
    }
    if(strnlen(str,MAX_PATH_LEN - STRLEN(BASEURL) + 10) > (MAX_PATH_LEN - STRLEN(BASEURL))) {
	fprintf(stderr, "Too long path given, cant proceed: %s\n", str);
	return -2;
    }
    return 0;
}

int getLocalAction(char *id, char **buffer) {

    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */ 

    char pathbuffer[MAX_PATH_LEN] = BASEURL;

    if(checkPathLen(id)) {
	return -1;
    }
    strncat(pathbuffer,id, (MAX_PATH_LEN - STRLEN(BASEURL)));

    curl = curl_easy_init();
    if(curl) {
	curl_easy_setopt(curl, CURLOPT_URL, pathbuffer);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
	 /* we pass our 'chunk' struct to the callback function */ 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(curl);
	/* Check for errors */ 
	if(res != CURLE_OK) {
	    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	    return res;
	}
	/* always cleanup */ 
	curl_easy_cleanup(curl);
	*buffer = chunk.memory;
    }
    return 0;
}
