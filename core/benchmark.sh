#!/bin/bash

export OMP_NUM_THREADS=1
echo $OMP_NUM_THREADS
make clean && make

'''
# Dataset: amazon
echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: AMAZON>~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
echo "<<<<<<<<<ALGO: FAULTY-RANK-PUSH>>>>>>>>>"
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 403394 -E 3387388 -f /home/aislam6/dataset-faultyrank/amazon0601.el
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 403394 -E 4886816 -f /home/aislam6/dataset-faultyrank/amazon0601-un.el

# Dataset: skitter
echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: SKITTER>~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
echo "<<<<<<<<<ALGO: FAULTY-RANK-PUSH>>>>>>>>>"
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 1696415 -E 11095298 -f /home/aislam6/dataset-faultyrank/as-skitter.el
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 1696415 -E 22190596 -f /home/aislam6/dataset-faultyrank/as-skitter-un.el

# Dataset: road
echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: ROAD>~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
echo "<<<<<<<<<ALGO: FAULTY-RANK-PUSH>>>>>>>>>"
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 1971281 -E 5533214 -f /home/aislam6/dataset-faultyrank/roadNet-CA.el

# Dataset: livejournal
echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: LIVEJOURNAL>~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
echo "<<<<<<<<<ALGO: FAULTY-RANK-PUSH>>>>>>>>>"
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 4847571 -E 68993773 -f /home/aislam6/dataset-faultyrank/soc-LiveJournal1.el
taskset --cpu-list 0-70:2 ./faulty_rank_push -N 4847571 -E 85702474 -f /home/aislam6/dataset-faultyrank/soc-LiveJournal1-un.el

# Dataset: generated-random
'''
echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: RANDOM>~~~~~~~~~~~~~~~~~~~~~~~~~"
echo
echo "<<<<<<<<<ALGO: FAULTY-RANK-PUSH>>>>>>>>>"
taskset --cpu-list 0-70:2 ./faultyrank -N 578072  -E 595666 -f /path_to_final_graph/final_graph.txt -u /path_to_final_unfilled/final_unfilled.txt