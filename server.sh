#!/bin/bash
#~/COLAS/src/abdprocess --process-type 2
#~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.2 --ip 172.17.0.3 --ip 172.17.0.4  --filesize  50  --wait 100  --algorithm SODAW  --code reed_solomon 
#--serverid $1
#~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.2 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 


#~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.2 --ip  172.17.0.3 --ip  172.17.0.4 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 







# DAVID
#valgrind --leak-check=full --tool=memcheck --suppressions=vg.supp   ~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --filesize  4  --wait 100  --algorithm ABD  --code reed_solomon 
#valgrind --tool=massif ~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --filesize  4  --wait 100  --algorithm ABD  --code reed_solomon 
#valgrind --tool=callgrind ~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --filesize  4  --wait 100  --algorithm SODAW --code reed_solomon 

#gdb -ex run  --args ~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --ip  172.17.0.3 --ip  172.17.0.4 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
 ~/COLAS/src/abdprocessc --process-type 2  --ip  172.17.0.1 --ip  172.17.0.3 --ip  172.17.0.4 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
