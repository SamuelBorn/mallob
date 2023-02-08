import argparse
import json
import os
import diversify_amend
import numpy as np


def main(num_vars, num_jobs, min_overlap, step_size, num_tests, num_cores):
    os.makedirs(f"scripts/cpcs/temp", exist_ok=True)
    results_group = {}
    results_nogroup = {}

    for overlap in np.arange(min_overlap, 1.0001, step_size):
        results_group[overlap] = []
        results_nogroup[overlap] = []

        for i in range(num_tests):
            instances = f"scripts/cpcs/temp/instances"
            with open(instances, "w") as f:
                f.writelines(line + "\n" for line in diversify_amend.main(num_vars, overlap, num_jobs))

            for identifier in ["nogroup", "group"]:
                print(f"{identifier} {i}/{num_tests}")
                output = os.popen(f'mpirun -np {num_cores} --bind-to core build/mallob -v=2 -c=1 -ajpc={num_jobs} -ljpc={2 * num_jobs} -J={num_jobs} \
                                    -job-desc-template={instances} \
                                    -job-template=scripts/cpcs/input/job-{identifier}.json \
                                    -client-template=templates/client-template.json').read()
                filtered = [float(line.split(" ")[4]) for line in output.splitlines() if "RESPONSE_TIME" in line]
                results_nogroup[overlap].append(sum(filtered))
                print(sum(filtered))

                with open("scripts/cpcs/output/{identifier}.json", "w") as f:
                    if identifier == "group":
                        f.write(json.dumps(results_group))
                    else:
                        f.write(json.dumps(results_nogroup))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", help="Num vars in problem", type=int, default=1000)
    parser.add_argument("-j", help="Num jobs to run", type=int, default=3)
    parser.add_argument("-m", help="Min overlap between problems", type=float, default=0.97)
    parser.add_argument("-s", help="Step size of overlap", type=float, default=0.01)
    parser.add_argument("-n", help="Tests to perform", type=int, default=10)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=16)
    args = parser.parse_args()
    main(args.v, args.j, args.m, args.s, args.n, args.c)
