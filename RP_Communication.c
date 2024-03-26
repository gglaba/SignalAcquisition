#include "RP_Communication.h"

char* ScpIPCheck() { //in case of ip/pc change 
    char *ssh_client = getenv("SSH_CLIENT");
    char *ip_address;

    if (ssh_client != NULL) {
        ip_address = (char *)malloc(20 * sizeof(char));
        if (ip_address == NULL) {
            fprintf(stdout, "Error allocating memory\n");
            return NULL;
        }
        sscanf(ssh_client, "%[^ ]", ip_address); //getting ip for scp later
    } else {
        fprintf(stdout, "ssh_client NULL! \n");
    }

    return ip_address;

}

int SendFile(char *filename,bool isDebug) // i know that this function is a brute force, but atm it works and doesnt require nfs which isnt working on my RP
{
    char *ip = ScpIPCheck();
    char *hostname = "grzesiula";
    char *debugPath;
    char *dataPath;
    sprintf(debugPath, "%s/Debug", MAIN_PATH);
    sprintf(dataPath, "%s/Data", MAIN_PATH);


    if(ip == NULL){
        fprintf(stdout,"IP is NULL!\n");
        return -1;
    }
    if (filename == NULL || access(filename, F_OK) == -1 || access(filename, R_OK) == -1)
    { // check if file exists and is readable
        fprintf(stdout, "File does not exist or is not readable \n");
        return -1;
    }

    char copy_file[200];
    if (isDebug)
    {
        sprintf(copy_file, "scp %s grzesiula@%s:%s", filename,ip,debugPath);
    }
    else
    {
        sprintf(copy_file, "scp %s grzesiula@%s:%s", filename,ip,dataPath);
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
            printf("\n Local file deleted \n");
        }
        else
        {
            printf( "Error deleting file \n");
        }
    }
    else
    {
        printf("Error sending the file \n");
        return -1;
    }

    free(ip);
    return 0;
}

char *GetFileNameOrPath(int type)
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