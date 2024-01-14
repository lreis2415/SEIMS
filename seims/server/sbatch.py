import time

from pygeoc.utils import UtilClass


def run(
    partition,
    nodes,
    ntasks,
    tasks_per_node,
    cali_ini,
    python_script_path,
    job_name='cali',
    cpus_per_task=1,
):
    sbatch = f"""#!/bin/bash
    #SBATCH -p {partition}
    #SBATCH --nodes={nodes}
    #SBATCH --ntasks={ntasks}
    #SBATCH --job-name={job_name}
    #SBATCH --cpus-per-task={cpus_per_task}
    #SBATCH --ntasks-per-node={tasks_per_node}
    #SBATCH --exclusive
    N_NODES={nodes}
    N_TASKS={ntasks}

    python -m scoop -n {ntasks} {python_script_path} -ini {cali_ini}
    """
    batch_file_name = 'cali.batch'
    with open(batch_file_name, 'w') as f:
        f.write(sbatch)
    runlogs = UtilClass.run_command(['sbatch', batch_file_name])
    # e.g., Submitted batch job 2691879
    job_id = runlogs[0].split()[-1]
    sacct_query = f'sacct -j {job_id} -n -o state,exitcode'
    # e.g., PENDING      0:0
    state = None
    while state != 'COMPLETED':
        time.sleep(10)
        runlogs = UtilClass.run_command(sacct_query.split(' '))
        state, exitcode = runlogs[0].split()
    return
