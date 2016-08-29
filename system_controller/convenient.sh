docker-compose down
docker commit hopeful_goldberg kishori82/dev:COLAS2
docker-compose scale reader=1 writer=2 server=3 controller=1
./system_management setup
