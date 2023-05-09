whiteOS

prerequisites:
- Docker
- qemu

create docker container (linux):
- $ ./script/createContainer.sh

build:
- $ ./script/startContainer.sh
- $ cd /os
- $ ./script/build.sh
- $ exit

run (linux):
- $ ./script/run.sh
