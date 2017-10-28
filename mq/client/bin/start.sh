#!/bin/bash

cd `dirname $0`
cd ..

cmd="API_URL=$1 ACCESS_TOKEN=$2 ARDUINO_URL=$3 node client.js"

nohup bash -c "($cmd > client.out) &> client_err.out" &
