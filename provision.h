curl -L 172.17.0.1:8080/SetName/writer_1
curl -L 172.17.0.1:8080/SetServers/172.17.0.2
curl -L 172.17.0.2:8080/SetName/server_1
curl -L 172.17.0.1:8080/Start
curl -L 172.17.0.2:8080/Start
