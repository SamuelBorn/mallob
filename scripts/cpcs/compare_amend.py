import argparse
import datetime
import subprocess
import json
import os
import time
import datetime

import diversify_amend
import numpy as np
import compare


def main(num_vars, num_jobs, min_overlap, step_size, num_tests, num_cores, timeout):
    os.makedirs(f"scripts/cpcs/temp", exist_ok=True)
    results_group = {}
    results_nogroup = {}

    for overlap in np.arange(min_overlap, 1.0001, step_size):
        results_group[overlap] = []
        results_nogroup[overlap] = []

        for i in range(num_tests):
            instance_file = f"scripts/cpcs/temp/instance_file"
            with open(instance_file, "w") as f:
                f.writelines(line + "\n" for line in diversify_amend.main(num_vars, overlap, num_jobs))

            for identifier in ["nogroup", "group-check"]:
                print(f"{datetime.datetime.now()} - {overlap} - {i:02d}/{num_tests} - {identifier}")
                time_acc = compare.run_once(timeout, instance_file, num_jobs, num_cores, identifier)
                print(time_acc)

                with open(f"scripts/cpcs/output/{identifier}.json", "w") as f:
                    if identifier == "group-check":
                        results_group[overlap].append(time_acc)
                        f.write(json.dumps(results_group))
                    else:
                        results_nogroup[overlap].append(time_acc)
                        f.write(json.dumps(results_nogroup))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", help="Num vars in problem", type=int, default=1000)
    parser.add_argument("-j", help="Num jobs to run", type=int, default=3)
    parser.add_argument("-m", help="Min overlap between problems", type=float, default=0.97)
    parser.add_argument("-s", help="Step size of overlap", type=float, default=0.01)
    parser.add_argument("-n", help="Tests to perform", type=int, default=10)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=4)
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=180)
    args = parser.parse_args()
    main(args.v, args.j, args.m, args.s, args.n, args.c, args.t)
