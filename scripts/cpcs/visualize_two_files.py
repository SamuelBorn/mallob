import json
import os
import shutil
import random
import argparse
import numpy
import time
import matplotlib.pyplot as plt


def main(input_file1_path, input_file2_path):
    with open(input_file1_path, "r") as f:
        y = [float(line) for line in f.readlines()]
        x = ["File 1"] * len(y)
        x_avg = ["File 1"]
        y_avg = [sum(y) / len(y)]

    with open(input_file2_path, "r") as f:
        lines = f.readlines()
        y.extend([float(line) for line in lines])
        x.extend(["File 2"] * len(lines))
        x_avg.append("File 2")
        y_avg.append(sum(y) / len(y))

    print(y_avg[0])
    print(y_avg[1])

    plt.scatter(x, y, s=5)
    plt.scatter(x_avg, y_avg)
    plt.title("Execution time with CPCS for problems that share x percent of clauses")
    plt.xlabel("Percentage of clauses kept")
    plt.ylabel("Normalized execution time (divide by num clauses)")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i1", help="INPUT File 1", required=True)
    parser.add_argument("-i2", help="INPUT File 2", required=True)
    args = parser.parse_args()
    main(args.i1, args.i2)
