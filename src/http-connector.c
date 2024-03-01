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

/*

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}
*/

size_t escapeURL(char * buffer, char **newBufferIn, size_t len) {
    const char URLEncodingLookup[] = SUBSITUTE_LOOKUP;
    size_t newSize = len + 1;
    int newOffset = 0;

    char *newBuffer = MALLOC(len);                              // we have to copy the original buffer to the new. 
                                                                // Some characters are expanded to %XX (Hex) 
                                                                // - so initially the new buffer is minimum that long than old 
                                                                
    if(!newBuffer) return 0;

    for(int i; i <= newSize + 1; ++i) {
        if(URLEncodingLookup[buffer[i]]) {
            newBuffer[i + newOffset] = buffer[i];
        } else {
            if(buffer[i] == '\0') {
                newBuffer[i + newOffset]  = buffer[i];
                break;
            }
            newSize += 3;
            
            char *tmpBuffer = REALLOC(newBuffer, newSize);
            if(!tmpBuffer) {
                FREE(newBuffer);
                newBuffer = NULL;
                return 0;
            }
            newBuffer = tmpBuffer;

            sprintf(newBuffer + i + newOffset, "%%%02X",buffer[i]);
            newOffset += 2;

        }
    } 
    *newBufferIn = newBuffer;
    return newSize;
}

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



/** *************************************************************************
 * Requests the folder listing for a specific folder
 * 
 * Parameter:
 *      char *path                      --> the path of the folder
 *      MemoryStruct_t **responseBuffer --> 
 * ************************************************************************ **/
int fsh_httpconnector_ListPath(char *path, MemoryStruct_t **responseBuffer) {

    LOG_DEBUG("requesting >%s< from remote resource.", path);

    CURL *curl;
    CURLcode res;
    int result = -1;
    MemoryStruct_t *chunk = MALLOC(sizeof(MemoryStruct_t));
    if(!chunk) goto ERROR;

    chunk->memory = NULL; /* will be grown as needed by the realloc above */
    chunk->size = 0;           /* no data at this point */

    char *escapedURLBuffer = NULL;
    int newUrlSize = escapeURL(path, &escapedURLBuffer, strnlen(path,MAX_PATH_LEN));
    if(!newUrlSize) {
        LOG_ERR("Filed to escape URL");
        result = -5;
        goto ERROR;
    }
    
    LOG_DEBUG("Escaped URL: >%s<", escapedURLBuffer);

    size_t pathbuffer_len = strnlen(BASEURL,MAX_PATH_LEN) + strnlen(escapedURLBuffer,MAX_PATH_LEN - strnlen(BASEURL,MAX_PATH_LEN)) + 1;
    char *pathbuffer = MALLOC(pathbuffer_len);
    if(!pathbuffer) goto ERROR; 


    if (checkPathLen(path)) {
        result = -4;
        goto ERROR;
    }

    strncpy(pathbuffer,BASEURL, pathbuffer_len);
    strncat(pathbuffer, escapedURLBuffer, pathbuffer_len - STRLEN(BASEURL) + 1);

    LOG_DEBUG("asembled path to: >%s<", pathbuffer);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        LOG_ERR("curl_easy_init() failed");
        goto ERROR;
    }

    curl_easy_setopt(curl, CURLOPT_URL, pathbuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, chunk);                               // pass 'chunk' struct to the callback function

    res = curl_easy_perform(curl);                                                  // Perform the request 

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        LOG_ERR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        goto ERROR;
    }

    chunk->memory[chunk->size] = '\0';
    *responseBuffer = chunk;
    result = 0;
    
    goto EXIT;
ERROR:
    LOG_ERR("Leaving in an error condition.");

    if(chunk) FREE(chunk->memory);
    FREE(chunk);

EXIT:
    FREE(pathbuffer);
    FREE(escapedURLBuffer);
   
    curl_global_cleanup();

    return result;
}
