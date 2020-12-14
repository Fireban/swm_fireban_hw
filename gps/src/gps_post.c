#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <curl/curl.h>

#include "GPS.h"

#define HTTP_URL	"http://www.fireban.kr/api/hw/gps"
#define MAC_ADD		"mac=dc:a6:32:bb:aa:f9"

int main( void)
{
	int result = 0;
	char *data = malloc(sizeof(char)*BUF_SIZE);

	CURL *curl;
	CURLcode res;
	
	curl_global_init(CURL_GLOBAL_ALL);


	while(1) {
		curl = curl_easy_init();

		if(curl) {
			result = getData();
			curl_easy_setopt(curl, CURLOPT_URL, HTTP_URL);

			if(result) {
				sprintf(data, "%s&cordinate_x=%s&cordinate_y=%s&alt=%s", MAC_ADD, GGA_data.LAT, GGA_data.LONG, GGA_data.ALT);
				printf("data : %s\n", data);

				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

				res = curl_easy_perform(curl);

				if(res != CURLE_OK)
					fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

				sleep(5);
			}
			else {
				printf("waiting for gps data...\n");
				sprintf(data, "%s&cordinate_x=127.044860&cordinate_x=37.503652&alt=100", MAC_ADD);

				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
				res = curl_easy_perform(curl);

				if(res != CURLE_OK)
					fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

				sleep(5);
			}

			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
	}

	free(data);
	return 0; 
}
