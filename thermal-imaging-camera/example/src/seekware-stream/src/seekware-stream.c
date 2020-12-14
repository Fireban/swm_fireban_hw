/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:     Seek Thermal SDK demo
 * Purpose:     This source demonstrates how to use the Seek Thermal SDK for PIR cameras.
 * Author:      Seek Thermal, Inc.
 * License:     Proprietary
 * Comments:    If you are interested in the SDK handling only, see the
 *              section "SeekThermal SDK Handling" below.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <seekware/seekware.h>

//v4l2
#include <linux/videodev2.h>
#include <assert.h>


#include "colour.h"

#define NUM_CAMS    4
#define VIDEO_DEVICE "/dev/video4"
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define FRAME_FORMAT V4L2_PIX_FMT_ARGB32 

typedef enum DISPLAY_TYPE {
    DISP_TYPE_NONE = 0,
    DISP_TYPE_SDL,
    DISP_TYPE_FB
} DISPLAY_TYPE;

typedef enum DISPLAY_MODE {
    DISP_RGB  = 0,
    DISP_THERMAL = 1,
    DISP_BINARY = 2
} DISPLAY_MODE;


static struct {
    const char * name;
    int value;
} lut_names[] = {
    { "white",  SW_LUT_WHITE    },
    { "black",  SW_LUT_BLACK    },
    { "iron",   SW_LUT_IRON     },
    { "cool",   SW_LUT_COOL     },
    { "amber",  SW_LUT_AMBER    },
    { "indigo", SW_LUT_INDIGO   },
    { "tyrian", SW_LUT_TYRIAN   },
    { "glory",  SW_LUT_GLORY    },
    { "envy",   SW_LUT_ENVY     },
    {  NULL,    0               }
};


pthread_mutex_t exitmutex = PTHREAD_MUTEX_INITIALIZER;
int exit_requested = false;
int dbl = 1;
int disp_type = DISP_TYPE_NONE;
int disp_mode = DISP_RGB;
int current_lut = SW_LUT_SPECTRA;
char *lut_name = NULL;
const char * fbdev_name = "/dev/fb0";
int screensize;
int fd_frame;
int bpp;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
uint8_t * pixels = NULL;
psw pl[NUM_CAMS];
pthread_t tids[NUM_CAMS];
SDL_Surface *screen;

//v4l2
int fdwr = 0;

static void printFWInfo(psw dev) {
    sw_retcode status; 
    sw_sdk_info sdk_info;
    int therm_ver;

    if((status = Seekware_GetSdkInfo(dev,&sdk_info)) != SW_RETCODE_NONE)
        fprintf(stderr,"Error: Seek GetSdkInfo returned %i\n", status);

    if((status = Seekware_GetSettingEx(dev,SETTING_THERMOGRAPHY_VERSION, &therm_ver, sizeof(therm_ver))) != SW_RETCODE_NONE)
        fprintf(stderr,"Error: Seek GetSetting returned %i\n", status);

    printf("Model: %u\n",dev->model);
    printf("ModelNumber: %s\n", dev->modelNumber);
    printf("Manufacture Date: %s\n", dev->manufactureDate);
    printf("SerialNumber: %s\n", dev->serialNumber);
    printf("Width: %u\n",dev->frame_cols);
    printf("Height: %u\n",dev->frame_rows);

    printf("FirmwareVersion: %u.%u.%u.%u\n",
            dev->fw_version_major,dev->fw_version_minor,
            dev->fw_build_major,dev->fw_build_minor);
    printf("SDK version: %u.%u.%u.%u\n",
            sdk_info.sdk_version_major,sdk_info.sdk_version_minor,
            sdk_info.sdk_build_major,sdk_info.sdk_build_minor);
    printf("LIB version: %u.%u.%u.%u\n",
            sdk_info.lib_version_major,sdk_info.lib_version_minor,
            sdk_info.lib_build_major,sdk_info.lib_build_minor);
    printf("THERM version: %i\n",therm_ver);
}


