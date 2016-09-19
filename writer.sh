#!/bin/bash
#~/COLAS/src/abdprocess --process-type 1
#!/bin/bash
#~/COLAS/src/abdprocessc --process-type 1  --ip  172.17.0.2 --ip 172.17.0.3 --ip 172.17.0.4  --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
#~/COLAS/src/abdprocessc --process-type 1  --ip  172.17.0.2 --ip 172.17.0.3  --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
~/COLAS/src/abdprocessc --process-type 1  --ip  172.17.0.2 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
# valgrind --leak-check=full --tool=memcheck --show-leak-kinds=all  ~/COLAS/src/abdprocessc --process-type 1  --ip  172.17.0.2 --filesize  4  --wait 100  --algorithm SODAW  --code reed_solomon 
#--serverid $1
