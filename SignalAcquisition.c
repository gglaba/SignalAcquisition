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

void sendFile(char *filename,bool isDebug) // i know that this function is a brute force, but atm it works and doesnt require nfts which isnt working on my RP
{
    if (filename == NULL || access(filename, F_OK) == -1 || access(filename, R_OK) == -1)
    { // check if file exists and is readable
        fprintf(stdout, "File does not exist or is not readable \n");
        exit(1);
    }

    char copy_file[200];
    if (isDebug)
    {
        sprintf(copy_file, "scp %s grzesiula@192.168.122.42:/home/grzesiula/Desktop/Inżynierka/SignalAcquisition/Debug", filename);
    }
    else
    {
        sprintf(copy_file, "scp %s grzesiula@192.168.122.42:/home/grzesiula/Desktop/Inżynierka/SignalAcquisition/Data", filename);
    }

    int status = system(copy_file); // send file to computer
    if (status == 0)
    {
        fprintf(stdout, "File sent succesfully");
        char delete_command[100];
        sprintf(delete_command, "rm %s", filename);

        int delete_status = system(delete_command); // delete file from the local machine

        if (delete_status == 0)
        {
            fprintf(stdout, "\n Local file deleted \n");
        }
        else
        {
            fprintf(stdout, "Error deleting file \n");
        }
    }
    else
    {
        printf("Error sending the file \n");
        exit(1);
    }
}

bool generateSignals()
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
    if (rp_GenSynchronise() != RP_OK)
    {
        return false;
    }
    return true;
}

char *getFileNameOrPath(int type)
{ // type == 1 for debug, type == 2 for path
    time_t t = time(NULL);
    char date_time[FILENAME_SIZE];
    strftime(date_time, sizeof(date_time), "%Y_%m_%d_%H_%M_%S", localtime(&t)); // get current date and time and format it to string

    if (type == 1)
    {
        char *debugfile = malloc(FILENAME_SIZE * sizeof(char)); // allocate memory for filename and filepath
        sprintf(debugfile, "/tmp/DEBUG_%s.txt", date_time);     // create debug filename
        return debugfile;
    }
    else if (type == 2)
    {
        char *filepath = malloc(FILEPATH_SIZE * sizeof(char));
        sprintf(filepath, "/tmp/CH1V_CH2V_%s.csv", date_time); // create filepath for data
        return filepath;
    }

    return NULL;
}

int Acquisition()
{
    clock_t start, end;
    double cpu_time_used;

    FILE *fptr;
    FILE *data;
    char *debugLog = getFileNameOrPath(1);
    char *filepath = getFileNameOrPath(2);

    fptr = fopen(debugLog, "w+");

    if (filepath != NULL)
    {
        data = fopen(filepath, "w+");
    }
    if (fptr == NULL || data == NULL)
    {
        fprintf(stdout, "One of the files was not created \n");
        exit(1);
    }
    fprintf(data, "CH1V,CH2V\n");

    if (rp_Init() != RP_OK)
    {
        fprintf(fptr, "Rp api init failed! \n");
    }
    fprintf(fptr, "Rp api init success! \n");

    if (generateSignals() == false)
    {
        fprintf(fptr, "Signal generation failed");
        return -1;
    }
    fprintf(fptr, "Signals generated");

    uint32_t buff_size = BUFFER_SIZE;                          // default buffer 16*1024
    float *buff1 = (float *)malloc(buff_size * sizeof(float)); // allocate memory for both channels buffers
    float *buff2 = (float *)malloc(buff_size * sizeof(float));
    fprintf(fptr, "Buffer allocated\n");

    rp_AcqReset();              // reset the acquisition
    rp_AcqSetDecimation(DECIM); // set decimation
    fprintf(fptr, "Decimation set\n");

    rp_AcqSetTriggerDelay(0); // trigger set to 0 (means that it starts at buff[8192])
    rp_AcqSetArmKeep(true);   // keeps acquistion triggered
    rp_AcqStart();            // start the acquisition
    fprintf(fptr, "Acquisition started\n");

    sleep(0.2);                                        // sleep needed to acquire fresh samples
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);           // setting trigger source to external trigger
    rp_acq_trig_state_t state = RP_TRIG_STATE_WAITING; //!!!!! CHANGE TO TRIGGERED IF NOT WORKING
    while (1)
    {
        rp_AcqGetTriggerState(&state);
        if (state == RP_TRIG_STATE_TRIGGERED) // if state triggered, break the loop
        {
            fprintf(fptr, "\n ACQUSITION TRIGGERED \n");
            sleep(1);
            break;
        };
    }

    bool fillState = false;
    while (!fillState) // wait until buffer is filled
    {
        rp_AcqGetBufferFillState(&fillState);
    }
    // rp_AcqStop();
    fprintf(fptr, "\n Buffer filled \n");
    uint32_t pos = 0; // position of the write pointer in the buff

    rp_AcqGetWritePointerAtTrig(&pos);    // get the position of the write pointer at the trigger
    start = clock();                      // measure time of acquisition
    for (int loop = 0; loop < 64; loop++) // acquire full buffer of 16*1024 samples 64 times (maximum amount of rows in csv file)
    {
        // rp_AcqGetWritePointerAtTrig(&pos);

        rp_AcqGetDataV(RP_CH_1, &pos, &buff_size, buff1);
        rp_AcqGetDataV(RP_CH_2, &pos, &buff_size, buff2);

        fprintf(fptr, "\n Data acquired \n");
        for (int i = 0; i < buff_size; i++)
        {
            fprintf(data, "%f,%f\n", buff1[i], buff2[i]); // write data to the file
        }
    }
    end = clock();

    time_passed = ((double)(end - start)) / CLOCKS_PER_SEC;
    fprintf(fptr, "\n Acquisition time: %f \n", time_passed);

    sendFile(filepath,false); // send file to computer
    sendFile(debugLog,true); // send debug log to computer

    // releasing resources
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
