/**
@file seekware.h
@brief This file contains declaration of the Seekware SDK
*/

#ifndef _SEEKWARE_PUBLIC_H
#define _SEEKWARE_PUBLIC_H

#include <inttypes.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Device related constants
#define RAW_COLS_206                208
#define RAW_ROWS_206                156
#define COLS_206                    206
#define ROWS_206                    156
#define RAW_PIXEL_SIZE_206          2

#define RAW_COLS_320                342
#define RAW_ROWS_320                260
#define COLS_320                    320
#define ROWS_320                    240
#define RAW_PIXEL_SIZE_320          2

#define FRAMEPIXELS_206             (RAW_COLS_206 * RAW_ROWS_206)
#define FRAMEPIXELS_320             (RAW_COLS_320 * RAW_ROWS_320)
#define FRAMESIZE_206               (FRAMEPIXELS_206 * RAW_PIXEL_SIZE_206)
#define FRAMESIZE_320               (FRAMEPIXELS_320 * RAW_PIXEL_SIZE_320)

#define MAX_COLS                    RAW_COLS_320
#define MAX_ROWS                    RAW_ROWS_320
#define MAX_PIXEL_SIZE              RAW_PIXEL_SIZE_320
#define MAXFRAMESIZE                FRAMESIZE_320

#define NR_LUTCOLORS                256

/**
* Defines the list of settings.
* Seekware_SetSetting and Seekware_GetSetting are valid for all settings.
*/
typedef enum sw_settings {
	SETTING_ACTIVE_LUT,
	SETTING_TEMP_UNITS,
	SETTING_TIMEOUT,
	SETTING_SOFTKNEE
} sw_settings;

/**
* Defines the list of LUTs that can be applied to display video frames.
*/
typedef enum sw_display_lut {
	SW_LUT_WHITE,
	SW_LUT_BLACK,
	SW_LUT_IRON,
	SW_LUT_COOL,
	SW_LUT_AMBER,
	SW_LUT_INDIGO,
	SW_LUT_TYRIAN,
	SW_LUT_GLORY,
	SW_LUT_ENVY,
	SW_LUT_NEWWHITE,
	SW_LUT_NEWBLACK,
	SW_LUT_SPECTRA,
	SW_LUT_PRISM,
	SW_LUT_NEWTYRIAN,
	SW_LUT_NEWIRON,
	SW_LUT_NEWAMBER,
	SW_LUT_HI,
	SW_LUT_HILO,
	SW_LUT_LAST,
} sw_display_lut;

/**
Defines the list of temperatures that can be applied.
*/
typedef enum sw_temp_units {
	SW_TEMP_F,					// Fahrenheit 
	SW_TEMP_C,					// Celsius
	SW_TEMP_K					// Kelvin
} sw_temp_units;

/**
Error codes that are returned from Seekware function calls.
*/
typedef enum sw_retcode {
	SW_RETCODE_NONE,			// No error has been detected. 
	SW_RETCODE_OPENEX,			// Device is already opened exclusively
	SW_RETCODE_BPARAM,			// Bad parameter.
   SW_RETCODE_DONOTPROCESS		// IP Do Not Process 
} sw_retcode;

/**
More error codes.
*/

typedef enum sw_exerrcode {
	SW_ERR_NONE,				// No error has been detected. 
	SW_ERR_FIND_OFLOW			// Found more objects than could be filled into the buffer.
} sw_exerrcode;

/**
This structure contains camera specific information to describe attached
devices.
*/
typedef struct sw {
	// Device Information
	unsigned char model[16];
	unsigned char serialNumber[12];
	uint8_t fw_version_major;
	uint8_t fw_version_minor;
	uint8_t fw_build_major;
	uint8_t fw_build_minor;
	uint16_t frame_rows;
	uint16_t frame_cols;


#ifdef defined _WIN32 || defined _WIN64
	// Windows Fields
	char * win_dev_path;
	FILE * win_dev_handle;
#else
#error "Platform was not defined."
#endif

	// Latest return code;
	sw_retcode retcode;

	// SDK Private Data
	void *ipPrivate;

	uint16_t rawframe_rows;
	uint16_t rawframe_cols;
} sw, *psw;

/**
Definition for the info param of Seekware_GetSdkInfo
*/
typedef struct sw_sdk_info {												
	uint8_t sdk_version_major; // SDK version number 
	uint8_t sdk_version_minor;
	uint8_t sdk_build_major;
	uint8_t sdk_build_minor;
	uint8_t lib_version_major; // Library version number 
	uint8_t lib_version_minor;
	uint8_t lib_build_major;
	uint8_t lib_build_minor;
} sw_sdk_info;

