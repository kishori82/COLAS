docker-compose down
docker commit  tiny_snyder kishori82/dev:COLAS3
docker-compose scale reader=2 writer=2 server=3 controller=1

./system_management setup
./system_management setfile_size 1000
./system_management setread_dist const 200
./system_management setwrite_dist const 200
./system_management start
