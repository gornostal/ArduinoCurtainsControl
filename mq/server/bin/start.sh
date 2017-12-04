#!/bin/bash

cd `dirname $0`
cd ..

docker stop curtains-ws-server

docker run \
  -d \
  --rm \
  --env-file .env \
  --log-driver awslogs \
  --log-opt awslogs-region=eu-west-1 \
  --log-opt awslogs-group=home-automation/curtains \
  --log-opt awslogs-stream=server \
  --log-opt awslogs-datetime-format="%Y-%m-%dT%H:%M:%S.%fZ" \
  -p 8080:8080\
  --name curtains-ws-server \
  curtains-ws-server
