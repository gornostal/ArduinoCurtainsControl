#!/bin/bash

cd `dirname $0`
cd ..

docker build -t curtains-ws-server .
