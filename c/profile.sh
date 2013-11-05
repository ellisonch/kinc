set -e
gcc -O2 -g -std=c11 imp.c -o imp
time operf ./imp
opannotate --source > anno.c
opreport --symbols > report.txt