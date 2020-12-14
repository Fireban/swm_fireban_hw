#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h>
#include <fcntl.h>
#include "GPS.h"

void formatCOORDS(char* coords);
void formatGGA(char* data_array);
int chooseMode(char recv[BUF_SIZE]);

float calculate_gps(char *gps_data, int add)
{
	char gps_deg[3];
	char gps_min[7];

	float result;

	memcpy(gps_deg, gps_data, add);
	memcpy(gps_min, gps_data+add, 7);

	result = atof(gps_deg) + atof(gps_min)/60;

	return result;
}


int getData(void)
{
	char recv[BUF_SIZE] = {0};
	FILE* fd;
	//int fd;
	struct	termios newtio;			// stop bit 크기 등의 시리얼 통신 환경 설정을 위한 termios  구조체
	//struct	pollfd	poll_events;
	//int		poll_state;

	NMEA mode = INVALID;
	
	//fd = open("/dev/serial0", O_RDWR | O_NOCTTY);		// O_RDWR : 읽기 쓰기 모드, O_NOCTTY : 시리얼 통신 장치에 맞춰 추가
	fd = fopen("/dev/serial0", "r");

	if(fd < 0) { fprintf(stderr, "ERR\n");	exit(-1); }

	memset(&newtio, 0, sizeof(newtio));
	printf("open complete\n");

	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = 0;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	//tcflush(fd, TCIFLUSH);
	//tcsetattr(fd, TCSANOW, &newtio);			// 포트에 대한 통신 환경 설정
	//fcntl(fd, F_SETFL, FNDELAY);
#if 0
	poll_events.fd		= fd;
	poll_events.events	= POLLIN | POLLERR;
	poll_events.revents	= 0;

	poll_state = poll((struct pollfd*)&poll_events, 1, 1000);
#endif
	//sleep(1);

	if(fgets(recv, BUF_SIZE, fd) != NULL)
		mode = chooseMode(recv);
	else {
		fclose(fd);
		return INVALID;
	}

	switch(mode) {
		case(GGA):	formatGGA(recv);	break;				
		case(GSA):	break;
		case(GSV):	break;
		case(RMC):	break;
		case(VTG):	break;
		case(INVALID):	return INVALID;
	}
	fclose(fd);
	return atoi(GGA_data.LAT);;
}


int chooseMode(char recv[BUF_SIZE]) {
	int mode = INVALID;
	if (((recv[3]) == 'G') && ((recv[4]) == 'G') && (recv[5] == 'A'))
		
	{
		mode=GGA;
	}
	else if (((recv[3]) == 'G') && ((recv[4]) == 'S') && (recv[5] == 'A'))
	{
		mode=GSA;
	}
	else if (((recv[3]) == 'G') && ((recv[4]) == 'S') && (recv[5] == 'V'))
	{
		mode=GSV;
	}
	else if (((recv[3]) == 'R') && ((recv[4]) == 'M') && (recv[5] == 'C'))
	{
		mode=RMC;
	}
	else if (((recv[3]) == 'V') && ((recv[4]) == 'T') && (recv[5] == 'G'))
	{
		mode=VTG;
	}
	return mode;
}

