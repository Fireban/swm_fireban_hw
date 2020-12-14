
#include <stdio.h>
#include <string.h>
#include <seekware/seekware.h>

#define NUM_CAMS            5

int main(int argc, char *argv[])
{
   printf("seekware-upload-firmware - uploads new firmware to a Seek Device\n");
    
   if(strcmp(argv[argc -1],"--help") == 0 || strcmp(argv[argc -1],"seekware-upgrade") == 0 || strcmp(argv[argc -1],"./seekware-upgrade") == 0 || argc > 4){
       printf("Usage: seekware-upgrade [UPGRADE FILE]\n");
       return 0;
   }
    
    sw_retcode status;
    psw dev = NULL;

    psw pl[NUM_CAMS];
    int num = 0;
    status = Seekware_Find(pl, NUM_CAMS, &num);
    if (status == SW_RETCODE_NONE) {
            if (num == 0) {
                    printf("Cannot find any cameras...exiting\n");
                    return 0;
            }
            else {
                    printf("Found %d camera devices...\n", num);
                    dev = pl[0];
            }
   } else {
        printf("Error finding device...exiting\n");
        return status;
   }
   
   int rtn = 0;
   rtn = Seekware_Open(dev);
   printf("Opening device...\n");
   
   int timeout = 60000;
   Seekware_SetSetting(dev,SETTING_TIMEOUT,timeout);
   
   rtn = Seekware_UploadFirmware(dev,argv[argc -1]);
   if(rtn){
        printf("Firmware Upgrade Failed!\n");
   } else{
       printf("Firmware Upgrade Successful!\n");
   }
   
   printf("Closing device...\n");
   rtn = Seekware_Close(dev);
   return rtn;
}
 