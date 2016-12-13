#include "process.h"


int main(int argc, char *argv[]) {

    char usage[] = "Usage: processc --process-type [0 (R), 1 (W), 2 (S)]\n\t\t\t[--ip  xx.xx.xx.xx] [...] (servers)\n\t\t\t[--filesize  s] (default 1 KB)\n\t\t\t[--wait m] (default 100ms)\n\t\t\t[--algorithm ABD/SODAW(default)]\n\t\t\t[--code rlnc(default)/reed-solomon]\n\t\t\t--serverid name\n";


    Parameters parameters;

    srand(time(NULL));
    setDefaults(&parameters);
    if( readParameters(argc, argv, &parameters)==0)  {
        printf("%s\n", usage);
        exit(EXIT_FAILURE);
    }

    char buf[BUFSIZE];
    switch(parameters.processtype) {
    case reader:
        sprintf(buf, "reader-%d", rand());
        break;
    case writer:
        sprintf(buf, "writer-%d", rand());
        break;
    case server:
        sprintf(buf, "server-%d", rand());
        break;
    default:
        break;
    }
    strcpy(parameters.server_id, buf);

    printParameters(parameters);

    Server_Args *server_args =  get_server_args(parameters);
    Server_Status *server_status =  get_server_status(parameters);

    printf("Server Str : %s\n", server_args->servers_str);
    printf("MDS     : (%d, %d)\n", server_args->N, server_args->K);


    if(parameters.processtype==reader) {
        reader_process(parameters);
    } else if(parameters.processtype==writer) {
        writer_process(parameters);
    } else if(parameters.processtype==server) {
        server_process(server_args, server_status);
    }

    destroy_server_args(server_args) ;
    free(server_status);

    return 0;
}


void reader_process(Parameters parameters) {
    unsigned int opnum=2;

    write_initial_data(parameters);
    EncodeData *encoding_info = create_EncodeData(parameters);
    ClientArgs *client_args = create_ClientArgs(parameters);

    for( opnum=2; opnum< 450; opnum++) {
        usleep(parameters.wait*1000);

        printf("%s  %d  %s %s\n", parameters.server_id, opnum, client_args->servers_str, parameters.port);

        RawData *abd_data;
        if(parameters.algorithm==abd)  {
          abd_data = ABD_read("atomic_object", opnum, client_args);
          free(abd_data->data);
          free(abd_data->tag);
          free(abd_data);
        }
        

        if(parameters.algorithm==sodaw) {
          char *payload_read = SODAW_read("atomic_object", opnum,  encoding_info, client_args);
        }

/*
        if( is_equal(payload, payload_read, filesize) ) {
            printf("INFO: The data sets %d are equal!!\n", opnum);
        } else {
            printf("ERROR: The data sets %d are NOT equal!!\n", opnum);
        }
*/
    }
}

void writer_process(Parameters parameters) {
    unsigned int opnum=0;
    EncodeData *encoding_info = create_EncodeData(parameters);
    ClientArgs *client_args = create_ClientArgs(parameters);
    RawData *abd_data = create_RawData(parameters) ;

    for( opnum=0; opnum< 50000; opnum++) {
        unsigned int payload_size = (unsigned int) ( (parameters.filesize_kb + rand()%5)*1024);
        char *payload = get_random_data(payload_size);

        if(parameters.algorithm==abd) {
            abd_data->data = payload;
            abd_data->data_size = payload_size;
            ABD_write("atomic_object", opnum, abd_data, client_args);
        }

        if(parameters.algorithm==sodaw)
            SODAW_write("atomic_object", opnum, payload, payload_size, encoding_info, client_args);


/*
        if(parameters.algorithm==soda){
          char *payload_read = SODA_read("atomic_object", opnum,  encoding_info, client_args);
        }
        if(parameters.algorithm==casgc){
          char *payload_read = SODA_read("atomic_object", opnum,  encoding_info, client_args);
        }
*/

        free(payload);
    }
}

void write_initial_data(Parameters parameters) {

    unsigned int opnum=1;
    unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);

    unsigned int payload_size=filesize;
    char *payload = get_random_data(filesize);

    EncodeData *encoding_info = create_EncodeData(parameters);
    ClientArgs *client_args = create_ClientArgs(parameters);
    RawData *abd_data = create_RawData(parameters) ;

    abd_data->data = payload;
    abd_data->data_size = payload_size;

    if(parameters.algorithm==abd)
        ABD_write("atomic_object", opnum, abd_data, client_args);

    if(parameters.algorithm==sodaw)
        SODAW_write("atomic_object", opnum, payload, payload_size,  encoding_info, client_args);

    free(payload);
}



unsigned int readParameters(int argc, char *argv[], Parameters *parameters) {

    if(argc < 3) return 0;

    int i=0,j = 0;
    parameters->num_servers=0;
    for( i =0; i < argc; i++)
        if( strcmp(argv[i], "--ip")==0) parameters->num_servers++;

    parameters->ipaddresses =  (char **)malloc( parameters->num_servers *sizeof(char *));
    for( i =0; i < parameters->num_servers; i++)  {
        parameters->ipaddresses[i] = (char *)malloc( 16*sizeof(char));
    }


    for( i =1; i < argc; i++)  {
        if( strncmp(argv[i], "--ip", strlen("--ip"))==0) {
            strcpy( parameters->ipaddresses[j++], argv[++i]);
        } else if( strncmp(argv[i], "--serverid", strlen("--serverid"))==0) {
            strcpy(parameters->server_id, argv[++i]);
        } else if( strncmp(argv[i], "--process-type", strlen("-process-type") )==0) {
            parameters->processtype = atoi(argv[++i]);
        } else if( strcmp(argv[i], "--filesize")==0) {
            parameters->filesize_kb = atof(argv[++i]);
        } else if( strcmp(argv[i], "--wait")==0) {
            parameters->wait = atoi(argv[++i]);
        } else if( strcmp(argv[i], "--algorithm")==0) {
            i++;
            if( strcmp(argv[i], "ABD")==0) {
                parameters->algorithm = abd;
            }
            if( strcmp(argv[i], "SODAW")==0)
                parameters->algorithm = sodaw;
        } else if( strcmp(argv[i], "--code")==0) {
            i++;
            if( strcmp(argv[i], "full_vector")==0) {
                parameters->coding_algorithm = full_vector;
            } else  if( strcmp(argv[i], "reed_solomon")==0) {
                parameters->coding_algorithm = reed_solomon;
            } else {
                printf("ERROR: unknown coding algorithm choice \"%s\"\n",argv[i]);
                return 0;
            }
        } else {
            printf("ERROR: unknown argument \"%s\"\n",argv[i]);
            return 0;
        }
    }
    return 1;

}

