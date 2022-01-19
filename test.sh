#!/bin/bash

echo "7-91 block"
/home/sadeghil/Installed_Tools/SMVAPICH/bin/mpirun -n 91 -hostfile hostfile-7-91 $1
wait
echo "7-91 cyclic"
/home/sadeghil/Installed_Tools/SMVAPICH/bin/mpirun -n 91 -hostfile cyclic-hostfile-7 $1
wait
echo "8-128 block"
/home/sadeghil/Installed_Tools/SMVAPICH/bin/mpirun -n 128 -hostfile hostfile-8-128 $1
wait
echo "8-128 cyclic"
/home/sadeghil/Installed_Tools/SMVAPICH/bin/mpirun -n 128 -hostfile cyclic-hostfile-8 $1
wait

