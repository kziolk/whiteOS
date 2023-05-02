whiteOS

prerequisites:
- cross compiler: https://wiki.osdev.org/GCC_Cross-Compiler
- make

build:
- go to project root directory
- create folder ./env
- create file ./env/cross_compiler_path.txt which contains path to the cross compiler 
  - (e.g. run command: echo /path/to/my/gcc > ./env/cross_compiler_path.txt)
- run command: chmod +x ./run.sh
- run command: ./run.sh

