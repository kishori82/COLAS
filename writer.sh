#!/bin/bash
#~/COLAS/src/abdprocess --process-type 1
#!/bin/bash
~/COLAS/src/abdprocessc --process-type 1  --ip  172.17.0.2 --ip 172.17.0.3 --ip 172.17.0.4  --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
#--serverid $1
