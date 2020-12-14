#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define KEY_POS 6
#define KEY_NUM 20
#define MAX_BUF 256

#define MAC_address		"mac=dc:a6:32:bb:aa:f9"
#define HTTP_URL		"http://www.fireban.kr/api/hw/init"
#define FFMPEG_THERMAL	"ffmpeg -re -i /dev/video4 -vcodec libx264 -an -s 320x240 -f flv rtmp://www.fireban.kr:1935/tic/"
#define FFMPEG_VIDEO	"ffmpeg -re -i /dev/video0 -vcodec libx264 -an -s 320x240 -f flv rtmp://www.fireban.kr:1935/app/"

#define TOTAL_FORK 2

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void * contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);

	if(!ptr) {
		printf("not enough memory (realloc returned NULL)\n");
    	return 0;
	}
	
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int main(void)
{
	CURL *curl;
	CURLcode res;
	long response_code;

	char *key_value = malloc(sizeof(char) *KEY_NUM+1);

	pid_t pids[TOTAL_FORK], pid;
	int runProcess = 0;						// 실행 프로세서 번호

	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();


	if(curl) {

		/* set URL */
		curl_easy_setopt(curl, CURLOPT_URL, HTTP_URL);


		while(response_code != 200) {

			memset(&chunk, 0, sizeof(struct MemoryStruct));

			/* recevie data */
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

			/* post data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, MAC_address);

			/* curl 실행*/
			res = curl_easy_perform(curl);


			if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			else
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);


			/* 해당 장비가 등록되지 않은 경우 */
			if(response_code == 201) {
				printf("Waitting for device registration...\n");
				sleep(10);
			}
			/* 장비가 등록되어 있는데 승인이 안된 경우 */
			else if(response_code == 401) {	
				printf("Active waiting...\n");
				sleep(10);
			}
		}

		char *pos;
		pos = strstr(chunk.memory, "key");

		strncat(key_value, (pos+KEY_POS), KEY_NUM);

		curl_easy_cleanup(curl);
		free(chunk.memory);
	}

	char *buff_thermal = malloc(sizeof(char)*MAX_BUF);
	char *buff_video = malloc(sizeof(char)*MAX_BUF);

	sprintf(buff_thermal, "%s%s", FFMPEG_THERMAL, key_value);
	sprintf(buff_video, "%s%s", FFMPEG_VIDEO, key_value);


	while(runProcess < TOTAL_FORK) {
		pids[runProcess] = fork();
		if(pids[runProcess] == -1) {
			printf("errors... \n");
			exit(0);
		}
		else if(pids[runProcess] == 0) {
			switch(runProcess) {
				case 0:
					system(buff_thermal);
					break;
				case 1:
					system(buff_video);
					break;
			}
		}
		else;

		runProcess++;
	}

	free(key_value);

	curl_global_cleanup();

	return 0;
}
