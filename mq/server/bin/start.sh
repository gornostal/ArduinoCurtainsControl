#!/bin/bash

cd `dirname $0`
cd ..

docker run \
  -d \
  --rm \
  --env-file .env \
  -p 8080:8080\
  --name curtains-ws-server \
  curtains-ws-server
