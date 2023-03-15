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
import diversify_amend


def main(single_problem, output_file_path, num_tests, num_jobs, num_cores, timeout):
    cube_instances = create_cube_instances(single_problem, num_jobs)
    duplication_instances = create_duplication_instances(single_problem, num_jobs)

    results = {
        "group + cube": compare.run_multiple_unfiltered(timeout, cube_instances, num_jobs, num_cores, "group-nocheck", num_tests, num_jobs),
        "no group + cube": compare.run_multiple_unfiltered(timeout, duplication_instances, num_jobs, num_cores, "nogroup", num_tests, num_jobs),
        "no group + mono": exec_mono_multiple(num_cores, timeout, single_problem, num_tests)
    }

    with open(output_file_path, "w") as f:
        json.dump(results, f)


def cube_compare(num_cores, num_tests, timeout=100):
    instances = ['../madagascar/domains/sat/snake-snake-empty-6x6-1-5-18-22170.035.cnf',
                 '../madagascar/domains/sat/snake-snake-empty-6x6-1-5-18-22170.036.cnf',
                 '../madagascar/domains/sat/snake-snake-empty-6x6-1-5-18-22170.037.cnf',
                 '../madagascar/domains/sat/snake-snake-empty-6x6-1-5-18-22170.038.cnf']

    for idx, instance in enumerate(instances):
        cube_instances = create_cube_instances(instance, 2)
        results = {
            "group + cube2": compare.run_multiple_unfiltered(timeout, cube_instances, 2, num_cores, "group-nocheck", num_tests, 2),
            "group + cube4": compare.run_multiple_unfiltered(timeout, cube_instances, 4, num_cores, "group-nocheck", num_tests, 4),
            "no group + cube2": compare.run_multiple_unfiltered(timeout, cube_instances, 2, num_cores, "nogroup", num_tests, 2),
            "no group + cube4": compare.run_multiple_unfiltered(timeout, cube_instances, 4, num_cores, "nogroup", num_tests, 4),
            "no group + mono": exec_mono_multiple(num_cores, timeout, instance, num_tests)
        }
        with open(f"scripts/cpcs/output/cube_results_{idx}.json", "w") as f:
            json.dump(results, f)


def cube_increase(timeout, cores, output="scripts/cpcs/output/cube_times.json"):
    results = {"cube2": [], "cube4": [], "mono": []}

    for i in range(200, 700, 50):
        print(i)
        instance = "scripts/cpcs/temp/cube_instance"
        with open(instance, "w") as f:
            f.write(diversify_amend.create_problem2(i))

        print('cube2')
        cube_instances = create_cube_instances(instance, 2)
        cube2 = compare.run_once_unfiltered(timeout, cube_instances, 2, cores, 'group-nocheck')
        results["cube2"].append(max(cube2))

        print('cube4')
        cube_instances = create_cube_instances(instance, 4)
        cube4 = compare.run_once_unfiltered(timeout, cube_instances, 4, cores, 'group-nocheck')
        results["cube4"].append(max(cube4))

        print('mono')
        mono = exec_mono(cores, timeout, instance)
        results["mono"].append(max(mono))

        with open(output, "w") as f:
            f.write(json.dumps(results))


def exec_mono_multiple(num_cores, timeout_seconds, single_problem, num_tests):
    results = []
    for i in range(num_tests):
        print(f"{datetime.datetime.now()} - {i}/{num_tests} - mono")
        result = exec_mono(num_cores, timeout_seconds, single_problem)
        print(result)
        results.append(result)
    return results


def exec_mono(num_cores, timeout_seconds, single_problem):
    output = subprocess.check_output(f'mpirun -np {num_cores} --bind-to core build/mallob -jwl={timeout_seconds} -v=2 -c=1 -mono={single_problem}', shell=True)
    return [float(line.split(" ")[5]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]


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
    parser.add_argument("-i", help="Input a single Problem file (will get duplicated)", default="instances/r3sat_500.cnf")
    parser.add_argument("-o", help="Output File", default="scripts/cpcs/output/cube_time.json")
    parser.add_argument("-n", help="Tests to perform", type=int, default=1)
    parser.add_argument("-j", help="Num jobs to run with", type=int, default=4)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=4)
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=240)
    args = parser.parse_args()
    # main(args.i, args.o, args.n, args.j, args.c, args.t)
    # cube_increase(args.t, args.c)
    cube_compare(args.c, args.n, args.t)
