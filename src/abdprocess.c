#include "abdprocess.h"


int main(int argc, char *argv[]) {

   char usage[] = "Usage: abdprocessc --process-type [0 (R), 1 (W), 2 (S)]\n\t\t\t[--ip  xx.xx.xx.xx] [...] (servers)\n\t\t\t[--filesize  s] (default 1 KB)\n\t\t\t[--wait m] (default 100ms)\n\t\t\t[--algorithm ABD/SODAW(default)]\n\t\t\t[--code rlnc(default)/reed-solomon]\n\t\t\t--serverid name\n";


   Parameters parameters;    
   
   srand(time(NULL));
   setDefaults(&parameters);
   if( readParameters(argc, argv, &parameters)==0)  {
      printf("%s\n", usage);
      exit(0);
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
   }
   else if(parameters.processtype==writer) {
       writer_process(parameters);
   }
   else if(parameters.processtype==server) {


       server_process(server_args, server_status);
   }
 
   return 0;
}


char *get_servers_str(Parameters parameters) {
     int i;
     char *q = (char *) malloc( 16*parameters.num_servers*sizeof(char));

     char *p = q;
     strncpy(p, parameters.ipaddresses[0], strlen(parameters.ipaddresses[0]));
     p += strlen(parameters.ipaddresses[0]);
     for(i=1; i < parameters.num_servers; i++) {
        *p = ' '; p++;
        strncpy(p, parameters.ipaddresses[i], strlen(parameters.ipaddresses[i]));
        p += strlen(parameters.ipaddresses[i]);
     } 
     *p = '\0';
     return q;
}


EncodeData *create_EncodeData(Parameters parameters) {

    unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);
    EncodeData *encoding_info  = (EncodeData *)malloc(sizeof(EncodeData));
    encoding_info->N = parameters.num_servers; 
    encoding_info->K= ceil((float)parameters.num_servers + 1)/2;
    encoding_info->symbol_size = SYMBOL_SIZE;
    encoding_info->raw_data_size = filesize;
    encoding_info->num_blocks = ceil( (float)filesize/(encoding_info->K*SYMBOL_SIZE));
 
    return encoding_info;
}

ClientArgs *create_ClientArgs(Parameters parameters) {

    char *servers_str = get_servers_str(parameters);

    ClientArgs *client_args  = (ClientArgs *)malloc(sizeof(ClientArgs));

    strcpy(client_args->client_id, parameters.server_id); 
    strcpy(client_args->port, parameters.port); 

    client_args->servers_str = (char *)malloc( (strlen(servers_str) +1)*sizeof(char));
    strcpy(client_args->servers_str, servers_str); 

    return client_args;
}


void reader_process(Parameters parameters) {
    unsigned int opnum=0;

    write_initial_data(parameters);

    EncodeData *encoding_info = create_EncodeData(parameters);

    ClientArgs *client_args = create_ClientArgs(parameters);

    unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);
    for( opnum=0; opnum< 2;opnum++) {
        usleep(parameters.wait*1000);
        char *payload = get_random_data(filesize);

   //   payload[5] ='4';        
        printf("%s  %d  %s %s\n", parameters.server_id, opnum, client_args->servers_str, parameters.port);

        //char *payload_read = SODAW_read("atomic_object", parameters.server_id, opnum, servers_str, parameters.port, &encoding_info);       
      //  char *payload_read = SODAW_read("atomic_object", opnum, servers_str, parameters.port, &encoding_info);       

        char *payload_read = SODAW_read("atomic_object", opnum,  encoding_info, client_args);       

/*
        for(i=0; i < filesize; i++) {
          printf("%c",payload_read[i]);
        }
        printf("\n");
*/


       
        if( is_equal(payload, payload_read, filesize) ) {
           printf("INFO: The data sets %d are equal!!\n", opnum);
        }
        else {
            printf("ERROR: The data sets %d are NOT equal!!\n", opnum);
            exit(0);
        }

       free(payload);
    } 
}


bool is_equal(char *payload1, char*payload2, unsigned int size) {
    int i =0;
    bool final=true;
    for(i=0; i <size ; i++) {
        if(payload1[i]!=payload2[i]) {
     //      printf("INFO: Mismatch at index %d  (%c %c)\n",i, payload1[i], payload2[i]);
           final=false;
      //     return false;
        }
    }
    return final; //true;
}


void writer_process(Parameters parameters) {
    unsigned int opnum=0;

    unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);
    
    unsigned int payload_size=filesize;

    EncodeData *encoding_info = create_EncodeData(parameters);
    ClientArgs *client_args = create_ClientArgs(parameters);
    for( opnum=0; opnum< 100000;opnum++) {

       char *payload = get_random_data(filesize);

       //SODAW_write("atomic_object", parameters.server_id, opnum, payload, payload_size,  servers_str, parameters.port);       

       SODAW_write("atomic_object", opnum, payload, payload_size,  encoding_info, client_args);       

       free(payload);
    }
}

