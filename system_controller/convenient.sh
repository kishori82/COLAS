docker-compose down
docker commit  agitated_shaw kishori82/dev:COLAS2
docker-compose scale reader=3 writer=3 server=5 controller=1
./system_management setup
./system_management setfile_size 1000
./system_management setread_dist const 200
./system_management setwrite_dist const 200
./system_management start
