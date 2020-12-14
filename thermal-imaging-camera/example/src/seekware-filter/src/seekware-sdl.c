/*Copyright (c) [2019] [Seek Thermal, Inc.]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The Software may only be used in combination with Seek cores/products.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     Seek Thermal SDK Demo
 * Purpose:     Demonstrates how to image Seek Thermal Cameras with SDL2
 * Author:      Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include <seekware/seekware.h>

#define FEATURE_V4L2_STREAMING
#ifdef FEATURE_V4L2_STREAMING  // by sulac
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <assert.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdint.h>
#include <curl/curl.h>

#define MAC_LEN 6
#define MAC_ADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDR_FMT_ARGS(addr) addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define VIDEO_DEVICE "/dev/video4"
#define FRAME_FORMAT V4L2_PIX_FMT_ARGB32
#endif

#ifdef _WIN32
#define strcasecmp _stricmp
#define inline __inline
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#ifdef __APPLE__
#include <SDL.h>
#endif

#ifdef __linux__
#include <sys/utsname.h>
#include <SDL2/SDL.h>
#endif

#define NUM_CAMS           5
#define SCALE_FACTOR       2
#define DRAW_RETICLE       true


typedef enum display_mode_t {
	DISPLAY_ARGB  = 0,
	DISPLAY_THERMAL = 1,
	DISPLAY_FILTERED = 2,
	DISPLAY_PERSON_FILTERED = 3
} display_mode_t;

typedef struct display_lut_t {
	const char * name;
	sw_display_lut value;
}display_lut_t;

typedef struct fire_rect {
	size_t min_x;
	size_t min_y;
	size_t max_x;
	size_t max_y;
}fire_rect;

static display_lut_t lut_names[] = {
	{ "white",  SW_LUT_WHITE    },
	{ "black",  SW_LUT_BLACK    },
	{ "iron",   SW_LUT_IRON     },
	{ "cool",   SW_LUT_COOL     },
	{ "amber",  SW_LUT_AMBER    },
	{ "indigo", SW_LUT_INDIGO   },
	{ "tyrian", SW_LUT_TYRIAN   },
	{ "glory",  SW_LUT_GLORY    },
	{ "envy",   SW_LUT_ENVY     }
};

bool exit_requested = false;
display_mode_t display_mode = DISPLAY_FILTERED;
sw_display_lut current_lut = SW_LUT_WHITE;

char result[22];

char* GetMacAddress(const char *ifname, uint8_t *mac_addr)
{
	struct ifreq ifr;
	int sockfd, ret;
	//char* result = malloc(sizeof(char)*16);

	printf("Get interface(%s) MAC address\n", ifname);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		printf("Fail to get Mac address - socket() failed %m\n");
		return "Fail";
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	if(ret < 0) {
		printf("Fail to get Mac address - ioctl(SIOCGIFHWADDR) failed %m\n");
		close(sockfd);
		return "Fail";
	}
	memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, MAC_LEN);

	close(sockfd);

	printf("Success to get Mac address : "MAC_ADDR_FMT"\n", MAC_ADDR_FMT_ARGS(mac_addr));
	sprintf(result, MAC_ADDR_FMT, MAC_ADDR_FMT_ARGS(mac_addr));
	return result;
}


static inline void print_time(void) {
	static time_t t = 0;
	static struct tm* timeptr = NULL;
	time(&t);
	timeptr = localtime(&t);
	printf("\n%s", asctime(timeptr));
}

static inline void print_fw_info(psw camera) {
	sw_retcode status; 
	int therm_ver;

	printf("Model Number:%s\n",camera->modelNumber);
	printf("SerialNumber: %s\n", camera->serialNumber);
	printf("Manufacture Date: %s\n", camera->manufactureDate);

	printf("Firmware Version: %u.%u.%u.%u\n",
			camera->fw_version_major,
			camera->fw_version_minor,
			camera->fw_build_major,
			camera->fw_build_minor);

	status = Seekware_GetSettingEx(camera, SETTING_THERMOGRAPHY_VERSION, &therm_ver, sizeof(therm_ver));
	if (status != SW_RETCODE_NONE) {
		fprintf(stderr, "Error: Seek GetSetting returned %i\n", status);
	}
	printf("Thermography Version: %i\n", therm_ver);

	sw_sdk_info sdk_info;
	Seekware_GetSdkInfo(NULL, &sdk_info);
	printf("Image Processing Version: %u.%u.%u.%u\n",
			sdk_info.lib_version_major,
			sdk_info.lib_version_minor,
			sdk_info.lib_build_major,
			sdk_info.lib_build_minor);

	printf("\n");
	fflush(stdout);
}
static void signal_callback(int signum) {
	printf("\nExit requested!\n");
	exit_requested  = true;
}

static void help(const char * name) {
	printf(
			"Usage: %s [option]...\n"
			"Valid options are:\n"
			"   -h | --help                             Prints help.\n"
			"   -lut | --lut <l>                        Sets the given LUT for drawing a color image. The following values can be set (default is \"black\"):\n"
			"   -display-thermal | --display-thermal    Demonstrates how to draw a grayscale image using fixed point U16 thermography data from the camera\n"
			"   -display-thermal | --display-thermal    Demonstrates how to draw a grayscale image using filtered, pre-AGC, image data from the camera:\n"
			"                         ", name
		  );

	for (int j = 0; lut_names[j].name; ++j) {
		if (j) {
			printf(", ");
		}
		printf("%s", lut_names[j].name);
	}
	printf("\n");
}

static int parse_cmdline(int argc, char **argv) {
	for (int i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			help(argv[0]);
			return 0;
		}
		else if (!strcmp(argv[i], "-lut") || !strcmp(argv[i], "--lut")) {
			char* lut_name = argv[++i];
			bool found = false;
			for (int j = 0; lut_names[j].name; ++j) {
				if (!strcasecmp(lut_name, lut_names[j].name)) {
					current_lut = lut_names[j].value;
					found = true;
					break;
				}
			}
			if (!found) {
				fprintf(stderr, "ERROR: Unknown parameter \"%s\" for \"-lut\"\n", lut_name);
				help(argv[0]);
				return 1;
			}
		}
		else if (!strcmp(argv[i], "-display-thermal") || !strcmp(argv[i], "---display-thermal")) {
			display_mode = DISPLAY_THERMAL;
		}
		else if (!strcmp(argv[i], "-display-filtered") || !strcmp(argv[i], "---display-filtered")) {
			display_mode = DISPLAY_THERMAL;
		}
		else {
			fprintf(stderr, "Unknown parameter: \"%s\"\n", argv[i]);
			help(argv[0]);
			return 1;
		}
	}
	return 1;
}

uint32_t FIRE_THRES = 60;
uint32_t PERSON_MIN = 25;
uint32_t PERSON_MAX = 40;

void person_fire_agc(char* mac_addr, uint16_t* u16_input, size_t elements_in, size_t cols, size_t rows, uint32_t* argb_output, size_t elements_out) {

	size_t fire_check = 0;

	fire_rect fire_xy = {cols+1, rows+1, 0, 0};

	if(u16_input == NULL || argb_output == NULL){
		return;
	}

	size_t i = 0;
	uint16_t max = 0;
	uint16_t min = 1e4;
	uint16_t pixel_in = 0;

	for(i = 0; i < elements_in; i++){
		pixel_in = u16_input[i]/64-41;

		if(pixel_in > max && pixel_in < FIRE_THRES){
			max = pixel_in;
		}

		if(pixel_in < min) {
			min = pixel_in;
		}
	} 
	printf("max: %d, min: %d\n", max, min);
	/*	
		if(max < PERSON_MAX){
		PERSON_MAX = max;
		}
	 */
	uint32_t luminance = 0;
	float relative_intensity = 0.0f;

	for (i = 0; i < elements_out; ++i){
		pixel_in = u16_input[i]/64-40;

		if(pixel_in > FIRE_THRES){
			argb_output[i] = 0xFFFFFFFF-1;   
			printf("fire tem: %d", pixel_in);

			if(fire_xy.min_x > (size_t)i/rows) fire_xy.min_x = (size_t)i/rows; 
			if(fire_xy.min_y > (size_t)i%cols) fire_xy.min_y = (size_t)i%cols; 
			if(fire_xy.max_x < (size_t)i/rows) fire_xy.max_x = (size_t)i/rows; 
			if(fire_xy.max_y < (size_t)i%cols) fire_xy.max_y = (size_t)i%cols; 
			fire_check = 1;
		}
#if 1
		else { 
			if(pixel_in < PERSON_MIN){
				relative_intensity = (float)(pixel_in-min)/(PERSON_MIN-min);
				luminance = (uint32_t)(relative_intensity * 255.0f) /8;
			}
			else if(pixel_in > PERSON_MAX){
				relative_intensity = (float)(pixel_in-PERSON_MAX)/(max-PERSON_MAX);
				luminance = (uint32_t)(relative_intensity * 255.0f) /8;
			}
			else if(pixel_in >= PERSON_MIN && pixel_in <= PERSON_MAX){
				relative_intensity = (float)(pixel_in-PERSON_MIN)/(PERSON_MAX-PERSON_MIN);
				luminance = (uint32_t)(relative_intensity * 255.0f) /8 *7;
				luminance = luminance + 32.0f;    
			}
			argb_output[i] = 0xFD000000 | luminance << 16 | luminance << 8 | luminance;
		}
#endif
	}

	if(fire_xy.min_x <= rows && fire_xy.min_y <= cols){
		printf("fire min x: %d\nfire min y: %d\nfire max x: %d\nfire max y: %d\n", 
				fire_xy.min_x, fire_xy.min_y, fire_xy.max_x, fire_xy.max_y);
	}

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	char* post_data = malloc(sizeof(argb_output[0])*elements_in);

	memcpy(post_data, argb_output, sizeof(argb_output[0])*elements_in);

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://www.fireban.kr/api/detect/find/");
		sprintf(post_data, "mac=%s&image=%u&width=%u&height=%u&min_x=%u&min_y=%u&max_x=%u&max_y=%u&type=%u", mac_addr, argb_output, rows, cols, fire_xy.min_x, fire_xy.min_y, fire_xy.max_x, fire_xy.max_y, fire_check);
		printf("post_data : %s\n", post_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

//Perform a simple min/max linear stretch to transform U16 grayscale image data to ARGB8888
//For advanced AGC options that are highly customizable, please see the AGC settings listed in the Seekware User Guide.
void simple_agc(char* mac_addr, float* flt_input, uint32_t* u32_input, size_t elements_in, size_t cols, size_t rows, uint32_t* argb_output, size_t elements_out) {
	if(u32_input == NULL || argb_output == NULL || flt_input == NULL){
		return;
	}

	size_t i = 0;
	size_t fire_check=0;
	fire_rect fire_xy = {cols+1, rows+1, 0, 0};
	float min = 0;
	float max = 0;
	float pixel_in = 0;


	//Find min and max of the input
	for (i = 0; i < elements_in; ++i){
		pixel_in = flt_input[i];
		if(pixel_in > max){
			max = pixel_in;
		}
		if(pixel_in < min) {
			min = pixel_in;
		}
		if(pixel_in > FIRE_THRES) { 
			if(fire_xy.min_x > (size_t)i/rows) fire_xy.min_x = (size_t)i/rows; 
			if(fire_xy.min_y > (size_t)i%cols) fire_xy.min_y = (size_t)i%cols; 
			if(fire_xy.max_x < (size_t)i/rows) fire_xy.max_x = (size_t)i/rows; 
			if(fire_xy.max_y < (size_t)i%cols) fire_xy.max_y = (size_t)i%cols; 
			fire_check = 1;
			//printf("%d %d %d %d\n", fire_xy.min_x, fire_xy.min_y, fire_xy.max_x, fire_xy.max_y);
		}
	}
	//if(fire_check == 1) printf("MAX tem : %f\n", max);

	//Find relative intensity based on min/max. Assign output RGB channels to computed 8bit luminance.
	uint16_t delta = max-min;
	uint32_t luminance = 0;
	float relative_intensity = 0.0f;
	if(delta > 0){
		argb_output[i] = u32_input[i];
	}

	char* image_data = malloc(sizeof(argb_output[0])*elements_in+100);
	memcpy(image_data, argb_output, sizeof(argb_output[0])*elements_in);

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();

	char* post_data = malloc((sizeof(argb_output[0])*elements_in+200));

	if(fire_check==1) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://www.fireban.kr/api/detect/find/");
		sprintf(post_data, "mac=%s&image=%s&width=%u&height=%u&min_x=%u&min_y=%u&max_x=%u&max_y=%u&type=%u", mac_addr, image_data, rows, cols, fire_xy.min_x, fire_xy.min_y, fire_xy.max_x, fire_xy.max_y, fire_check);

//printf("post_data : %s\n", post_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	free(post_data);	
	free(image_data);
}

int main(int argc, char ** argv) {

#ifdef FEATURE_V4L2_STREAMING  // by sulac
	int fdwr = 0;
	int ret_code =0;
	struct v4l2_capability vid_caps;
	struct v4l2_format vid_format;
	size_t framesize = 0;
	size_t linewidth = 0;
#endif
	int x0 = 0;
	int y0 = 0;
	int offset = 2;
	int window_texture_pitch = 0;
	float spot = 0.0f;
	size_t frame_count = 0;
	size_t frame_pixels = 0;
	uint16_t* thermography_data = NULL;
	float* thermal_data = NULL;
	uint16_t* filtered_data = NULL;
	uint32_t* window_texture_data = NULL;

	bool camera_running = false;
	psw camera = NULL;
	psw camera_list[NUM_CAMS] = {0};
	sw_retcode status = SW_RETCODE_NONE;

	const char *ifname = "eth0";
	uint8_t mac_addr[MAC_LEN];
	char *mac_address = GetMacAddress(ifname, mac_addr);

	SDL_Event event;
	SDL_Window* window = NULL;
	SDL_Renderer* window_renderer = NULL;
	SDL_Texture* window_texture = NULL;

	signal(SIGINT, signal_callback);
	signal(SIGTERM, signal_callback);

	printf("seekware-sdl: Seek Thermal imaging tool for SDL2\n\n");

	sw_sdk_info sdk_info;
	Seekware_GetSdkInfo(NULL, &sdk_info);
	printf("SDK Version: %u.%u\n\n", sdk_info.sdk_version_major, sdk_info.sdk_version_minor);


	system("killall ffmpeg");
	system("killall autorun");


	/* * * * * * * * * * * * * Find Seek Cameras * * * * * * * * * * * * * * */

	int num_cameras_found = 0;
	Seekware_Find(camera_list, NUM_CAMS, &num_cameras_found);
	if (num_cameras_found == 0) {
		printf("Cannot find any cameras.\n");
		goto cleanup;
	}

	/* * * * * * * * * * * * * Initialize Seek SDK * * * * * * * * * * * * * * */

	//Open the first Seek Camera found by Seekware_Find
	for(int i = 0; i < num_cameras_found; ++i){
		camera = camera_list[i];
		status = Seekware_Open(camera);
		if (status == SW_RETCODE_OPENEX) {
			continue;   
		}
		if(status != SW_RETCODE_NONE){
			fprintf(stderr, "Could not open camera (%d)\n", status);
		}
		camera_running = true;
		break;
	}

	if(status != SW_RETCODE_NONE){
		fprintf(stderr, "Could not open camera (%d)\n", status);
		goto cleanup;
	}

	frame_pixels = (size_t)camera->frame_cols * (size_t)camera->frame_rows;


	printf("::Seek Camera Info::\n");
	print_fw_info(camera);

	// Set the default display lut value
	current_lut = SW_LUT_WHITE;		// SW_LUT_TYRIAN_NEW;

	// Parse the command line to additional settings
	if (parse_cmdline(argc, argv) == 0) {
		goto cleanup;
	}

	if(Seekware_SetSettingEx(camera, SETTING_ACTIVE_LUT, &current_lut, sizeof(current_lut)) != SW_RETCODE_NONE) {
		fprintf(stderr, "Invalid LUT index\n");
		goto cleanup;
	}

	// Allocate memory to store thermography data returned from the Seek camera
	if(display_mode == DISPLAY_THERMAL){
		thermography_data = (uint16_t*) malloc(frame_pixels * sizeof(uint16_t));
	} else if(display_mode == DISPLAY_FILTERED){
		filtered_data = (uint16_t*) malloc(frame_pixels * sizeof(uint16_t));
		thermal_data = (float*) malloc(frame_pixels * sizeof(float));
	} else if(display_mode == DISPLAY_PERSON_FILTERED){
		thermography_data = (uint16_t*) malloc(frame_pixels * sizeof(uint16_t));
	}

	/* * * * * * * * * * * * * Initialize SDL * * * * * * * * * * * * * * */

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		printf("Display driver: %s\n", SDL_GetVideoDriver(0));
	} else {
		perror("Error: Cannot initialize SDL2");
		goto cleanup;
	}

	// Initialize an SDL window:
	window = SDL_CreateWindow(
			"seekware-sdl",                         // window title
			100,                                    // initial x position
			100,                                    // initial y position
			camera->frame_cols * SCALE_FACTOR,		// width, in pixels
			camera->frame_rows * SCALE_FACTOR,      // height, in pixels
			SDL_WINDOW_SHOWN                        // present window on creation
			);
	if (window == NULL) {
		fprintf(stdout, "Could not create SDL window: %s\n", SDL_GetError());
		goto cleanup;
	}
#if SDL_VERSION_ATLEAST(2,0,5)
	SDL_SetWindowResizable(window, SDL_TRUE);
#endif

	//Initialize an SDL Renderer
	//If you would like to use software rendering, use SDL_RENDERER_SOFTWARE for the flags parameter of SDL_CreateRenderer
#ifdef __linux__
	struct utsname host_info;
	memset(&host_info, 0, sizeof(host_info));
	uname(&host_info);
	if(strstr(host_info.nodename, "raspberrypi") != NULL) {
		window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	}else {
		window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	}
#else
	window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#endif
	if(window_renderer == NULL){
		fprintf(stdout, "Could not create SDL window renderer: %s\n", SDL_GetError());
		goto cleanup;
	}

	if(SDL_RenderSetLogicalSize(window_renderer, camera->frame_cols, camera->frame_rows) < 0){
		fprintf(stdout, "Could not set logical size of the SDL window renderer: %s\n", SDL_GetError());
		goto cleanup;
	}

	//Create a backing texture for the SDL window
	window_texture = SDL_CreateTexture(window_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, camera->frame_cols, camera->frame_rows);
	if(window_texture == NULL){
		fprintf(stdout, "Could not create SDL window texture: %s\n", SDL_GetError());
		goto cleanup;
	}
#ifdef FEATURE_V4L2_STREAMING  // by sulac
	fdwr = open(VIDEO_DEVICE, O_RDWR);
	assert(fdwr >= 0);

	ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
	assert(ret_code != -1);

	memset(&vid_format, 0, sizeof(vid_format));

	ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
	vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	vid_format.fmt.pix.width = camera->frame_cols; //dev->frame_cols;
	vid_format.fmt.pix.height = camera->frame_rows; //dev->frame_rows;
	vid_format.fmt.pix.pixelformat = FRAME_FORMAT;
	vid_format.fmt.pix.sizeimage = framesize;
	vid_format.fmt.pix.field = V4L2_FIELD_NONE;
	vid_format.fmt.pix.bytesperline = linewidth;
	vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

	ret_code = ioctl(fdwr, VIDIOC_S_FMT, &vid_format);
	assert(ret_code != -1); 


#endif
	/* * * * * * * * * * * * * Imaging Loop * * * * * * * * * * * * * * */

	print_time(); printf(" Start! \n");
	do{
		//Lock the backing texture and get a pointer for accessing the texture memory directly
		if(SDL_LockTexture(window_texture, NULL, (void**)(&window_texture_data), &window_texture_pitch) != 0){
			fprintf(stdout, "Could not lock SDL window texture: %s\n", SDL_GetError());
			exit_requested = true;
			continue;
		}

		//Get data from the camera
		if(display_mode == DISPLAY_ARGB){
			// Pass a pointer to the texture directly into Seekware_GetImage for maximum performance
			status =  Seekware_GetDisplayImage(camera, window_texture_data, (uint32_t)frame_pixels);

#ifdef FEATURE_V4L2_STREAMING  // by sulac
			write(fdwr, window_texture_data, frame_pixels*4 );
#endif 
		}
		if(display_mode == DISPLAY_THERMAL){
			status =  Seekware_GetThermographyImage(camera, thermography_data, (uint32_t)frame_pixels);
		}
		if(display_mode == DISPLAY_FILTERED) {
			status =  Seekware_GetImage(camera, filtered_data, thermal_data, window_texture_data);
#ifdef FEATURE_V4L2_STREAMING
			write(fdwr, window_texture_data, frame_pixels*4);
#endif
		}
		if(display_mode == DISPLAY_PERSON_FILTERED) {
			status =  Seekware_GetThermographyImage(camera, thermography_data, (uint32_t)frame_pixels);
		}

		//Check for errors
		if(camera_running){
			if(status == SW_RETCODE_NOFRAME){
				print_time(); printf(" Seek Camera Timeout ...\n");
			}
			if(status == SW_RETCODE_DISCONNECTED){
				print_time(); printf(" Seek Camera Disconnected ...\n");
			}
			if(status != SW_RETCODE_NONE){
				print_time(); printf(" Seek Camera Error : %u ...\n", status);
				break;
			}
		}

		//Do AGC
		if (display_mode == DISPLAY_THERMAL) {
			//simple_agc(thermography_data, frame_pixels, (size_t)camera->frame_cols, (size_t)camera->frame_rows, window_texture_data, frame_pixels);
		}
		if (display_mode == DISPLAY_FILTERED) {
			simple_agc(mac_address, thermal_data, window_texture_data, frame_pixels, (size_t)camera->frame_cols, (size_t)camera->frame_rows, window_texture_data, frame_pixels);
		}
		if (display_mode == DISPLAY_PERSON_FILTERED) {
			person_fire_agc(mac_address, thermography_data, frame_pixels,(size_t)camera->frame_cols, (size_t)camera->frame_rows, window_texture_data, frame_pixels);
		}

		//Unlock texture
		SDL_UnlockTexture(window_texture);

		++frame_count;

		//Load Texture
		if (SDL_RenderCopyEx(window_renderer, window_texture, NULL, NULL, 0, NULL, SDL_FLIP_NONE) < 0) {
			fprintf(stdout, "\n Could not copy window texture to window renderer: %s\n", SDL_GetError());
			break;
		}

		//Draw Reticle
#if DRAW_RETICLE
		x0 = camera->frame_cols / 2;
		y0 = camera->frame_rows / 2;
		SDL_SetRenderDrawColor(window_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderDrawLine(window_renderer, x0, y0 - offset, x0, y0 + offset);
		SDL_RenderDrawLine(window_renderer, x0 - offset, y0, x0 + offset, y0);
#endif

		//Blit
		SDL_RenderPresent(window_renderer);

		// Request additional temperature information from the Seek camera (optional)
		if (Seekware_GetSpot(camera, &spot, NULL, NULL) != SW_RETCODE_NONE) {
			fprintf(stderr, "\n Get Spot error!\n");
			break;
		}

		//Update temperatures to stdout
		//On select cameras that do not support thermography, nan is returned for spot, min, and max
		if(!exit_requested){
			if (frame_count > 1) {
				static const int num_lines = 4;
				for (int i = 0; i < num_lines; i++); //{
				//printf("\033[A");
				//}
			}
			//printf("\r\33[2K--------------------------\n");
			//printf("\33[2K\x1B[42m\x1B[37m spot:%*.1fC \x1B[0m\n", 3, spot);
			//printf("\33[2K--------------------------\n\n");
			//for(size_t i = 0; i < frame_pixels; i++) {
			//	printf("[%u]%u\n", i, window_texture_data[i]);
			//}
			//printf("\n");
			fflush(stdout);
		}

		//Check for SDL window events
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE){
				exit_requested = true;
			}
			if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE){
				if(camera_running){
					Seekware_Stop(camera);
					camera_running = false;
				}else {
					Seekware_Start(camera);
					camera_running = true;
				}
			}
		}
	} while (!exit_requested);

	/* * * * * * * * * * * * * Cleanup * * * * * * * * * * * * * * */
cleanup:

	printf("Exiting...\n");

	if (camera != NULL) {
		Seekware_Close(camera);
	}
	if(thermography_data != NULL){
		free(thermography_data);
	}
	if(filtered_data != NULL){
		free(filtered_data);
	}
	if(window_texture != NULL) {
		SDL_DestroyTexture(window_texture);
	}
	if(window_renderer != NULL) {
		SDL_DestroyRenderer(window_renderer);
	}
	if(window != NULL) {
		SDL_DestroyWindow(window);
	}


#ifdef FEATURE_V4L2_STREAMING  // by sulac
	close(fdwr);
#endif
	//free(mac_address);
	SDL_Quit();
	return 0;
}

/* * * * * * * * * * * * * End - of - File * * * * * * * * * * * * * * */

