#!/bin/bash

mkdir -p /mnt/d
sudo mount -t vfat ./bin/whiteos.bin /mnt/d
sudo cp -r ./fat16_root_dir/* /mnt/d/
sudo umount /mnt/d

qemu-system-i386 -hda ./bin/whiteos.bin
