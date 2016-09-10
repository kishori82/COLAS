#!/bin/bash

curl -L 172.17.0.1:8080/SetName/writer_0
curl -L 172.17.0.1:8080/SetServers/172.17.0.2_172.17.0.3_172.17.0.4

curl -L 172.17.0.2:8080/SetName/server_0
curl -L 172.17.0.2:8080/SetServers/172.17.0.2_172.17.0.3_172.17.0.4

curl -L 172.17.0.3:8080/SetName/server_1
curl -L 172.17.0.3:8080/SetServers/172.17.0.2_172.17.0.3_172.17.0.4

curl -L 172.17.0.4:8080/SetName/server_2
curl -L 172.17.0.4:8080/SetServers/172.17.0.2_172.17.0.3_172.17.0.4

curl -L 172.17.0.1:8080/StartProcess
curl -L 172.17.0.2:8080/StartProcess
curl -L 172.17.0.3:8080/StartProcess
curl -L 172.17.0.4:8080/StartProcess
