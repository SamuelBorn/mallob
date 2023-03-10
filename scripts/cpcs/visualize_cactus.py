import json

import compare
import subprocess
import sys
import argparse
import matplotlib.pyplot as plt

markers = ['^', 's', 'o', '+', 'x', '*']
colors = ['#377eb8', '#ff7f00', '#e41a1c', '#f781bf', '#a65628', '#4daf4a', '#984ea3', '#999999', '#dede00', '#377eb8']


def visualize_cactus(input_file):
    plt.rc('font', size=18)
    plt.figure(figsize=(8, 14))
    plt.ylabel("XX")
    plt.xlabel("XX")

    with open(input_file, "r") as f:
        results = json.load(f)

    for idx, (group_nogroup, finish_times) in enumerate(results.items()):
        x, y = generate_cactus_data(finish_times)
        plt.plot(x, y, marker=markers[idx % len(markers)], color=colors[idx % len(colors)])

    plt.show()


def generate_cactus_data(finish_times):
    cactus_x = [0]
    cactus_y = [0]
    for idx, time in enumerate(finish_times):
        cactus_x.append(time)
        cactus_y.append(idx + 1)
    return cactus_x, cactus_y


if __name__ == '__main__':
    visualize_cactus(sys.argv[1])
