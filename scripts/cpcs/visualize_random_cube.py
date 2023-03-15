import json

import compare
import subprocess
import sys
import argparse
import matplotlib.pyplot as plt

markers = ['^', 's', 'o', '+', 'x', '*']
colors = ['#377eb8', '#ff7f00', '#e41a1c', '#f781bf', '#a65628', '#4daf4a', '#984ea3', '#999999', '#dede00', '#377eb8']


def main(input_file="/home/sam/Dropbox/semester7/Bachelorarbeit/cube"):
    plt.rc('font', size=18)
    plt.figure(figsize=(16, 8))

    plt.ylabel("Run time t / s")
    plt.xlabel("# variables in random 3SAT")

    x = list(range(300, 500, 10))

    with open(input_file, "r") as f:
        results = json.load(f)

    for idx, (group_nogroup, finish_times) in enumerate(results.items()):
        y = finish_times
        print(x)
        print(y)
        plt.plot(x,y, marker=markers[idx % len(markers)], color=colors[idx % len(colors)], label=group_nogroup)

    plt.legend(loc="upper left")
    plt.show()


if __name__ == '__main__':
    main()
