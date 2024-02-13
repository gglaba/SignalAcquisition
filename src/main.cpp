#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <iostream>
#include <fstream>
#include "main.h"


CBooleanParameter START_ACQ("START_ACQ",CBaseParameter::RW,false,0);
//CFloatParameter IS_CONNECTED("IS_CONNECTED",CBaseParameter::RO,0,0);

void AcquireSignal(){
    std::fstream data;
    data.open("/tmp/data.txt",std::fstream::in | std::fstream::out | std::fstream::app);
    uint32_t buff_size = 16384;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    rp_AcqReset();
        rp_AcqSetDecimation(RP_DEC_8);
        //rp_AcqSetTriggerLevel(RP_T_CH_2, 1.0);
        //rp_AcqSetTriggerDelay(50);

        rp_AcqStart();

        /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
        /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
        /*length and smaling rate*/

        sleep(5);
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

        while(1){
                rp_AcqGetTriggerState(&state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                break;
                }
        }

        bool fillState = false;
        while(!fillState){
		rp_AcqGetBufferFillState(&fillState);
	}

    rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff);
    int i;
    for (i = 0; i < buff_size; i++)
    {
        data << buff[i] << "   ";
    }
    /* Releasing resources */
    free(buff);
    data.close();
}

void debugLog(std::string message){
    std::fstream fs;
    fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::trunc);
    fs << message << "\n";
    fs.close();
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
    srand(time(0));
	CDataManager::GetInstance()->SetParamInterval(100);

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


void UpdateParams(void){
    debugLog("START_ACQ VALUE: " + START_ACQ.Value() ? "true" : "false");
    if(START_ACQ.IsNewValue()){
        START_ACQ.Update();
    }

}


void OnNewParams(void) {

   START_ACQ.IsNewValue();
    START_ACQ.Update();
 
    if (START_ACQ.Value() == true)
    {
        AcquireSignal();
        START_ACQ.Set(false);
    }   
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}

