"""
This program launches a benchmark for an instance of the exact cover problem.
It writes the results in a csv file.

It loads the config of the benchmark from a json file.

Params:
- instance:     path to the instance file
- csvfile:      path to the csv file (output)
- xlabel:       "Number of processors" or "Number of threads" in general
- xlist:        list of values for the number of processors/threads to test
- sequential:   true to test the sequential program
- openmp:       true to test the program with OpenMP
- mpi_static:   true to test the program with MPI (static)
- mpi_dynamic:  true to test the program with MPI (dynamic)
- hybrid:       true to test the program with MPI + OpenMP
- num_threads:  number of threads for the hybrid version (default: 2)
- make:         true to run "make" command (default: true)
- clean:        true to run "make clean" command (default: true)
- show:         true to show the output of the executions (default: true)
- g5k           true if you run the benchmark on g5k (default: false)

Usage:
python3 benchmark.py [file.json]

Author: Quentin Deschamps
Date: 2021
"""
import os
import sys
from time import time
import subprocess
import csv
import json


def make():
    """Creates the executables."""
    os.system('cd .. && make')


def clean():
    """Cleans the executables."""
    os.system('cd .. && make clean')


def runtime(command: list, show=True, env=None):
    """Runs the command and returns the runtime."""
    print('START:', *command)
    t_start = time()
    if show:
        r = subprocess.run(command, env=env)
    else:
        r = subprocess.run(command, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE, env=env)
    t_end = time()
    if r.returncode != 0:
        raise Exception(r.stderr.decode())
    t = t_end - t_start
    print('END: runtime =', round(t, 3), 'seconds')
    return t


def benchmark(config: dict):
    """
    Launches the exact cover solutions on an instance
    and writes the result in a csv file.
    """
    # Get instance
    instance = config['instance']
    print('Instance:', instance)

    # CSV file
    csvfilename = config['csvfile']
    print('CSV file:', csvfilename)

    g5k = config['g5k'] if 'g5k' in config else False
    show = config['show'] if 'show' in config else True
    xlist = config['xlist']
    num_threads = config['num_threads'] if 'num_threads' in config else 2

    # Get $OAR_NODE_FILE
    if g5k:
        oar_nodefile = os.environ['OAR_NODE_FILE']
        print('$OAR_NODE_FILE:', oar_nodefile)

    programs = {
        'sequential': 'Sequential',
        'openmp': 'Parallel: OpenMP',
        'mpi_static': 'Parallel: MPI Static',
        'mpi_dynamic': 'Parallel: MPI Dynamic',
        'hybrid': 'Parallel: MPI + OpenMP'
    }

    print('== Start benchmark ==')
    t_start_benchmark = time()

    with open(csvfilename, 'w') as csvfile:
        # CSV writer
        writer = csv.writer(csvfile)

        # Write headers
        headers = [config['xlabel']]
        for prog, name in programs.items():
            if prog in config and config[prog]:
                headers.append(name)
        writer.writerow(headers)

        # Sequential solution
        if config['sequential']:
            print('-- Sequential --')
            command = ['../sequencial/exact_cover.out', '--in', instance]
            t_sequential = runtime(command, show)

        # Parallel solutions
        print('-- Parallel --')
        for x in xlist:
            print('x =', x)
            row = [str(x)]

            # Sequential
            if 'sequential' in config and config['sequential']:
                row.append(str(t_sequential))

            # OpenMP
            if 'openmp' in config and config['openmp']:
                env = dict(os.environ, OMP_NUM_THREADS=str(x))
                command = ['../openmp/exact_cover_omp.out', '--in', instance]
                row.append(str(runtime(command, show, env=env)))

            # MPI Static
            if 'mpi_static' in config and config['mpi_static']:
                if g5k:
                    command = [
                        'mpirun', '-max-vm-size', str(x), '--map-by',
                        'ppr:1:node', '--hostfile', oar_nodefile,
                        '../mpi/exact_cover_mpi_static.out', '--in', instance
                    ]
                else:
                    command = [
                        'mpirun', '-n', str(x),
                        '../mpi/exact_cover_mpi_static.out', '--in', instance
                    ]
                row.append(str(runtime(command, show)))

            # MPI Dynamic
            if 'mpi_dynamic' in config and config['mpi_dynamic']:
                if g5k:
                    command = [
                        'mpirun', '-max-vm-size', str(x), '--map-by',
                        'ppr:1:node', '--hostfile', oar_nodefile,
                        '../mpi/exact_cover_mpi_dynamic.out', '--in', instance
                    ]
                else:
                    command = [
                        'mpirun', '-n', str(x),
                        '../mpi/exact_cover_mpi_dynamic.out', '--in', instance
                    ]
                row.append(str(runtime(command, show)))

            # MPI + OpenMP
            if 'hybrid' in config and config['hybrid']:
                if g5k:
                    command = [
                        'mpirun', '-x', 'OMP_NUM_THREADS=' + num_threads,
                        '-max-vm-size', str(x), '--map-by', 'ppr:1:node',
                        '--hostfile', oar_nodefile,
                        '../hybrid/exact_cover_hybrid.out', '--in', instance
                    ]
                else:
                    command = [
                        'mpirun', '-x', 'OMP_NUM_THREADS=2', '-n', str(x),
                        '../hybrid/exact_cover_hybrid.out', '--in', instance
                    ]
                row.append(str(runtime(command, show)))

            # Write results in csvfile
            writer.writerow(row)

    t_end_benchmark = time()
    print('Benchmark done in', t_end_benchmark - t_start_benchmark, 'seconds')
    print('== End benchmark ==')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('USAGE: python3 benchmark.py [file.json]')
        exit()

    # Get json
    jsonfile = sys.argv[1]

    # Read json
    with open(jsonfile, 'r') as f:
        config = json.loads(f.read())

    print(config)

    # Creates executables
    if 'make' not in config or config['make']:
        make()

    # Launch benchmark
    benchmark(config)

    # Clean executables
    if 'clean' not in config or config['clean']:
        clean()
