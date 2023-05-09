whiteOS

prerequisites:
    Docker
    qemu

create docker container:
    $ ./script/createContainer.sh

build:
    $ ./script/startContainer.sh
    $ cd /os
    $ ./script/build.sh
    $ exit

run:
    $ ./script/run.sh
