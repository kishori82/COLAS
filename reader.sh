#!/bin/bash
#~/COLAS/src/abdprocess --process-type 0
gdb -ex run --args ~/COLAS/src/abdprocessc --process-type 0  --ip  172.17.0.1 --ip 172.17.0.2 --ip 172.17.0.3  --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
#--serverid $1
