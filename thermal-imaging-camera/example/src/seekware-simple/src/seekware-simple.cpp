
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include <seekware/seekware.h>
#include <stdbool.h>

#define NUM_CAMS            4
#define WRITE_RAW_FILE      false
#define WRITE_OTHER_FILES   false
#define WRITE_THERMO_FILE   false

psw pl[NUM_CAMS];
int current_lut = SW_LUT_BLACK;
bool shouldexit = false;


static void printFWInfo(psw dev) {
    sw_retcode status; 
    sw_sdk_info sdk_info;
    int therm_ver;

    if((status = Seekware_GetSdkInfo(dev,&sdk_info)) != SW_RETCODE_NONE)
        fprintf(stderr,"Error: Seek GetSdkInfo returned %i\n", status);

    if((status = Seekware_GetSettingEx(dev,SETTING_THERMOGRAPHY_VERSION, &therm_ver, sizeof(therm_ver))) != SW_RETCODE_NONE)
        fprintf(stderr,"Error: Seek GetSetting returned %i\n", status);

    printf("Model Number: %s\n",dev->modelNumber);
    printf("SerialNumber: %s\n", dev->serialNumber);
    printf("Manufacture Date: %s\n", dev->manufactureDate);
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


void *thread_process(void *arg)
{
    long index = (long) arg;
    psw dev = pl[index];
    uint16_t *frame_raw        = NULL;
    float    *frame_temperature= NULL;
    uint32_t *frame_color      = NULL;
    int       emissivity = 0;
    int       background = 0;

    sw_retcode status = Seekware_Open(dev);
    if (SW_RETCODE_NONE != status) {
      fprintf(stderr, "Could not open PIR Device (%d) (first)\n", status);
      return NULL;
    }

    // Must read firmware info AFTER the device has been opened
    printf("::Camera %ld Firmware Info::\n", index);
    printFWInfo(dev);
    
    /* Allocate necessary frames, leave as NULL if don't care */   
    frame_raw = new uint16_t[dev->frame_cols*(dev->frame_rows+1)];
    if (!frame_raw){
      fprintf(stderr, "frame_raw memory allocation failed!\n");
      return NULL;      
    }
    
    frame_temperature = new float[dev->frame_cols*dev->frame_rows];
    if (!frame_temperature){
      fprintf(stderr, "frame_temperature memory allocation failed!\n");
      return NULL;      
    }
    
    frame_color = new uint32_t[dev->frame_cols*dev->frame_rows];
    if (!frame_color){
      fprintf(stderr, "frame_color memory allocation failed!\n");
      return NULL;      
    }
   
    if (Seekware_SetSettingEx(dev, SETTING_ACTIVE_LUT, &current_lut, sizeof(current_lut)) != SW_RETCODE_NONE) {
        fprintf(stderr, "Seekware_SetSettingEx() failed!\n");
        return NULL;
    } 
    
    if (Seekware_GetSettingEx(dev, SETTING_EMISSIVITY, &emissivity, sizeof(emissivity)) != SW_RETCODE_NONE) {
        fprintf(stderr, "Cannot get emissivity setting\n");
    }
    else {
        printf("Emissivity: %d %%\n", emissivity);
    }

    if (Seekware_GetSettingEx(dev, SETTING_BACKGROUND, &background, sizeof(background)) != SW_RETCODE_NONE) {
        fprintf(stderr, "Cannot get background setting\n");
    }
    else {
        printf("Background: %2.1f\n", (double) background);
    }

    static int frames=0;
    long ns_old = 0;
    long ns = 0;
    uint8_t rate = 0;
    uint8_t rate_old = 0;
    float temp, min, max;
    struct timespec spec;
    uint32_t fieldcount = 0;
    uint16_t tDiodeTemp = 0;
    uint32_t envTempBits = 0;
    float envTemp = 0.0f;

    clock_gettime(CLOCK_REALTIME, &spec);         
    ns = spec.tv_nsec;
    
    status = Seekware_GetImageEx(dev, NULL, NULL, NULL);
    if(status == SW_RETCODE_NONE || status == SW_RETCODE_NOFRAME){
        /*Add THERM adjustments here if needed*/
    } else{
        printf("Couldn't grab first frame\n");
        shouldexit = true;
    }

    while (!shouldexit) {
        status = Seekware_GetImageEx(dev, frame_raw, frame_temperature, NULL);
        
        if(status != SW_RETCODE_NONE) {
            break;
        }
        
        if (Seekware_GetSpot(dev, &temp, &min, &max) != SW_RETCODE_NONE) {
            break;
        }

        int row,col;

        if (WRITE_RAW_FILE){  
            FILE *fp=fopen("raw.txt","w");
            for(row=0;row<(dev->frame_rows+1);++row){
               for(col=0;col<dev->frame_cols;++col){
                  fprintf(fp,"%d ",frame_raw[(row*dev->frame_cols)+col]);         
               }
               fprintf(fp,"\n");
            }
            fclose(fp);
        }  
     
        if (WRITE_OTHER_FILES){  
            FILE *fp=fopen("thermography.txt","w");
            for(row=0;row<dev->frame_rows;++row){
               for(col=0;col<dev->frame_cols;++col){
                  fprintf(fp,"%f ",frame_temperature[(row*dev->frame_cols)+col]);         
               }
               fprintf(fp,"\n");
            }
            fclose(fp);
        }
         
        if (WRITE_OTHER_FILES){  
            FILE *fp=fopen("color.txt","w");
            for(row=0;row<dev->frame_rows;++row){
               for(col=0;col<dev->frame_cols;++col){
                  fprintf(fp,"%08x ",frame_color[(row*dev->frame_cols)+col]);         
               }
               fprintf(fp,"\n");
            }
            fclose(fp);
        }
        
        if (WRITE_THERMO_FILE){  
            FILE *fp=fopen("thermo.txt","w");
            uint16_t thermo_frame[dev->frame_cols*dev->frame_rows];
            Seekware_GetThermographyImage(dev,&thermo_frame[0],(uint32_t)(dev->frame_cols*dev->frame_rows));
            for(row=0;row<dev->frame_rows;++row){
               for(col=0;col<dev->frame_cols;++col){
                  fprintf(fp,"%04x ",thermo_frame[(row*dev->frame_cols)+col]);         
               }
               fprintf(fp,"\n");
            }
            fclose(fp);
        }

        // Extract telemetry
        fieldcount = (uint32_t) frame_raw[dev->frame_rows*dev->frame_cols] + ((uint32_t) frame_raw[dev->frame_rows*dev->frame_cols+1] << 16);
        tDiodeTemp = frame_raw[dev->frame_rows*dev->frame_cols+2];
        if (WRITE_OTHER_FILES){  
            FILE *fp=fopen("diode.txt","a");
            fprintf(fp,"%hu",tDiodeTemp);         
            fprintf(fp,"\n");
            fclose(fp);
        }
        envTempBits = (uint32_t) frame_raw[dev->frame_rows*dev->frame_cols+3] + ((uint32_t) frame_raw[dev->frame_rows*dev->frame_cols+4] << 16);
        memcpy(&envTemp,&envTempBits,sizeof(float));
        
        // Calculate the frame rate
        clock_gettime(CLOCK_REALTIME, &spec);         
        ns_old = ns; 
        ns = spec.tv_nsec;
        if ((ns != 0) && (ns_old != 0)) {
            rate_old = rate;
            rate = ceil(1.0e9f / (ns - ns_old));
        }
        if(rate > 32){
            rate = rate_old;
        }
        printf("camera %ld frame:%u rate:%u field:%u diode:%d env:%2.0f spot:%2.0f min: %2.0f max: %2.0f\n", index, frames++, rate, fieldcount, tDiodeTemp, envTemp, temp, min, max);
    }
   
    if (frame_color){
       delete [] frame_color;
    }
    if (frame_temperature){
       delete [] frame_temperature;
    }
    if (frame_raw){
       delete [] frame_raw;
    }
   
    if (SW_RETCODE_NONE != Seekware_Close(dev)) {
      fprintf(stderr, "Could not close PIR Device!\n");
    }
    return NULL;
}


static void SigIntHandler(int signum)
{
    printf("\nExit requested.\n");
    shouldexit = true;
}


int main(int argc, char *argv[])
{
    sw_retcode status;
    int num = 0;
    pthread_t tids[NUM_CAMS];

	printf("seekware-simple - Threaded real-time frame grabber and logger\n");

    memset(tids, 0, sizeof(tids));

    status = Seekware_Find(pl, NUM_CAMS, &num);
    if (status == SW_RETCODE_NONE) {
        
        signal(SIGINT, SigIntHandler);

		if (num == 0) {
			printf("Cannot find any cameras...exiting\n");
		}
		else {
			printf("Got %d camera devices. Press Control-C to exit...\n", num);
			for (long i = 0; i < num; ++i) {
				pthread_create(tids+i, NULL, thread_process, (void*)i);
			}
		
			for (long i = 0; i < num; ++i) {
				pthread_join(tids[i], NULL);
			}
		}
   }

   return 0;
}
