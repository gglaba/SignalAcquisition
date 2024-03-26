#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "rp.h"
#include "rp_hw-profiles.h"
#include "RP_Acquisition.h"
#include "RP_Communication.h"


bool generateSignals()
{
    /* Reset Generation and */
    rp_GenReset();

    /* Generation */
    rp_GenSynchronise();

    rp_GenWaveform(RP_CH_1, WAVEFORM1);
    if(rp_GenFreq(RP_CH_1, FREQ) != RP_OK){
        fprintf(stdout,"Problem with generating at this freq! \n");
        return false;
    }
    rp_GenAmp(RP_CH_1, 0.5);

    rp_GenWaveform(RP_CH_2, WAVEFORM2);
    rp_GenFreq(RP_CH_2, FREQ);
    rp_GenAmp(RP_CH_2, 0.5);

    rp_GenOutEnableSync(true);
    if (rp_GenSynchronise() != RP_OK)
    {
        return false;
    }
    return true;
}

int Acquisition()
{
    clock_t start, end;
    double time_passed;

    FILE *fptr;
    FILE *data;
    char *debugLog = GetFileNameOrPath(1);
    char *filepath = GetFileNameOrPath(2);

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

    uint32_t buff_size = BUFFER_SIZE;                          // default buffer 16*1024 floats
    float *buff1 = (float *)malloc(buff_size * sizeof(float)); // allocate memory for both channels buffers
    float *buff2 = (float *)malloc(buff_size * sizeof(float));
    fprintf(fptr, "Buffer allocated\n");

    rp_AcqReset();              // reset the acquisition
    rp_AcqSetDecimation(DECIM); // set decimation
    fprintf(fptr, "Decimation set\n");

    rp_AcqSetTriggerDelay(DELAY); // trigger set to 0 (means that it starts at buff[8192])
    rp_AcqSetArmKeep(true);   // keeps acquistion triggered
    rp_AcqStart();            // start the acquisition
    fprintf(fptr, "Acquisition started\n");

    //sleep(0.2);                                        // sleep needed to acquire fresh samples
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);           // setting trigger source to external trigger
    rp_acq_trig_state_t state = RP_TRIG_STATE_WAITING; //!!!!! CHANGE TO TRIGGERED IF NOT WORKING
    while (1)
    {
        rp_AcqGetTriggerState(&state);
        if (state == RP_TRIG_STATE_TRIGGERED) // if state triggered, break the loop
        {
            fprintf(fptr, "\n ACQUSITION TRIGGERED \n");
            //sleep(1);
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
    fprintf(stdout, "Reading signals \n");

    rp_AcqGetWritePointerAtTrig(&pos);    // get the position of the write pointer at the trigger
    start = clock();                      // measure time of acquisition
    for (int loop = 0; loop < 64; loop++) // acquire full buffer of 16*1024 samples 64 times (maximum amount of rows in csv file)
    {
        rp_AcqGetWritePointerAtTrig(&pos);

        rp_AcqGetDataV(RP_CH_1, &pos, &buff_size, buff1);
        rp_AcqGetDataV(RP_CH_2, &pos, &buff_size, buff2);

        // rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff1);
        // rp_AcqGetOldestDataV(RP_CH_2, &buff_size, buff2);

        fprintf(fptr, "\n Data acquired \n");
        for (int i = 0; i < buff_size; i++)
        {
            fprintf(data, "%f,%f\n", buff1[i], buff2[i]); // write data to the file
        }
    }
    end = clock();

    time_passed = ((double)(end - start)) / CLOCKS_PER_SEC;
    fprintf(fptr, "\n Acquisition time: %f \n", time_passed);

    fflush(fptr);
    fflush(data);

    SendFile(filepath,false); // send file to computer
    SendFile(debugLog,true); // send debug log to computer

    // releasing resources
    free(buff1);
    free(buff2);

    rp_Release();
    fprintf(stdout, "\n Memory freed \n");

    fclose(fptr);
    fclose(data);
    free(filepath);
    free(debugLog);

    return 0;
}

int main(int argc, char **argv)
{
    Acquisition();
    return 0;
}
