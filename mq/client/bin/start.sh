#!/bin/bash

cd `dirname $0`
cd ..

./bin/env.sh
cmd="API_URL=$API_URL ACCESS_TOKEN=$ACCESS_TOKEN ARDUINO_URL=$ARDUINO_URL node client.js"

nohup bash -c "($cmd > client.out) &> client_err.out" &
