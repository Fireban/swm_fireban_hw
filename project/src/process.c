#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "GPS.h"

#define MAC_LEN 6
#define MAC_ADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDR_FMT_ARGS(addr) addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define KEY_START 6
#define KEY_NUM 20
#define MAX_BUF 256

#define INIT_URL		"http://www.fireban.kr/api/hw/init"
#define GPS_URL			"http://www.fireban.kr/api/hw/gps"
#define FFMPEG_THERMAL	"ffmpeg -re -i /dev/video4 -vcodec h264_omx -an -s 320x240 -f flv rtmp://www.fireban.kr:1935/tic/"
#define FFMPEG_VIDEO	"ffmpeg -re -i /dev/video0 -vcodec h264_omx -an -s 320x240 -f flv rtmp://www.fireban.kr:1935/app/"

#define TOTAL_FORK 2


char* GetMacAddress(const char *ifname, uint8_t *mac_addr)
{
	struct ifreq ifr;
	int sockfd, ret;
	char* result = malloc(sizeof(char)*16);

	printf("Get interface(%s) MAC address\n", ifname);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		printf("Fail to get interface MAC address - socket() failed - %m\n");
		return "Fail";
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	if(ret < 0) {
		printf("Fail to get interface MAC address - ioctl(SIOCGIFHWADDR) failed - %m\n");
		close(sockfd);
		return "Fail";
	}
	memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, MAC_LEN);

	close(sockfd);

	printf("Success to get Mac address : "MAC_ADDR_FMT"\n", MAC_ADDR_FMT_ARGS(mac_addr));
	sprintf(result, "mac="MAC_ADDR_FMT, MAC_ADDR_FMT_ARGS(mac_addr));
	return result;
}




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
	/* mac address 관련 변수 */
	const char *ifname = "eth0";
	uint8_t mac_addr[MAC_LEN];
	char *mac_address = GetMacAddress(ifname, mac_addr);

	/* key 관련 변수  */
	long response_code;
	char *key_pos;
	char *key_value = malloc(sizeof(char) *KEY_NUM+1);
	char *buff_thermal = malloc(sizeof(char)*MAX_BUF);
	char *buff_video = malloc(sizeof(char)*MAX_BUF);

	/* gps 관련 변수 */
	int gps_response = 0;
	char *gps_data = malloc(sizeof(char)*BUF_SIZE);
	strcpy(GPS_data.LAT, "37.503652");
	strcpy(GPS_data.LONG, "127.044860");
	strcpy(GPS_data.ALT, "100");

	/* multi process 관련 변수  */
	pid_t pids[TOTAL_FORK];
	int runProcess = 0;						// 실행 프로세서 번호


	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;


	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();


	if(curl) {
		
		/* set URL */
		curl_easy_setopt(curl, CURLOPT_URL, INIT_URL);


		while(response_code != 200) {

			memset(&chunk, 0, sizeof(struct MemoryStruct));

			/* recevie data */
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

			/* post data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mac_address);

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
			else {
				printf("other response code\n");
				sleep(10);
			}
		}

		key_pos = strstr(chunk.memory, "key");

		strncat(key_value, (key_pos+KEY_START), KEY_NUM);

		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
	else {
		printf("check curl\n");
		return 0;
	}

	sprintf(buff_thermal, "%s%s", FFMPEG_THERMAL, key_value);
	sprintf(buff_video, "%s%s", FFMPEG_VIDEO, key_value);


	curl_global_cleanup();

	/* multi process 수행  */
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


	/* gps post 실행 */
	while(1) {

		CURL *curl;
		CURLcode res;

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		if(curl) {
			gps_response = getData();
			curl_easy_setopt(curl, CURLOPT_URL, GPS_URL);

			if(gps_response) {
				strcpy(GPS_data.LAT, GGA_data.LAT);
				strcpy(GPS_data.LONG, GGA_data.LONG);
				strcpy(GPS_data.ALT, GGA_data.ALT);
			}
			sprintf(gps_data, "%s&cordinate_x=%s&cordinate_y=%s&alt=%s", mac_address, GPS_data.LAT, GPS_data.LONG, GPS_data.ALT);
			printf("data : %s\n", gps_data);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, gps_data);

			res = curl_easy_perform(curl);

			if(res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
	}

	free(mac_address);
	free(key_value);
	free(buff_thermal);
	free(buff_video);
	free(gps_data);

	return 0;
}
