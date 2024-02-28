/* ***************************************************************************
 *
 * Thomas Schmidt, 2020, 2024
 *
 * This file is part of the Fused ShaperHub Project
 *
 * here the HTTPS interface to the ShaperHub (shapertools.com) cloud service is implemted
 * Informations come in via HTTPS from the ShaperHub Cloud API are handled here
 * and forwarded to the data layer
 * Vice versa the data layer might push information upstream to the
 * ShaperHub API
 *
 * License: Not Defined Yet
 *
 * Project URL: https://github.com/tseiman/Fused-ShaperHub
 *
 ************************************************************************** */

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <global.h>
#include <messages.h>
#include <alloc.h>
#include <http-connector.h>

#define BASEURL "http://localhost:3000"
// #define MAX_PATH_LEN 512

#define STRLEN(s) (sizeof(s) / sizeof(s[0]))

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct_t *mem = (MemoryStruct_t *)userp;

    char *ptr = REALLOC(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        /* out of memory! */
        LOG_ERR("not enough memory (realloc returned NULL)");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int checkPathLen(char *str) {
    if (strnlen(str, MAX_PATH_LEN - STRLEN(BASEURL)) < 1) {
        LOG_ERR("Empty string given, cant proceed: %s", str);
        return -1;
    }
    if (strnlen(str, MAX_PATH_LEN - STRLEN(BASEURL) + 10) > (MAX_PATH_LEN - STRLEN(BASEURL))) {
        LOG_INFO("Too long path given, cant proceed: %s", str);
        return -2;
    }
    return 0;
}

int fsh_HTTPListPath(char *path, MemoryStruct_t **responseBuffer) {

    CURL *curl;
    CURLcode res;
    MemoryStruct_t *chunk = MALLOC(sizeof(MemoryStruct_t));

    int result = -1;


//    chunk.memory = MALLOC(1); /* will be grown as needed by the realloc above */
    chunk->memory = NULL; /* will be grown as needed by the realloc above */
    chunk->size = 0;           /* no data at this point */
    LOG_INFO("fsh_ListPath %s", path);

//    char pathbuffer[MAX_PATH_LEN] = BASEURL;

    char *pathbuffer = MALLOC(strlen(BASEURL) + strlen(path) + 1);
    
    if(!pathbuffer) {
        LOG_ERR("not enough memory fsh_HTTPListPath pathbuffer allocation");
        result = -3;
        goto ERROR; 
    }

    if (checkPathLen(path)) {
        result = -4;
        goto ERROR;
    }
    
    strcpy(pathbuffer,BASEURL);
   // strncat(pathbuffer, path, (MAX_PATH_LEN - STRLEN(BASEURL) + 1));
    strcat(pathbuffer, path);

    LOG_DEBUG("fsh_ListPath() Setting path to: %s", pathbuffer);
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        LOG_ERR("curl_easy_init() failed");
        goto ERROR;
    }

    curl_easy_setopt(curl, CURLOPT_URL, pathbuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, chunk);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK) {
        LOG_ERR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        return res;
    }
    /* always cleanup */
    curl_easy_cleanup(curl);

    chunk->memory[chunk->size] = '\0';
    *responseBuffer = chunk;
    result = 0;
    
    goto EXIT;
ERROR:
    if(!chunk) FREE(chunk);
    chunk = NULL;
EXIT:
    if(!pathbuffer) FREE(pathbuffer);
    pathbuffer = NULL; 
    
    curl_global_cleanup();

    return result;
}
