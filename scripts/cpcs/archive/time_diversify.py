import json
import os
import shutil
import random
import argparse
import numpy
import time
import diversify_delete


def main(input_file_path, output_file_path, step_size, n, max_clause_delete):
    results = {}

    with open(input_file_path, "r") as f:
        num_clauses = len(f.readlines()) - 2

    for i in range(0, max_clause_delete, step_size):
        results[i] = time_overlap(input_file_path, i, n)
        with open(output_file_path, "w") as f:
            f.write(json.dumps(results))


def time_overlap(input_file_path, clauses_to_delete, n):
    num_instances = 2
    instance_file = "scripts/cpcs/output/timing_instance_file"
    with open(instance_file, "w") as f:
        for div in delete_diversify.main(input_file_path, clauses_to_delete, num_instances):
            f.write(div + "\n")

    times = []
    for i in range(n):
        print(f"Deleted {clauses_to_delete} - {i}/{n}: ")
        start = time.time()
        os.system(f'mpirun -np 4 --bind-to core build/mallob -v=1 -c=1 -ajpc={num_instances} -ljpc={2 * num_instances} -J={num_instances} \
                    -job-desc-template={instance_file} \
                    -job-template=scripts/cpcs/input/job-template.json \
                    -client-template=scripts/cpcs/input/client-template.json -pls=0 \
                    && pkill mallob')
        end = time.time()
        times.append(end - start)
        print(end - start)
    return times


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance (single problem file)", required=True)
    parser.add_argument("-o", help="Output File", required=True)
    parser.add_argument("-s", help="Step Size of Clauses to delete", type=int, default=1)
    parser.add_argument("-n", help="Tests to perform per clauses to delete", type=int, default=20)
    parser.add_argument("-m", help="Max clauses to delete", type=int, default=30)
    args = parser.parse_args()
    main(args.i, args.o, args.s, args.n, args.m)
