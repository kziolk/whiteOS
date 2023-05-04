#!/bin/bash

CONTAINER_NAME=$(cat script/container_name.txt)

docker container start $CONTAINER_NAME -i
