"""
This program creates graphs to show the results of a benchmark.
It uses matplotlib.
It saves the figures in a png file.

Usage:
python3 csvgraph.py [filename.csv]

Author: Quentin Deschamps
Date: 2021
"""
import os
import sys
import csv
import matplotlib.pyplot as plt


def speedup(t1, t):
    """Returns the list of speedup."""
    return [t1 / tp for tp in t]


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('USAGE: python3 csvgraph.py [filename.csv]')
        exit()

    csvfilename = sys.argv[1]
    name = os.path.splitext(os.path.basename(csvfilename))[0]
    with open(csvfilename, 'r', newline='') as csvfile:
        # CSV reader
        reader = csv.reader(csvfile)

        # Read CSV
        x, y_list = [], []
        for i, row in enumerate(reader):
            if i == 0:
                xlabel, labels = row[0], row[1:]
                for _ in range(len(labels)):
                    y_list.append([])
            else:
                x.append(int(row[0]))
                for j, t in enumerate(row[1:]):
                    y_list[j].append(float(t))

    # Plot data
    fig, (ax1, ax2) = plt.subplots(2, figsize=(10, 10))
    fmts = ['--', '-o', '-s', '-p', '-^', '-v'][:len(y_list)]

    # Runtime
    for y, fmt, label in zip(y_list, fmts, labels):
        ax1.plot(x, y, fmt, label=label)
    ax1.set_xlabel(xlabel)
    ax1.set_ylabel('Time (seconds)')
    ax1.set_title(f'Exact Cover: {name} runtime')
    ax1.legend()
    ax1.grid(linestyle='--')

    # Speedup
    t1 = y_list[0][0]
    if 'nodes' not in xlabel:
        ax2.plot(x, x, fmts[0], label='Optimal')
    for y, fmt, label in zip(y_list[1:], fmts[1:], labels[1:]):
        ax2.plot(x, speedup(t1, y), fmt, label=label)
    ax2.set_xlabel(xlabel)
    ax2.set_ylabel('Speedup')
    ax2.set_title(f'Exact Cover: {name} speedup')
    ax2.legend()
    ax2.grid(linestyle='--')

    # Save figure
    plt.savefig(os.path.join('png', name))

    # Show
    plt.show()
