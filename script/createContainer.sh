#!/bin/bash

docker build -t osenv .
docker run --name whiteos_builder -v .:/os -it osenv 
