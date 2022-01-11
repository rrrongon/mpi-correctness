#!/bin/bash
EXE=/home/sadeghil/mpi-correctness/allgather_correctness.exe

unset MV2_INTER_ALLGATHER_TUNING
unset SECURITY_APPROACH
unset SHMEM_LEADERS
unset CONCURRENT_COMM

echo "Testing Unencrypted Default"  
./test.sh "$EXE" 
echo "--------------------------------------"  



export SECURITY_APPROACH=1001
echo "Testing Naive Default"  
./test.sh "$EXE" 
echo "--------------------------------------"  

export SECURITY_APPROACH=2005
echo "Testing Opportunistic Default"  
./test.sh "$EXE" 
echo "--------------------------------------"  


export MV2_INTER_ALLGATHER_TUNING=12
unset SECURITY_APPROACH

echo "Testing C-Ring"  
./test.sh "$EXE" 
echo "--------------------------------------"  

export SECURITY_APPROACH=2005
echo "Testing Encrypted C-Ring"  
./test.sh "$EXE" 
echo "--------------------------------------"  

export MV2_INTER_ALLGATHER_TUNING=13
unset SECURITY_APPROACH

echo "Testing C-RD"  
./test.sh "$EXE" 
echo "--------------------------------------"  

export SECURITY_APPROACH=2005
echo "Testing Encrypted C-RD"  
./test.sh "$EXE" 
echo "--------------------------------------"  


export MV2_INTER_ALLGATHER_TUNING=18
export SECURITY_APPROACH=2006

echo "Testing HS2"  
./test.sh "$EXE" 
echo "--------------------------------------"  


export MV2_INTER_ALLGATHER_TUNING=17
export SECURITY_APPROACH=2005

echo "Testing O-RD2"  
./test.sh "$EXE" 
echo "--------------------------------------"  


export MV2_INTER_ALLGATHER_TUNING=14
unset SECURITY_APPROACH
echo "Testing Shared-Mem"  
./test.sh "$EXE" 
echo "--------------------------------------"  


export SECURITY_APPROACH=2006
export SHMEM_LEADERS=1

echo "Testing HS1"  
./test.sh "$EXE" 
echo "--------------------------------------"  




export MV2_INTER_ALLGATHER_TUNING=20
export CONCURRENT_COMM=1
unset SECURITY_APPROACH
echo "Testing CHS"  
./test.sh "$EXE" 
echo "--------------------------------------"  


export SECURITY_APPROACH=2006
echo "Testing Encrypted CHS"  
./test.sh "$EXE" 
echo "--------------------------------------"  