// Utility function to determine whether a file actually exists.
bool file_exists(const char * filename)
{
    if (FILE *file = fopen(filename, "r")) {
        fclose(file);
        return true;
    }
    return false;
}


// Utility function to determine whether a string starts with a number or not
int is_number(char *str) {
    int value;
    return (sscanf(str, "%d", &value) == 1);
}


// Utility function to count the number of substrings in a buffer
int num_substrings(char *str, const char *tokens) {
    int count = 0;
    char buffer[80];
    strcpy(buffer, str);
    char *pch = strtok(buffer, tokens);
    while (pch != NULL) {
        pch = strtok(NULL, tokens);
        count++;
    }
    return count;
}


// Loads a LUT from a file and stores it into a ARGB32 array
// Supports all known variants of imagej LUTs
// Supports Seek *.txt LUT files.
int load_user_lut(const char *filename, uint32_t *lut_data) {
    // Read up ImageJ LUTs and store the RGB data into lut_data.
    int result, colors = 0;
    uint8_t header[32], red[NR_LUTCOLORS], green[NR_LUTCOLORS], blue[NR_LUTCOLORS];
    char *ptr, buffer[81];

    //printf("Loading LUT %s\n", lut_name);

    // Open the LUT file
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        fprintf(stderr, "Cannot open LUT file: %s - %s\n", lut_name, strerror(errno));
        return false;
    }

    // Measure the length of the file
    int file_start, file_end, file_length;
    fseek(infile, 0, SEEK_SET);
    file_start = ftell(infile);
    fseek(infile, 0, SEEK_END);
    file_end = ftell(infile);
    fseek(infile, 0, SEEK_SET);
    file_length = file_end - file_start;
    //printf("LUT file is %d bytes long.\n", file_length);

    switch (file_length) {
    case 800:
        // Parse an 800 byte long binary RGB LUT with header
        // Reads, but ignores the header
        result = fread(header, sizeof(uint8_t), 32, infile);
        if (result != 32) {
            printf("Cannot read file header\n");
            fclose(infile);
            return false;
        }
        colors = 256;
    case 768:
        // Parse a 768 byte long binary RGB LUT
        result = fread(red, sizeof(uint8_t), NR_LUTCOLORS, infile);
        if (result != NR_LUTCOLORS) {
            printf("Cannot read red data.  Got %d items.\n", result);
            fclose(infile);
            return false;
        }
        result = fread(green, sizeof(uint8_t), NR_LUTCOLORS, infile);
        if (result != NR_LUTCOLORS) {
            printf("Cannot read green data.  Got %d items.\n", result);
            fclose(infile);
            return false;
        }
        result = fread(blue, sizeof(uint8_t), NR_LUTCOLORS, infile);
        if (result != NR_LUTCOLORS) {
            printf("Cannot read blue data.  Got %d items.\n", result);
            fclose(infile);
            return false;
        }
        colors = 256;
        break;
    default:
        // Read ASCII LUTs.  There are several variants
        //printf("Reading ASCII LUT\n");
        int i, r, g, b, cols;

        // Process lines one at a time
        ptr = fgets(buffer, 80, infile);
        if (ptr == NULL) {
            return false;
        }

        colors = 0;
        fseek(infile, 0, SEEK_SET);  // Start over

        // Parse the line
        switch (cols = num_substrings(buffer, " \t")) {
        case 2:
            //printf("Reading Seek ASCII LUT\n");
            do {
                // Read another line
                ptr = fgets(buffer, 80, infile);

                //printf("scanning %s for color %d\n", buffer, colors);
                result = sscanf(buffer, " ARGB(%d,%d,%d)", &r, &g, &b);
                if (result == 3) {
                    red[colors] = r;
                    green[colors] = g;
                    blue[colors] = b;
                    colors++;
                }
            } while (!feof(infile) && (colors < NR_LUTCOLORS));
            break;
        case 3:
            //printf("Reading 3 column ASCII LUT\n");
            do {
                // Read another line
                ptr = fgets(buffer, 80, infile);

                // Parse the line
                if (is_number(buffer)) {
                    //printf("scanning %s for color %d\n", buffer, colors);
                    result = sscanf(buffer, "%d %d %d", &r, &g, &b);
                    if (result == 3) {
                        red[colors] = r;
                        green[colors] = g;
                        blue[colors] = b;
                        colors++;
                    }
                    else {
                        printf("sscanf error: %s scanned %d elements\n", buffer, result);
                    }
                }
            } while (!feof(infile) && (colors < NR_LUTCOLORS));
            break;
        case 4:
            //printf("Reading 4 column ASCII LUT\n");
            do {
                // Read another line
                ptr = fgets(buffer, 80, infile);

                // Parse the line
                if (is_number(buffer)) {
                    //printf("scanning %s for color %d\n", buffer, colors);
                    result = sscanf(buffer, "%d %d %d %d", &i, &r, &g, &b);
                    if (result == 4) {
                        red[colors] = r;
                        green[colors] = g;
                        blue[colors] = b;
                        colors++;
                    }
                    else {
                        printf("sscanf error: %s scanned %d elements\n", buffer, result);
                    }
                }
            } while (!feof(infile) && (colors < NR_LUTCOLORS));
            break;
        default:
            printf("Unsupported LUT format: %d columns\n", cols);
            fclose(infile);
            return false;
            break;
        }
        break;
    }

    // Copy RGB data into lut_data.  Assume alpha is 0xFF (opaque)
    //printf("Read %d colors\n", colors);
    if (colors == 256) {
        for (uint32_t i=0; i<NR_LUTCOLORS; i++) {
            //printf("LUT %d = %d, %d, %d\n", i, red[i], green[i], blue[i]);
            lut_data[i] = (0xFF << 24) | (red[i] << 16) | (green[i] << 8) | blue[i];
        }
    }

    fclose(infile);

    return true;
}


