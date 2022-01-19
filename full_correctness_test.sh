#!/bin/bash

rm -f allgather_algs_correctness.txt
./test_allgather_algs.sh >> allgather_algs_correctness.txt
wait


