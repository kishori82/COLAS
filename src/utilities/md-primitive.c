#include "md-primitive.h"

#define DEBUG_MODE  1


void md_meta_send(void *sock_to_servers, int num_servers, char *names[],  int n, ...) {
    RawData *rawdata;
    va_list valist;
    int i =0, j;

    va_start(valist, n);

    void **values = (void **)malloc(n*sizeof(void *));

    /*
        for(i=0; i < n; i++)  {
           values[i] = (void *)malloc(10*sizeof(void));
        }
    */

    zframe_t **frames = (zframe_t **)malloc(n*sizeof(zframe_t *));
    assert(values!=NULL);
    assert(frames!=NULL);
    for(i=0; i < n; i++ ) {
        if( strcmp(names[i], OPNUM)==0)   {
            values[i] = (void *)va_arg(valist, unsigned  int *);
            frames[i]= zframe_new( (const void *)values[i], sizeof(unsigned int));
            //frames[i]= zframe_new((const void *)values[i], sizeof(*values[i]));
        }
        else if( strcmp(names[i], PAYLOAD)==0)   {
            rawdata = va_arg(valist, RawData *);
            values[i] = rawdata;
            frames[i]= zframe_new(rawdata->data, rawdata->data_size);
        } else {
            values[i] = va_arg(valist, char *);
            frames[i]= zframe_new(values[i], strlen((char *)values[i]));
        }
    }
    va_end(valist);

    // it to all servers in a round robin fashion
    int rc;
    if(DEBUG_MODE)  printf("\n");
    if(DEBUG_MODE) printf("\t\tsending ..\n");
    for(i=0; i < num_servers; i++) {
        printf("\t\t\tserver : %d\n", i);
        for(j=0; j < n-1; j++) {
            if(DEBUG_MODE) {
                if( strcmp(names[j], OPNUM)==0)
                    printf("\t\t\tFRAME%d :%s  %u\n",j, names[j], *((unsigned int *)values[j]) );
                else if( strcmp(names[j], PAYLOAD)==0)
                    printf("\t\t\tFRAME%d :%s  %d\n", j, names[j],  ((RawData *)values[j])->data_size);
                else
                    printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);

                rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE +  ZFRAME_MORE);

                if( rc < 0) {
                    printf("ERROR: %d\n", rc);
                    exit(EXIT_FAILURE);
                }

                assert(rc!=-1);

            }
        }


        rc = zframe_send(&frames[j], sock_to_servers, ZFRAME_REUSE + ZFRAME_DONTWAIT);

        if(DEBUG_MODE) {
            if( strcmp(names[j], OPNUM)==0) {
                printf("\t\t\tFRAME%d :%s  %u\n", j, names[j],   *((unsigned int *)values[j]) );
            } else if( strcmp(names[j], PAYLOAD)==0) {
                printf("\t\t\tFRAME%d :%s  %d\n", j, names[j],  ((RawData *)values[j])->data_size);
            } else {
                printf("\t\t\tFRAME%d :%s  %s\n", j, names[j],   (char *)values[j]);
            }
        }

        if( rc < 0) {
            printf("ERROR: %d\n", rc);
            exit(EXIT_FAILURE);
        }

        if(DEBUG_MODE)  printf("\n");
    }

    printf("\n");
    if(DEBUG_MODE)  printf("\n");

    //!! Do we not need to free the inner loopo?
//!! potential memory conflict
    /*
    for(i=0; i < n; i++ ) {
    free(values[i]);
    }
    */
    if( values!=NULL) free(values);

    for(i=0; i < n; i++ ) {
        zframe_destroy(&frames[i]);
    }
    if( frames!=NULL) free(frames);
}