// Imaging thread
void *work_thread(void *arg) {
    char title[40];
    long index = (long)arg;
    psw dev = pl[index];
    int result, shouldexit = false;
    float temp, min, max;
    SDL_Window *window = NULL;
    SDL_Surface *window_surface = NULL;
    SDL_Surface *image = NULL;
    SDL_Event event;
    
    uint16_t *image_bin;
    union imgdata *image_rgb;
    float *image_f;

    //sulac v4l2 init value. 
    int ret_code = 0;
    struct v4l2_capability vid_caps;
    struct v4l2_format vid_format;
    size_t framesize = 0;
    size_t linewidth = 0;

    sw_retcode status = Seekware_Open(dev);
    if (SW_RETCODE_NONE != status) {
        fprintf(stderr, "Could not open PIR Device (%d)\n", status);
        return NULL;
    }

    printf("::Camera %ld Firmware Info::\n", index);
    printFWInfo(dev);

    // If the current_lut is a user lut, load the file and write the LUT data
    if (current_lut >= SW_LUT_USER0) {
        uint32_t lut_data[NR_LUTCOLORS];
        result = load_user_lut(lut_name, lut_data);
        if (result == false) {
            Seekware_Close(dev);
            return NULL;
        }
        result = Seekware_SetUserLUT(dev, current_lut, lut_data, NR_LUTCOLORS);
        if (result != SW_RETCODE_NONE) {
            fprintf(stderr, "Cannot set user LUT.  Got %d\n", result);
            Seekware_Close(dev);
            return NULL;
        }
    }

    // Set the current lut value
    if (Seekware_SetSettingEx(dev, SETTING_ACTIVE_LUT, &current_lut, sizeof(current_lut)) != SW_RETCODE_NONE) {
        fprintf(stderr, "Invalid LUT index\n");
        Seekware_Close(dev);
        return NULL;
    }

    // If we're using SDL, open a window
    if (disp_type == DISP_TYPE_SDL) {
        // Create an application window with the following settings:
        sprintf(title, "seekware-test: cam%ld", index);
        window = SDL_CreateWindow(
            title,                              // window title
            SDL_WINDOWPOS_UNDEFINED,           // initial x position
            SDL_WINDOWPOS_UNDEFINED,           // initial y position
            dev->frame_cols,                   // width, in pixels
            dev->frame_rows,                   // height, in pixels
            SDL_WINDOW_SHOWN                   // flags - see below
        );
        if (window == NULL) {
            // In the case that the window could not be made...
            fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
            return NULL;
        }
        window_surface = SDL_GetWindowSurface(window);
        SDL_FillRect(window_surface, NULL, SDL_MapRGB(window_surface->format, 0, 0, 0));
        SDL_UpdateWindowSurface(window);
        SDL_SetWindowResizable(window,SDL_FALSE);

        //v4l2 init by sulac
        {
            fdwr = open(VIDEO_DEVICE, O_RDWR);
            assert(fdwr >= 0);
            
            ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
            assert(ret_code != -1); 

            memset(&vid_format, 0, sizeof(vid_format));
            
            ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
            vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
            vid_format.fmt.pix.width = dev->frame_cols; //FRAME_WIDTH; 
            vid_format.fmt.pix.height = dev->frame_rows; //FRAME_HEIGHT; 
            vid_format.fmt.pix.pixelformat = FRAME_FORMAT; 
            vid_format.fmt.pix.sizeimage = framesize; 
            vid_format.fmt.pix.field = V4L2_FIELD_NONE;
            vid_format.fmt.pix.bytesperline = linewidth;
            vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

            ret_code = ioctl(fdwr, VIDIOC_S_FMT, &vid_format);
            assert(ret_code != -1);
#if 0
            if(!format_properties(vid_format.fmt.pix.pixelformat,
                        vid_format.fmt.pix.width, vid_format.fmt.pix.height, 
                         &linewidth, 
                         &framesize)) {  
                 printf("unable to guess correct settings for format '%d'\n", FRAME_FORMAT);   
                }
#endif                             
        }
    }

    // Allocate image data
    image_bin = (uint16_t*) calloc(sizeof(uint16_t), dev->frame_cols*dev->frame_rows);
    image_rgb = (union imgdata*) calloc(sizeof(imgdata), dev->frame_cols*dev->frame_rows);
    image_f = (float*) calloc(sizeof(float), dev->frame_cols*dev->frame_rows);
  
    // Main image acquisition loop
    // Acquisition is NOT pipeline threaded here; it's done in the SDK.
    while (!shouldexit) {
        // Grab an image from the camera
        sw_retcode status;
        status = Seekware_GetImage(dev, NULL, (float_t*) image_f, (uint32_t *) image_rgb);
        
        if(status == SW_RETCODE_NOFRAME){
            continue;
        }
        
        if (status != SW_RETCODE_NONE) {
            fprintf(stderr, "Get Image error!\n");
            return NULL;
        }

        if (Seekware_GetSpot(dev, &temp, &min, &max) != SW_RETCODE_NONE) {
            fprintf(stderr, "Get Spot error!\n");
            shouldexit = true;
            break;
        }

        // If we're on X, use SDL to image in a window
        if (disp_type == DISP_TYPE_SDL) {
            // Create a surface from the display image
            image = SDL_CreateRGBSurfaceFrom((void*)image_rgb, dev->frame_cols, dev->frame_rows, 32, 4*dev->frame_cols, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
            if (image == NULL) {
                fprintf(stderr, "SDL_CreateRGBSurfaceFrom cannot create surface: %s\n", SDL_GetError());
                shouldexit = true;
                continue;
            }
            //sulac v4l2 
            write(fdwr, image_rgb, dev->frame_cols*dev->frame_rows*4 ); 

            // BLIT the surface to the window
            result = SDL_BlitScaled(image, NULL, window_surface, NULL);
            if (result != 0) {
                fprintf(stderr, "SDL_BlitSurface error: %d\n", result);
                shouldexit = true;
                continue;
            }

            // Update the window and free the image
            SDL_UpdateWindowSurface(window);
            SDL_FreeSurface(image);
        }
        else {
            // We're not under X, use the linux framebuffer
            int x, y;
            int rows = dev->frame_rows;
            int cols = dev->frame_cols;

            const union imgdata *pixel = image_rgb;
            for (y=0; y<rows; ++y) {
                int dy = y;
                for (x=0; x<cols; ++x, ++pixel) {
                    int dx = x;
                    if (dbl) {
                        if (putPixelDirect(2*dx, 2*dy, pixel)) {
                            return NULL;
                        }
                        if (putPixelDirect(2*dx+1, 2*dy, pixel)) {
                            return NULL;
                        }
                        if (putPixelDirect(2*dx+1, 2*dy+1, pixel)) {
                            return NULL;
                        }
                        if (putPixelDirect(2*dx, 2*dy+1, pixel)) {
                            return NULL;
                        }
                    }
                    else {
                        if (putPixelDirect(dx, dy, pixel)) {
                            return NULL;
                        }
                    }
                }
            }
        }

        printf("camera %ld spot:%2.0f min: %2.0f max: %2.0f\n", index, temp, min, max);
 
        // Poll for exit events
        // NOTE: SDL_PollEvent somehow blocks BlitScaled from scaling.
        SDL_PollEvent(&event);
        // Check for exit_requested
        pthread_mutex_lock(&exitmutex);
        shouldexit = exit_requested | (event.type == SDL_KEYDOWN) | (event.type == SDL_QUIT);
        pthread_mutex_unlock(&exitmutex);
    }

    // close v4l2 
    close(fdwr);
    // Free allocate image storage
    free(image_bin);
    free(image_rgb);
    free(image_f);

    // Close the SDK
    Seekware_Close(dev);

    // Free the window surface
    SDL_FreeSurface(window_surface);
    SDL_DestroyWindow(window);

    return NULL;
}


// Handle Ctrl-C interrupt
static void signalCallback(int signum) {
    pthread_mutex_lock(&exitmutex);
    fprintf(stderr, "\nExit requested.\n");
    exit_requested = true;
    pthread_mutex_unlock(&exitmutex);
}


// Filter SDL events to keep the exit latency down
int FilterEvents(void *userdata, SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        return(1);
    }
    if (event->type == SDL_KEYDOWN) {
        return(1);
    }
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        return(1);
    }
    return(0);
}


