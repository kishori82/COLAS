#!/bin/bash
#~/COLAS/src/abdprocess --process-type 0
~/COLAS/src/abdprocessc --process-type 0  --ip  172.17.0.2 -ip 172.17.0.3 -ip 172.17.0.4  --filesize  1  --wait 100  --algorithm SODAW  --code rlnc --serverid $1
