import argparse
import json
import os
import random
import time
import datetime
import subprocess
import numpy as np
import shutil
import math
import compare


def main(single_problem, output_file_path, num_tests, num_jobs, num_cores, timeout):
    cube_instances = create_cube_instances(single_problem, num_jobs)
    print(cube_instances)
    duplication_instances = create_duplication_instances(single_problem, num_jobs)

    results = {
        "cube": compare.run_multiple(timeout, cube_instances, num_jobs, num_cores, "group-nocheck", num_tests, 1),
        "group-nocheck": compare.run_multiple(timeout, duplication_instances, num_jobs, num_cores, "group-nocheck", num_tests, 1),
        "nogroup": compare.run_multiple(timeout, duplication_instances, num_jobs, num_cores, "nogroup", num_tests, 1),
        "nogroup-one-job": compare.run_multiple(timeout, duplication_instances, 1, num_cores, "nogroup", num_tests, 1),
    }

    with open(output_file_path, "w") as f:
        json.dump(results, f)


def create_duplication_instances(single_problem, num_jobs):
    duplication_instances = f"scripts/cpcs/temp/duplication-instances"
    with open(duplication_instances, "w") as f:
        for i in range(num_jobs):
            f.write(single_problem + "\n")
    return duplication_instances


def create_cube_instances(single_problem, num_jobs):
    assert np.ceil(np.log2(num_jobs)).is_integer()
    num_cube_vars = int(np.log2(num_jobs))
    cube_vars = list(range(1, num_cube_vars + 1))

    cube_instances = f"scripts/cpcs/temp/cube-instances"
    open(cube_instances, "w").close()

    for i in range(num_jobs):
        cube_instance = f"scripts/cpcs/temp/cube-instance-{i}.cnf"
        shutil.copyfile(single_problem, cube_instance)
        with open(cube_instance, "a") as f:
            f.write("a " + " ".join([str(num) for num in negate_list_based_on_int_bits(i, cube_vars)]) + " 0\n")
        with open(cube_instances, "a") as f:
            f.write(cube_instance + "\n")

    return cube_instances


# negates nums[i] if the i-th bit of x is set
def negate_list_based_on_int_bits(x, nums):
    new_nums = nums.copy()
    for i, num in enumerate(nums):
        if (x >> i) & 1:
            new_nums[i] = -nums[i]
    return new_nums


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input a single Problem file (will get duplicated)", required=True)
    parser.add_argument("-o", help="Output File", required=True)
    parser.add_argument("-n", help="Tests to perform", type=int, default=20)
    parser.add_argument("-j", help="Num jobs to run with", type=int, default=4)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=4)
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=240)
    args = parser.parse_args()
    main(args.i, args.o, args.n, args.j, args.c, args.t)
