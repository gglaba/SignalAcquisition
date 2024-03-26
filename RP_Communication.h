#ifndef RP_COMMUNICATION_H
#define RP_COMMUNICATION_H

#include <time.h>
#include <string.h>
#include "rp.h"
#include "rp_hw-profiles.h"

#define FILENAME_SIZE 100
#define FILEPATH_SIZE 200
#define MAIN_PATH "/home/grzesiula/Desktop/Inżynierka/SignalAcquisition"

char* ScpIPCheck();
int SendFile();
char *GetFileNameOrPath(int type);
int ConnectSecondaryRP(char* ip);

#endif