typedef struct COLOR_ARGB {
	union {
		uint32_t color;
		struct {
			uint8_t a;
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
	};
} COLOR_ARGB;

typedef COLOR_ARGB PALETTE[];

typedef enum PALETTE_INDEX {
	PALETTE_grayscale,
	PALETTE_cool,
	PALETTE_fire,
	PALETTE_glow,
	PALETTE_ica,
	PALETTE_spectrum,
	PALETTE_smart,
	PALETTE_unionjack,
	PALETTE_yellowhot,
	PALETTE_coldest,
	PALETTE_hottest,
	PALETTE_redhot,
	PALETTE_sepia,
	PALETTE_royal,
	PALETTE_jet,
	PALETTE_gem,
	PALETTE_black,
	PALETTE_petcolor,
	PALETTE_hotmetalblue,
	PALETTE_hotiron,
	// New Crew
	PALETTE_white = 100,
	PALETTE_blacknew,
	PALETTE_spectra,
	PALETTE_prism,
	PALETTE_iron,
	PALETTE_tyrian,
	PALETTE_amber,
	PALETTE_hi,
	PALETTE_hilo
} PALETTE_INDEX;


// Function Prototypes

/// Find the connected Seekware devices
/*! Search the target environment for all connected devices.<br>
Then, starting at index zero, fills the given array with a pointer to
a device structure for each connected device up to length, then sets
numfound to the number of devices found. If there are more than 'length'
devices connected fills the array as much as possible, sets numfound = length,
and returns the error code SW_RETCODE_OFLOW.
@param  pswlist     A 'length' sized array of pointers. The pointers
will be allocated by this function and must be freed
by calling the function \ref Seekware_Free().
@param  length      The size of the array 'pswlist'.
@param  numfound    Returns the number of allocated objects. Note that the
remaining part of the given array is not modified by
this function.
*/
sw_retcode Seekware_Find(psw pswlist[], int length, int *numfound);

/**
Open the device for use, populate internal elements in *sw,
allocate any memory or events necessary for operation then return.
Open devices are available exclusively to the SDK that opened
them. A call to this function on a device that is already open
should return SW_RETCODE_OPENEX.

@param id A pointer to a Seekware device structure
@return sw_retcode SW_RETCODE_NONE on success
*/
sw_retcode Seekware_Open(psw id);

/**
Close the device, release any memory and terminate any events.
@param id A pointer to a Seekware device structure
@return sw_retcode SW_RETCODE_NONE on success
*/
sw_retcode Seekware_Close(psw id);

/**
Returns a structure containing information about the SDK.

@param info A pointer to a sw_dev_info structure.
@return sw_retcode SW_RETCODE_NONE on success
*/
sw_retcode Seekware_GetSdkInfo(sw_sdk_info *info);

/**
Gets the value of the requested setting.

@param id A pointer to a Seekware device structure
@param index The setting index
@param value A pointer to the location to write the setting value
@return sw_retcode SW_RETCODE_NONE on success
*/
sw_retcode Seekware_GetSetting(psw id, sw_settings index, int *value);

/**
Sets the value of the requested setting.

@param id A pointer to a Seekware device structure
@param index The setting index
@param value The setting value
@return sw_retcode SW_RETCODE_NONE on success
*/
sw_retcode Seekware_SetSetting(psw id, sw_settings index, int value);

/**
Uploads firmware

@param id A pointer to a Seekware device structure
@param filename A pointer to the filename of the uploaded firmware
@return sw_retcode SW_RETCODE_NONE on success
*/
sw_retcode Seekware_UploadFirmware(psw id, char *filename);		

/**
Converts a frame of corrected image.

@param id A pointer to a Seekware device structure
@param binaryImage - A pointer to a buffer to hold raw image data
@param temperatureImage – A pointer to a buffer to hold temperature data
@param display – A pointer to a buffer to hold display data
@return sw_retcode SW_RETCODE_NONE on success

@note: If a buffer is supplied, then it must be appropriately sized
based on the data type and the number of pixels in the image. If
a buffer is not supplied, then the computations and memory
allocation required for that function shall not be performed.
*/
sw_retcode Seekware_GetImage(psw id, uint16_t *binaryImage, float *temperatureImage, uint32_t *display);

/**
Gets the palette.

@param context A pointer to	a Seekware device structure
@param index The palette index 
@param palette A pointer to the location of the palette
@return The palette of the data
*/
sw_retcode Analysis_GetPalette(void *context, PALETTE_INDEX index, PALETTE *palette);

/**
Gets the min and max thermography data.

@param context A pointer to	a Seekware device structure
@param thermographyData	A pointer to a buffer to hold thermography data
@param min A pointer to the location of the min value
@param min A pointer to the location of the max value
@return The min and max values of the thermography data
*/
sw_retcode Analysis_GetThermographyMinMax(void *context, float *thermographyData, float *min, float *max);


#ifdef __cplusplus
	}
#endif

#endif