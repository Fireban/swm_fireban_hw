#ifndef	PmodGPS_H
#define	PmodGPS_H

#define BUF_SIZE	128


typedef enum{
	INVALID = 0,
	GGA,
	GSA,
	GSV,
	RMC,
	VTG
} NMEA;

typedef struct SATELLITE_T {
	int ID;
	int ELV;
	int AZM;
	int SNR;
} SATELLITE;

typedef struct GGA_DATA_STRUCT {
	char UTC[11];
	char LAT[14];
	char NS;
	char LONG[15];
	char EW;
	char PFI;
	char NUMSAT[3];
	char HDOP[5];
	char ALT[10];
	char AUNIT;
	char GSEP[5];
	char GUNIT;
	char AODC[11];
	char CHECKSUM[3];
} GGA_DATA_T;

GGA_DATA_T GGA_data;
GGA_DATA_T GPS_data;

extern int getData(void);

#endif	// PmodGPS_H
