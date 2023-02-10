import json
import os
import shutil
import random
import argparse
import numpy
import time
import matplotlib.pyplot as plt
import sys


def main():
    plt.rc('font', size=18)
    plt.figure(figsize=(8, 14))
    plt.ylabel("Execution time (seconds)")

    for fileidx, file in enumerate(sys.argv[1:]):
        with open(file) as f:
            data = json.load(f)

        if fileidx == 0:
            plt.boxplot(data.values(), labels=[round(float(key), 3) for key in data.keys()], medianprops=dict(linewidth=3, color='green'), boxprops=dict(color='blue'))
        else:
            plt.boxplot(data.values(), labels=[round(float(key), 3) for key in data.keys()], medianprops=dict(linewidth=3, color='orange'), boxprops=dict(color='red'))
    plt.show()


if __name__ == '__main__':
    main()
