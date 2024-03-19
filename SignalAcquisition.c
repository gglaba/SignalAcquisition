#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "rp.h"
#include "rp_hw-profiles.h"

#define FILENAME_SIZE 100
#define FILEPATH_SIZE 200
#define FREQ 25000
#define DECIM 8
#define BUFFER_SIZE 16384

void sendFile(char *filename)
{
    char copy_file[200];
    sprintf(copy_file, "scp %s grzesiula@192.168.122.42:/home/grzesiula/Desktop/Inżynierka/SignalAcquisition/Data", filename);
    int status = system(copy_file);
    if (status == 0)
    {
        fprintf(stdout, "File sent succesfully");
        char delete_command[100];
        sprintf(delete_command, "rm %s", filename);

        int delete_status = system(delete_command);

        if (delete_status == 0)
        {
            fprintf(stdout, "\nLocal file deleted.\n");
        }
        else
        {
            fprintf(stdout, "Error while deleting the file.\n");
        }
    }
    else
    {
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
    rp_GenFreq(RP_CH_1, FREQ);
    rp_GenAmp(RP_CH_1, 1);

    rp_GenWaveform(RP_CH_2, RP_WAVEFORM_PWM);
    rp_GenFreq(RP_CH_2, FREQ);
    rp_GenAmp(RP_CH_2, 1);

    rp_GenOutEnableSync(true);
    rp_GenSynchronise();
}

char *getFileNameOrPath(int type) { //type == 1 for name type == 2 for path
    time_t t = time(NULL);
    char date_time[FILENAME_SIZE]; 
    strftime(date_time, sizeof(date_time), "%Y_%m_%d_%H_%M_%S", localtime(&t));
    char *filename = malloc(FILENAME_SIZE * sizeof(char));
    char *filepath = malloc(FILEPATH_SIZE * sizeof(char));
    sprintf(filename, "CH1_CH2_%s.csv", date_time);
    sprintf(filepath, "/tmp/%s", filename);
    if (type == 1) {
        return filename;
    } else if (type == 2) {
        return filepath;
    }
    return NULL;
}

int Acquisition()
{

    FILE *fptr;
    FILE *data;
    char *filepath = getFileNameOrPath(2);
    fptr = fopen("/tmp/DebugLog.txt", "w+");
    if (filepath != NULL)
    {
        data = fopen(filepath, "w+");
    }
    if (fptr == NULL || data == NULL)
    {
        fprintf(stdout, "one of the files was not created!");
        exit(1);
    }
    fprintf(data, "CH1V,CH2V\n");

    if (rp_Init() != RP_OK)
    {
        fprintf(fptr, "Rp api init failed!\n");
    }
    fprintf(fptr, "Rp api init success!\n");

    generateSignals();
    fprintf(fptr, "Signals generated");

    uint32_t buff_size = BUFFER_SIZE;                                // buffer 16*1024
    float *buff1 = (float *)malloc(buff_size * sizeof(float)); // allocate memory for buffer
    float *buff2 = (float *)malloc(buff_size * sizeof(float));
    fprintf(fptr, "Buffer allocated\n");

    rp_AcqReset(); // reset the acquisition
    rp_AcqSetDecimation(DECIM);
    fprintf(fptr, "Decimation set\n");

    //rp_AcqSetTriggerLevel(RP_CH_1, 0.1); // set trigger level to 1V
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
    // rp_AcqStop();
    fprintf(fptr, "\n Buffer filled \n");
    uint32_t pos = 0;

    rp_AcqGetWritePointerAtTrig(&pos);
    for (int loop = 0; loop < 64; loop++)
    {
        //rp_AcqGetWritePointerAtTrig(&pos);

        rp_AcqGetDataV(RP_CH_1, &pos, &buff_size, buff1);
        rp_AcqGetDataV(RP_CH_2, &pos, &buff_size, buff2);

        fprintf(fptr, "\n Data acquired \n");
        for (int i = 0; i < buff_size; i++)
        {
            fprintf(data, "%f,%f\n", buff1[i], buff2[i]);
        }
    }

    sendFile(filepath);

    free(buff1);
    free(buff2);
    free(filepath);

    rp_Release();
    fprintf(fptr, "\n Memory freed \n");
    
    fclose(fptr);
    fclose(data);
    return 0;
}

int main(int argc, char **argv)
{
    Acquisition();
    return 0;
}


