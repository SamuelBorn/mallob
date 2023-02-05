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

    for key, value in data.items():
        keys = [float(key)] * len(value)
        times = [float(t) for t in value]
        plt.scatter(keys, times, s=5, c='blue')
        plt.scatter(float(key), sum(times) / len(times), c='red')

    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="INPUT File", required=True)
    args = parser.parse_args()
    main(args.i)
