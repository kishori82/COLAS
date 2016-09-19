#!/bin/bash
#~/COLAS/src/abdprocess --process-type 2
#~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.2 --ip 172.17.0.3 --ip 172.17.0.4  --filesize  50  --wait 100  --algorithm SODAW  --code reed_solomon 
#--serverid $1
#~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
valgrind --leak-check=full --tool=memcheck --show-leak-kinds=all  --suppressions=vg.supp   ~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
#--serverid $1
