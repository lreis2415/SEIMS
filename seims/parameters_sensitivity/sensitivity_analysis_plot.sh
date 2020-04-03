#!/bin/bash
#SBATCH -p work
#SBATCH --time=72:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --job-name="sa_plot"

#N_NODES=8
#N_TASKS=48
#python write_hostsfile.py -N $N_NODES  -n $N_TASKS

# check if we were able to start writing the conf file
#SCOOPHOST_FILE="hostname.txt"
#if [ -f $SCOOPHOST_FILE ]; then
#    echo "file created..."
#else
#    echo "$0: can't create $SCOOPHOST_FILE"
#    exit 1
#fi

python main.py -ini /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/data/youwuzhen/workspace/sensitivity_analysis.ini

