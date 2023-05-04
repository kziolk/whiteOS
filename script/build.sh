#!/bin/bash

make clean
make all
chown newuser:newuser -R /os
