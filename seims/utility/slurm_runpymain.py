from slurmpy import Slurm

s = Slurm('test_mpi', {'W': '', 'partition': 'work', 'N': 2, 'n': 8, 'c': 2}, bash_strict=False)
jobid = s.run('echo $SLURM_JOB_NODELIST\n'
              'NODES_PYMAIN=$(<nodes_pymain)\n'
              'ALL_NODES=$NODES_PYMAIN\',\'$SLURM_JOB_NODELIST\n'
              'for i in {1..5}; do\n'
              '  srun --nodelist=$ALL_NODES -n 4 /GPUFS/igsnrr_czqin_2/test/test_mpi &\n'
              'done\n'
              'wait',
              name_addition='th2'
              )

print('All Slurm jobs done!')
