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
    visualize_filtered()


def visualize_filtered():
    plt.rc('font', size=18)
    plt.figure(figsize=(8, 14))
    plt.ylabel("Execution time (seconds)")
    plt.xlabel("Absolute number of deleted clauses")

    for file_idx, file in enumerate(sys.argv[1:]):
        with open(file) as f:
            data = json.load(f)

        if file_idx == 0:
            plt.boxplot(data.values(), labels=[key for key in data.keys()], medianprops=dict(linewidth=2, color='green'), boxprops=dict(color='blue'))
        else:
            plt.boxplot(data.values(), labels=[key for key in data.keys()], medianprops=dict(linewidth=3, color='orange'), boxprops=dict(color='red'))
    plt.show()


def visualize_unfiltered():
    plt.rc('font', size=18)
    plt.figure(figsize=(7.2, 14))
    plt.ylabel("Execution time (seconds)")

    for file_idx, file in enumerate(sys.argv[1:]):
        with open(file) as f:
            data = json.load(f)

        if file_idx == 0:
            plt.boxplot([[sum(l) for l in lout] for lout in data.values()], labels=[key for key in data.keys()], medianprops=dict(linewidth=3, color='green'), boxprops=dict(color='blue'))
        else:
            plt.boxplot(sum(data.values()), labels=[key for key in data.keys()], medianprops=dict(linewidth=3, color='orange'), boxprops=dict(color='red'))
    plt.show()


if __name__ == '__main__':
    main()
