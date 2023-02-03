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
    delta = 0.0005
    last_x = 1
    avg = 0
    num_elements = 0
    with open(input_file_path, "r") as f:
        for line in f.readlines():
            x_elem_str, y_elem_str = line.split(" ")
            x_elem = pow(float(x_elem_str), 3)
            y_elem = float(y_elem_str) / pow(float(x_elem_str), 1)
            if abs(x_elem - last_x) < delta:
                avg += y_elem
                num_elements += 1
            else:
                x_avg.append(last_x)
                y_avg.append(avg / num_elements)
                num_elements = 1
                avg = y_elem
                last_x = x_elem

            x.append(x_elem)
            y.append(y_elem)
        x_avg.append(last_x)
        y_avg.append(avg / num_elements)

    plt.scatter(x, y, s=3)
    plt.scatter(x_avg, y_avg, c="red")
    plt.title("Execution time with CPCS for problems that share x percent of clauses")
    plt.xlabel("Percentage of clauses kept")
    plt.ylabel("Normalized execution time (divide by num clauses)")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="INPUT File", required=True)
    args = parser.parse_args()
    main(args.i)
