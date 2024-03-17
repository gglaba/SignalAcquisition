#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "rp.h"
#include "rp_hw-profiles.h"


void sendFile(char *filename){
    char copy_file[200];
    sprintf(copy_file, "scp %s grzesiula@192.168.122.42:/home/grzesiula", filename);
    int status = system(copy_file);
    if(status ==0){
        fprintf(stdout,"File sent succesfully");
          char delete_command[100];
        sprintf(delete_command, "rm %s", filename);

        // Execute the command to delete the local file
        system(delete_command);
        
        printf("Local file deleted.\n");
    } else {
        printf("Error occurred while sending the file.\n");
    }

}

void generateSignals()
{
    /* Reset Generation and */
    rp_GenReset();

    /* Generation */
    rp_GenSynchronise();

    rp_GenWaveform(RP_CH_1, RP_WAVEFORM_RAMP_UP);
    rp_GenFreq(RP_CH_1, 2000);
    rp_GenAmp(RP_CH_1, 1);

    rp_GenWaveform(RP_CH_2, RP_WAVEFORM_PWM);
    rp_GenFreq(RP_CH_2, 2000);
    rp_GenAmp(RP_CH_2, 1);

    rp_GenOutEnableSync(true);
    rp_GenSynchronise();
}

char getFileNameOrPath(int type){ //type == 1 for name type == 2 for path
    time_t t = time(NULL);
    char date_time = strftime(date_time, sizeof(date_time), "%Y%m%d_%H%M%S", localtime(&t));
    char filename[100];
    sprintf(filename, "CH1_CH2_%s.csv", date_time);
    char filepath[100];
    sprintf(filepath, "/tmp/%s", filename);
    if(type == 1){
        return filename;
    } else if(type == 2){
        return filepath;
    }
    return 0;

}


int Acquisition(){

    FILE *fptr;
    char *filepath = getFileNameOrPath(2);
    fptr = fopen(filepath, "w+");
    if(fptr == NULL){
        fprintf(stdout, "NIE UTWORZONO PLIKU");
        exit(1);
    }



    if (rp_Init() != RP_OK)
    {
        fprintf(fptr, "Rp api init failed!\n");
    }
    fprintf(fptr, "Rp api init success!\n");

    generateSignals();
    fprintf(fptr, "Signals generated");

    uint32_t buff_size = 16384;                                // buffer 16*1024
    float *buff1 = (float *)malloc(buff_size * sizeof(float)); // allocate memory for buffer
    float *buff2 = (float *)malloc(buff_size * sizeof(float));
    fprintf(fptr, "Buffer allocated\n");

    rp_AcqReset(); // reset the acquisition
    rp_AcqSetDecimation(RP_DEC_8);
    fprintf(fptr, "Decimation set\n");

    rp_AcqSetTriggerLevel(RP_CH_1, 0.1); // set trigger level to 1V
    // rp_AcqSetTriggerLevel(RP_CH_2,0.5);
    fprintf(fptr, "Trigger level set\n");

    rp_AcqSetTriggerDelay(0);
    rp_AcqSetArmKeep(true);
    rp_AcqStart(); // start the acquisition
    fprintf(fptr, "Acquisition started\n");

    sleep(0.2); // sleep for 1 second
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);
    rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
    while (1)
    {
        rp_AcqGetTriggerState(&state);
        if (state == RP_TRIG_STATE_TRIGGERED)
        {
            fprintf(fptr, "\n ACQUSITION TRIGGERED \n");
            sleep(1);
            break;
        };
    }

    bool fillState = false;
    while (!fillState)
    {
        rp_AcqGetBufferFillState(&fillState);
        fprintf(fptr, "\n Buffer not filled \n");
    }
    //rp_AcqStop();
    fprintf(fptr, "\n Buffer filled \n");
    uint32_t pos = 0;

    rp_AcqGetWritePointerAtTrig(&pos);

    for(int loop = 0; loop < 10; loop++){
        rp_AcqGetWritePointerAtTrig(&pos);
        fprintf(fptr,"\n%s %i","LOOP NUMBER",loop);
        fprintf(fptr,"\n%s %i","WRITER POS ",&pos);
        rp_AcqGetDataV(RP_CH_1, &pos, &buff_size, buff1);
        rp_AcqGetDataV(RP_CH_2, &pos, &buff_size, buff2);
        fprintf(fptr, "\n Data acquired \n");
        fprintf(fptr, "FROM BUFF1");
        for (int i = 0; i < buff_size; i++)
        {
            fprintf(fptr, "%f\n", buff1[i]);
        }
        fprintf(fptr, "\nFROM BUFF2\n");
        for (int i = 0; i < buff_size; i++)
        {
            fprintf(fptr, "%f\n", buff2[i]);
        }
    }



    sendFile(filepath)

    free(buff1);
    free(buff2);
    rp_Release();
    fprintf(fptr, "\n Buffer freed \n");
    fclose(fptr);
    return 0;
}


int main(int argc, char **argv)
{
    Acquisition();
    return 0;
}