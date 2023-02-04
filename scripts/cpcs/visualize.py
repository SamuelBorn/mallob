import json
import os
import shutil
import random
import argparse
import numpy
import time
import matplotlib.pyplot as plt


def main(input_file_path):
    x = []
    y = []
    x_avg = []
    y_avg = []

    with open(input_file_path) as f:
        data = json.load(f)

    for overlap, value in data.items():
        acc = 0
        for exec_time in value:
            normalized_exec_time = float(exec_time) / float(overlap)
            x.append(float(overlap))
            y.append(normalized_exec_time)
            acc += normalized_exec_time
        x_avg.append(float(overlap))
        y_avg.append(normalized_exec_time)

    plt.scatter(x, y, s=5)
    plt.scatter(x_avg, y_avg)
    plt.title("Execution time with CPCS for problems that share x percent of clauses")
    plt.xlabel("Percentage of clauses kept")
    plt.ylabel("Normalized execution time (divide by num clauses)")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="INPUT File", required=True)
    args = parser.parse_args()
    main(args.i)
