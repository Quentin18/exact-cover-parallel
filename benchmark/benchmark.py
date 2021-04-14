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


def make():
    """Creates the executables."""
    os.system('cd .. && make')


def clean():
    """Cleans the executables."""
    os.system('cd .. && make clean')


def runtime(command, show=False):
    """Runs the command and returns the runtime."""
    t_start = time()
    if show:
        r = subprocess.run(command)
    else:
        r = subprocess.run(command, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
    t_end = time()
    if r.returncode != 0:
        raise Exception(r.stderr.decode())
    return t_end - t_start


def benchmark(instance, csvfilename, p_list, g5k, show=False):
    """
    Launches the exact cover solutions on an instance
    and writes the result in a csv file.
    """
    # Get $OAR_NODE_FILE
    if g5k:
        oar_nodefile = os.environ['OAR_NODE_FILE']
        print('$OAR_NODE_FILE:', oar_nodefile)

    print('== Start benchmark ==')
    t_start_benchmark = time()

    with open(csvfilename, 'w') as csvfile:
        # CSV writer
        writer = csv.writer(csvfile)
        writer.writerow([
            'Number of processors',
            'Sequential',
            'Parallel: MPI Static',
            'Parallel: MPI Dynamic',
            'Parallel: MPI + OpenMP'
        ])

        # Sequential solution
        print('-- Sequential --')
        t = runtime(['../sequencial/exact_cover.out', '--in', instance])
        print('t =', t)

        # Parallel solutions
        print('-- Parallel --')
        for p in p_list:
            print('p =', p)

            # MPI Static
            if g5k:
                command = [
                    'mpirun', '-max-vm-size', str(p), '--map-by', 'ppr:1:node',
                    '--hostfile', oar_nodefile,
                    '../mpi/exact_cover_mpi_static.out', '--in', instance
                ]
            else:
                command = [
                    'mpirun', '-n', str(p),
                    '../mpi/exact_cover_mpi_static.out', '--in', instance
                ]
            t1 = runtime(command, show)
            print('\tt1 =', t1)

            # MPI Dynamic
            if g5k:
                command = [
                    'mpirun', '-max-vm-size', str(p), '--map-by', 'ppr:1:node',
                    '--hostfile', oar_nodefile,
                    '../mpi/exact_cover_mpi_dynamic.out', '--in', instance
                ]
            else:
                command = [
                    'mpirun', '-n', str(p),
                    '../mpi/exact_cover_mpi_dynamic.out', '--in', instance
                ]
            t2 = runtime(command, show)
            print('\tt2 =', t2)

            # MPI + OpenMP
            if g5k:
                command = [
                    'mpirun', '-x', 'OMP_NUM_THREADS=2', '-max-vm-size',
                    str(p), '--map-by', 'ppr:1:node', '--hostfile',
                    oar_nodefile, '../mpi/exact_cover_mpi_dynamic.out',
                    '--in', instance
                ]
            else:
                command = [
                    'mpirun', '-x', 'OMP_NUM_THREADS=2', '-n', str(p),
                    '../hybrid/exact_cover_hybrid.out', '--in', instance
                ]
            t3 = runtime(command, show)
            print('\tt3 =', t3)

            # Write results in csvfile
            writer.writerow(list(map(str, [p, t, t1, t2, t3])))

    t_end_benchmark = time()
    print('Benchmark done in', t_end_benchmark - t_start_benchmark, 'seconds')
    print('== End benchmark ==')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('USAGE: python3 benchmark.py [instance.ec]')
        exit()

    # Get instance
    instance = sys.argv[1]

    # CSV file
    csvfilename = os.path.basename(instance).replace('.ec', '.csv')

    # Creates executables
    make()

    # Launch benchmark
    benchmark(instance, csvfilename, [2, 4], False, False)

    # Clean executables
    clean()
