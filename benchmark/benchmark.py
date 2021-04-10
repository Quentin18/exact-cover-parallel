"""
This program launches a benchmark for an instance of the exact cover problem.
It writes the results in a csv file.

You need to compile the files with "make" before using this program.

Usage:
python3 benchmark.py [instance.ec]

Author: Quentin Deschamps
Date: 2021
"""
import os
import sys
from time import time
import subprocess
import csv


def runtime(command, show=False):
    """Runs the command and returns the runtime."""
    t_start = time()
    if show:
        r = subprocess.run(command)
    else:
        r = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    t_end = time()
    if r.returncode != 0:
        raise Exception(r.stderr.decode())
    return t_end - t_start


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('USAGE: python3 benchmark.py [instance.ec]')
        exit()

    # # Get $OAR_NODE_FILE
    # oar_nodefile = os.environ['OAR_NODE_FILE']
    # print('$OAR_NODE_FILE:', oar_nodefile)

    # Get instance
    filename = sys.argv[1]

    # CSV file
    csvfilename = os.path.basename(filename).replace('.ec', '.csv')

    print('== Start benchmark ==')
    t_start_benchmark = time()

    with open(csvfilename, 'w') as csvfile:
        # CSV writer
        writer = csv.writer(csvfile)
        writer.writerow(['Number of processors', 'Sequential', 'Parallel with MPI'])

        # Sequential solution
        print('-- Sequential --')
        t = runtime(['../sequencial/exact_cover', '--in', filename])
        print('t =', t)

        # Parallel solutions
        print('-- Parallel --')
        for p in [2, 3]:
            print('p =', p)
            t1 = runtime(['mpiexec', '-n', str(p), '../mpi/exact_cover', '--in', filename])
            # t1 = runtime([
            #     'mpiexec', '-max-vm-size', str(p), '--map-by', 'ppr:1:node',
            #     '--hostfile', oar_nodefile, '../mpi/exact_cover', '--in', filename
            # ])
            print('\tt1 =', t1)
            writer.writerow([str(p), str(t), str(t1)])

    t_end_benchmark = time()
    print('Benchmark done in', t_end_benchmark - t_start_benchmark, 'seconds')
    print('== End benchmark ==')
