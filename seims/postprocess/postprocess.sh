#!/bin/bash
#SBATCH -p work
#SBATCH --time=72:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --job-name="postprocess"

python main.py -ini /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/data/youwuzhen/workspace/postprocess.ini

