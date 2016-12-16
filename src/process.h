#ifndef __ABDPROCESSC__
#define __ABDPROCESSC__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "utilities/algo_utils.h"
#include "baseprocess/client.h"
#include "codes/rlnc_rs.h"

#include "sodaw/sodaw_client.h"
#include "sodaw/sodaw_reader.h"
#include "sodaw/sodaw_writer.h"
#include "sodaw/sodaw_server.h"

#include "soda/soda_reader.h"

#include "baseprocess/server.h"
#include "abd/abd_client.h"
#include "abd/abd_writer.h"
#include "abd/abd_reader.h"
#include "utilities/helpers.h"

unsigned int readParameters(int argc, char *argv[], Parameters *parameters);
void reader_process(Parameters parameters) ;
void writer_process(Parameters parameters) ;
void write_initial_data(Parameters parameters);
#endif
