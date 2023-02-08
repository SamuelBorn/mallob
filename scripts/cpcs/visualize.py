import json
import os
import shutil
import random
import argparse
import numpy
import time
import matplotlib.pyplot as plt


def main(input_file_path):
    with open(input_file_path) as f:
        data = json.load(f)

    plt.rc('font', size=18)
    plt.figure(figsize=(8, 14))

    plt.boxplot(data.values(), labels=data.keys())
    plt.ylabel("Execution time (seconds)")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="INPUT File", required=True)
    args = parser.parse_args()
    main(args.i)