void write_initial_data(Parameters parameters) {

    unsigned int opnum=0;
    unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);
    unsigned int payload_size=filesize;
    char *payload = get_random_data(filesize);

    EncodeData *encoding_info = create_EncodeData(parameters);
    ClientArgs *client_args = create_ClientArgs(parameters);

//    SODAW_write("atomic_object", parameters.server_id, opnum, payload, payload_size,  servers_str, parameters.port);       

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
       }
       else if( strncmp(argv[i], "--serverid", strlen("--serverid"))==0) { 
           strcpy(parameters->server_id, argv[++i]);
       }
       else if( strncmp(argv[i], "--process-type", strlen("-process-type") )==0) { 
          parameters->processtype = atoi(argv[++i]);
       }
       else if( strcmp(argv[i], "--filesize")==0) { 
          parameters->filesize_kb = atof(argv[++i]); }
       else if( strcmp(argv[i], "--wait")==0) { 
          parameters->wait = atoi(argv[++i]);
       }
       else if( strcmp(argv[i], "--algorithm")==0) { 
          i++;
          if( strcmp(argv[i], "ABD")==0) {
               parameters->algorithm = abd;
          }
          if( strcmp(argv[i], "SODAW")==0)
               parameters->algorithm = sodaw;
       }
       else if( strcmp(argv[i], "--code")==0) { 
          i++;
          if( strcmp(argv[i], "full_vector")==0)
               parameters->codingalgorithm = full_vector;

          if( strcmp(argv[i], "reed_solomon")==0) {
               parameters->codingalgorithm = reed_solomon;
          }
       }
       else { printf("ERROR: unknown argument \"%s\"\n",argv[i]); return 0; } } return 1;

}

void setDefaults(Parameters *parameters) {
     parameters->num_servers = 0;
     parameters->ipaddresses = NULL;
     parameters->algorithm = sodaw;
     parameters->codingalgorithm = full_vector;
     parameters->wait = 100;
     parameters->filesize_kb = 1.1;
     parameters->processtype = server;
     strcpy(parameters->port, "8081");
}

void printParameters(Parameters parameters) {
    int i; 

     printf("Parameters\n");
     printf("\tName  \t\t\t\t: %s\n", parameters.server_id);;

     switch(parameters.processtype) {
          case reader:
                printf("\tprocesstype\t\t\t: %s\n", "reader");
                break;
          case writer:
                printf("\tprocesstype\t\t\t: %s\n", "writer");
                break; 
          case server:
                printf("\tprocesstype\t\t\t: %s\n", "server");
                break;
          default:
                break;
     }

     printf("\tnum servers\t\t\t: %d\n", parameters.num_servers);
     for(i=0; i < parameters.num_servers; i++) {
          printf("\t\tserver %d\t\t: %s\n",i, parameters.ipaddresses[i] );
     }

     switch(parameters.algorithm) {
          case sodaw:
                printf("\talgorithm\t\t\t: %s\n", SODAW   );
                break;
          case abd:
                printf("\talgorithm\t\t\t: %s\n", ABD );
                break;
          default:
                break;
     }

     switch(parameters.codingalgorithm) {
          case full_vector:
                printf("\tcoding algorithm\t\t: %s\n", "RLNC"   );
                break;
          case reed_solomon:
                printf("\tcoding algorithm\t\t: %s\n", "REED-SOLOMON" );
                break;
          default:
                break;
     }
     printf("\tinter op wait (ms)\t\t: %d\n", parameters.wait);
     printf("\tfile size (KB)\t\t\t: %.2f\n", parameters.filesize_kb);
}


Server_Args * get_server_args( Parameters parameters) {



     Server_Args *server_args = (Server_Args *) malloc(sizeof(Server_Args));

     strcpy(server_args->server_id, parameters.server_id);


     server_args->servers_str =  get_servers_str(parameters);
     printf("servers args %s\n", server_args->servers_str);

     strcpy(server_args->server_id, parameters.server_id);
      
     strcpy(server_args->port, "8081");

     unsigned int filesize = (unsigned int) (parameters.filesize_kb*1024);
     server_args->init_data= get_random_data(filesize);
     server_args->init_data_size= filesize;

     server_args->sock_to_servers = NULL;
     server_args->symbol_size = 1400;
     server_args->coding_algorithm = parameters.codingalgorithm;
     server_args->N = parameters.num_servers;
     server_args->K = ceil((float)parameters.num_servers + 1)/2;
     server_args->status = NULL;


     return server_args;
}

Server_Status * get_server_status( Parameters parameters) {
     Server_Status *server_status = (Server_Status *) malloc(sizeof(Server_Status));

     server_status->network_data = 0;
     server_status->metadata_memory = 0;
     server_status->cpu_load = 0;
     server_status->time_point = 0;
     server_status->process_memory = 0;
     return server_status;
}


char * get_random_data(unsigned int size) {
   srand(23);
   int i;
   char *data = (char *)malloc( (size+1)*sizeof(char));

   for( i = 0 ; i < size ; i++ ) 
   {
      data[i] = 65 + rand()%25; 
      //data[i] = 65 + i%25; 
   }
   data[i]='\0';
   return data;
}
