import argparse
import json
import os
import time
import diversify_amend
import numpy as np


def main(num_vars, num_jobs, min_overlap, step_size, num_tests):
    os.makedirs(f"scripts/cpcs/temp", exist_ok=True)
    num_cores = 4

    results_group = {}
    results_nogroup = {}

    for o in np.arange(min_overlap, 1, step_size):
        results_group[o] = []
        results_nogroup[o] = []
        for i in range(num_tests):
            instances = f"scripts/cpcs/temp/instances"
            with open(instances, "w") as f:
                f.writelines(diversify_amend.main(num_vars, o, num_jobs))

            print(f"Nogroup {i}/{num_tests}")
            start = time.time()
            os.system(f'mpirun -np {num_cores} --bind-to core build/mallob -v=1 -c=1 -ajpc={num_jobs} -ljpc={2 * num_jobs} -J={num_jobs} \
                        -job-desc-template={instances} \
                        -job-template=scripts/cpcs/input/job-nogroup.json \
                        -client-template=templates/client-template.json -pls=0 \
                        && pkill mallob')
            end = time.time()
            print(end - start)
            results_group[o].append(end - start)

            print(f"Group {i}/{num_tests}")
            start = time.time()
            os.system(f'mpirun -np {num_cores} --bind-to core build/mallob -v=1 -c=1 -ajpc={num_jobs} -ljpc={2 * num_jobs} -J={num_jobs} \
                        -job-desc-template={instances} \
                        -job-template=scripts/cpcs/input/job-group-check.json \
                        -client-template=templates/client-template.json -pls=0 \
                        && pkill mallob')
            end = time.time()
            print(end - start)
            results_group[o].append(end - start)

            with open("scripts/cpcs/output/group.json", "w") as f:
                f.write(json.dumps(results_group))
            with open("scripts/cpcs/output/nogroup.json", "w") as f:
                f.write(json.dumps(results_nogroup))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", help="Num vars in problem", type=int, default=1700)
    parser.add_argument("-j", help="Num jobs to run", type=int, default=3)
    parser.add_argument("-m", help="Min overlap between problems", type=float, default=0.9)
    parser.add_argument("-s", help="Step size of overlap", type=float, default=0.01)
    parser.add_argument("-n", help="Tests to perform", type=int, default=10)
    args = parser.parse_args()
    main(args.v, args.j, args.m, args.s, args.n)
