#!/bin/bash
#SBATCH -p work
#SBATCH --time=72:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --job-name="reimport_run_post"

cd /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/seims/preprocess
python main.py -ini /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/data/youwuzhen/workspace/preprocess.ini
echo "preprocess finished!"

cd /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/seims
python run_seims.py -ini /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/data/youwuzhen/workspace/runmodel.ini
echo "run model finished！"

cd /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/seims/postprocess
python main.py -ini /GPUFS/igsnrr_czqin_2/shenshen/SEIMS/data/youwuzhen/workspace/postprocess.ini
echo "postprocess finished!"
