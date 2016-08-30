docker-compose down
docker commit dreamy_einstein kishori82/dev:COLAS2
docker-compose scale reader=2 writer=0 server=3 controller=1
./system_management setup
./system_management setfile_size 10
./system_management setread_dist const 51
./system_management setwrite_dist const 51
