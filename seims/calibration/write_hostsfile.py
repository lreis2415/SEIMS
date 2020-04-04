#!/GPUFS/app_GPU/application/anaconda3/5.3.1/envs/python-3.6/bin/python
import os
import argparse
import subprocess


def main(n_nodes, n_tasks):
    n_workers_per_node = n_tasks / n_nodes
    cmd = 'srun -N {0} -n {1} -p work hostname'.format(n_nodes, n_nodes)
    args = cmd.split(' ')
    # print(args)
    outputs = subprocess.check_output(args)
    lines = outputs.strip().split(os.linesep)
    print(lines)  # ['cpn206', 'cpn213', 'cpn216', 'cpn223']
    print(len(lines))

    with open('hostname.txt', 'w') as fp:
        for line in lines:
            line += ' {0}{1}'.format(n_workers_per_node, os.linesep)
            fp.write(line)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-n', type=int, help='the number of tasks')
    parser.add_argument('-N', type=int, help='the number of nodes')
    args = parser.parse_args()
    print('{0} nodes, {1} tasks'.format(args.N, args.n))
    main(args.N, args.n)