// Display the help text on the console
static void help(const char * name) {
    printf(
        "Usage: %s [option]...\n"
        "Valid options are:\n"
        "   -h | --help         Print this useful text and exit.\n"
        "   --device <dev>      The name of the framebuffer device. Its default is \"/dev/fb0\".\n"
        "   -d | --double       Doubles the size of the displayed rectangle(s) both in horizontal and vertical directions.\n"
        "   -min <t>            Lowest temperature (black) for thermal image (floating point value in Celsius).\n"
        "   -range <t>          Range between highest and lowest temperatures for thermal image (floating point value in Celsius).\n"
        "   -t | --filter <t>   First order low pass filter time constant for thermal image (floating point value, in seconds).\n"
        "   -lut | --lut <l>    Sets the given LUT for RGB image. The following values can be set (default is \"black\"):\n"
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


// Parse the command line and set variables accordingly
int parse_cmdline(int argc, char **argv) {\
    for (int i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--double")) {
            dbl = 1;
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            help(argv[0]);
            return 0;
        }
        else if (!strcmp(argv[i], "--device")) {
            fbdev_name = argv[++i];
        }
        else if (!strcmp(argv[i], "-lut") || !strcmp(argv[i], "--lut")) {
            lut_name = argv[++i];
            bool found = false;
            // Try to read the argument as a number.  If we parse a number, use it as a LUT index.
            int index = 0;
            if (sscanf(lut_name, "%d", &index) == 1) {
                current_lut = index;
                found = true;
            }
            // Look for a . which indicates a filename
            else if (strchr(lut_name, '.') != NULL) {
                current_lut = SW_LUT_USER0;
                found = true;
            }
            else {
                // Not a number, read the argument as a name
                for (int j = 0; lut_names[j].name; ++j) {
                    if (!strcasecmp(lut_name, lut_names[j].name)) {
                        current_lut = lut_names[j].value;
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                fprintf(stderr, "ERROR: Unknown parameter \"%s\" for \"-lut\"\n", lut_name);
                help(argv[0]);
                return 1;
            }
        }
        else {
            fprintf(stderr, "Unknown parameter: \"%s\"\n", argv[i]);
            help(argv[0]);
            return 1;
        }
    }
    return 1;
}


int main(int argc, char ** argv) {
    int num = 0;

    printf("seekware-test: Seek Thermal command-line Imaging tool\n");

    // Parse the command line
    if (parse_cmdline(argc, argv) == 0) {
        exit(1);
    }

    // Look for connected cameras
    Seekware_Find(pl, NUM_CAMS, &num);
    if (num == 0) {
        printf("Cannot find any cameras.\n");
        return 1;
    }
    printf("Got %d camera devices.\nPress Control-C to exit...\n", num);

    // Init the display environment
    // Try to init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0) {
        //printf("Display driver: %s\n", SDL_GetVideoDriver(0));
        if (strcmp(SDL_GetVideoDriver(0), "x11") == 0){
            /* Filter quit and mouse motion events */
            SDL_SetEventFilter(FilterEvents, NULL);
            disp_type = DISP_TYPE_SDL;
        }
    }
    else {
        // If SDL_Init failed, use the linux framebuffer device
        fd_frame = open(fbdev_name, O_RDWR);
        if (fd_frame == -1) {
            char msg[100];
            snprintf(msg, sizeof msg, "Error: Cannot open framebuffer device '%s'", fbdev_name);
            perror(msg);
            printf("This program must be run in a native console.\n");
            return 1;
        }
        
	if (ioctl(fd_frame, FBIOGET_FSCREENINFO, &finfo) == -1) {
            perror("Error reading fixed information");
            return 1;
        }

        if (ioctl(fd_frame, FBIOGET_VSCREENINFO, &vinfo) == -1) {
            perror("Error reading variable information");
            return 1;
        }
        bpp = vinfo.bits_per_pixel / 8;
        screensize = vinfo.xres * vinfo.yres * bpp;

        // Memory map the frame buffer into the process memory space
        pixels = (uint8_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_frame, 0);
        if ((void*)pixels == MAP_FAILED) {
            perror("Error: failed to map framebuffer device to memory");
            return 1;
        }

        disp_type = DISP_TYPE_FB;
    }

    // Init the exit mutex
    pthread_mutex_init(&exitmutex, NULL);

    // Hook signals for CTRL-C
    signal(15, signalCallback);
    signal(2, signalCallback);

    // Launch acquisition/display treads for each camera
    for (long i = 0; i < num; ++i) {
            pthread_create(tids+i, NULL, work_thread, (void*)i);
    }

    for (long i = 0; i < num; ++i) {
            pthread_join(tids[i], NULL);
    }
    
    // Destroy the mutex when you're done
    pthread_mutex_destroy(&exitmutex);

    // Deinit the display
    if (disp_type == DISP_TYPE_SDL) {
        SDL_Quit();
    }
    else {
        munmap(pixels, screensize);
        close(fd_frame);
    }

    return 0;
}

/* * * * * * * * * * * * * End - of - File * * * * * * * * * * * * * * */
