curl -L 172.17.0.1:8080/SetName/reader_1
curl -L 172.17.0.1:8080/SetServers/172.17.0.2
curl -L 172.17.0.2:8080/SetServers/172.17.0.2
curl -L 172.17.0.2:8080/SetName/server_1
curl -L 172.17.0.1:8080/StartProcess
curl -L 172.17.0.2:8080/StartProcess