void formatGGA(char* data_array)
{
	enum cases {UTC, LAT, NS, LONG, EW, PFI, NUMSAT, HDOP, ALT, AUNIT, GSEP, GUNIT, AODC};
	int datamember = UTC;
	char* start_ptr;
	char* end_ptr = data_array+7;			//Set start pointer after the message ID ("$GPGGA,")
	_Bool flag=1;
	char COORDbuf[14]={0};
	char checksum[3]={0};


	while (flag)
	{
		start_ptr = end_ptr;
		
		while (*end_ptr!=',' && (*(end_ptr+1)!=10)&& *end_ptr!='*')end_ptr++;//Increment ptr until a comma is found
		
		if (*end_ptr==10||*(end_ptr+1)==10||(*end_ptr=='*'&&*(end_ptr-1)==',')) {//End reached
			flag=0;
			break;
		}

		float temp;
		
		switch(datamember){
				case UTC:
					memcpy(GGA_data.UTC, start_ptr, (end_ptr - start_ptr));
					GGA_data.UTC[end_ptr - start_ptr] = '\0';		//End null char
					datamember = LAT;
					break;
				case LAT:
					memcpy(COORDbuf, start_ptr, (end_ptr - start_ptr));
					if (*COORDbuf) {
						//formatCOORDS(COORDbuf);
						memcpy(GGA_data.LAT, COORDbuf, 13);
						temp = calculate_gps(GGA_data.LAT, 2);
						sprintf(GGA_data.LAT, "%f", temp);
						memset(COORDbuf, 0, 13);	
					}
					datamember=NS;
					break;
				case NS:
					if (*start_ptr!=',') {
						GGA_data.NS = *start_ptr;
						//strncat(GGA_data.LAT, start_ptr, 1);
					}
					datamember=LONG;
					break;
				case LONG:
					memcpy(COORDbuf, start_ptr, (end_ptr - start_ptr ));
					if (*COORDbuf) {
						//formatCOORDS(COORDbuf);						// 표현 형식 변환
						memcpy(GGA_data.LONG, COORDbuf, 14);
						temp = calculate_gps(GGA_data.LONG, 3);
						sprintf(GGA_data.LONG, "%f", temp);
					}
					datamember=EW;
					break;
				case EW:
					if (*start_ptr!=',') {
						GGA_data.EW = *start_ptr;
						//strncat(GGA_data.LONG, start_ptr, 1);
					}
					datamember=PFI;
					break;
				case PFI:
					if (*start_ptr!=',')GGA_data.PFI = *start_ptr;
					datamember=NUMSAT;
					break;
				case NUMSAT:
					memcpy(GGA_data.NUMSAT, start_ptr, (end_ptr - start_ptr));
					GGA_data.NUMSAT[end_ptr - start_ptr] = '\0';
					datamember=HDOP;
					break;
				case HDOP:
					memcpy(GGA_data.HDOP, start_ptr, (end_ptr - start_ptr));
					GGA_data.HDOP[end_ptr - start_ptr] = '\0';
					datamember=ALT;
					break;
				case ALT:
					memcpy(GGA_data.ALT, start_ptr, (end_ptr - start_ptr));
					GGA_data.ALT[end_ptr - start_ptr] = '\0';
					datamember=AUNIT;
					break;
				case AUNIT:
					if (*start_ptr!=',')GGA_data.AUNIT= *start_ptr;
					datamember=GSEP;
					break;
				case GSEP:
					memcpy(GGA_data.GSEP, start_ptr, (end_ptr - start_ptr));
					GGA_data.GSEP[end_ptr - start_ptr] = '\0';
					datamember=GUNIT;
					break;
				case GUNIT:
					if (*start_ptr!=',')GGA_data.GUNIT=*start_ptr;
					datamember=AODC;
					break;
				case AODC:
					memcpy(GGA_data.AODC, start_ptr, (end_ptr - start_ptr));
					GGA_data.AODC[end_ptr - start_ptr] = '\0';
					flag=0;
					break;

		}

			end_ptr++;				//Increment past the last comma
	}
	//Get checksum
	while(*(end_ptr)!=10)end_ptr++;
		checksum[0] = *(end_ptr - 3);
		checksum[1] = *(end_ptr - 2);
		checksum[2] = NULL;
	memcpy(GGA_data.CHECKSUM, checksum, 3);
	return;
}
		
void formatCOORDS(char* coords)
{
	char formatted[14]={0};
	int i=0;
	char* coordsstart= coords;

	while (*(coords)){

		formatted[i]=*coords;
		formatted[++i]; coords++;
		if (*(coords+2)=='.')
		{
			formatted[i]='C';		//degrees symbol
			i++;
		}
		else if (*coords=='.')
		{
			formatted[i] = '`';		// ' symbol for minutes
			i++;
			coords++;
		}
		else if (*(coords-3)=='.')
		{
			formatted[i]='.';		//Decimal for seconds
			i++;
		}
		else if (*(coords-5)=='.')
		{
			formatted[i]='"';		// " for seconds
			i++;
			formatted[i]='\0';		//Null char
		}
	}

	strcpy(coordsstart, formatted);

	//coords=formatted;
	return;

}
