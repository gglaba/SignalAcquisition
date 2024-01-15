#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <iostream>
#include <ofstream>
#include "main.h"


CBooleanParameter START_ACQ("START_ACQ",CBaseParameter::RW,false,0);
CFloatParameter IS_CONNECTED("IS_CONNECTED",CBaseParameter::RO,0,0);

void AcquireSignal(){

}


const char *rp_app_desc(void)
{
    return (const char *)"Fast Analog Acquisition.\n";
}


int rp_app_init(void)
{
        // Initialization of API
    if (rpApp_Init() != RP_OK)
    {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }
    else fprintf(stderr, "Red Pitaya API init success!\n");
    std::ofstream ofs;
    ofs.open("test.txt", std::ofstream::out | std::ofstream::app);
    ofs << " more lorem ipsum";
    ofs.close();

    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading  application\n");
    rpApp_Release();
    return 0;
}


int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}


int rp_get_params(rp_app_params_t **p)
{
    return 0;
}


int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}








void UpdateSignals(void){}


void UpdateParams(void){}


void OnNewParams(void) {

    START_ACQ.Update();
    if (START_ACQ.Value() == true)
    {
        std::ofstream debug("debuglog.txt",std::ofstream::out);
        debug << "smth happens";
        debug.close();
        START_ACQ.Set(false);
    }   
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}

