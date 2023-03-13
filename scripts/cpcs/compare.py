import argparse
import json
import os
import time
import datetime
import subprocess


def main(single_problem, instance_file, output_file_path, n, j, num_cores, timeout):
    if not instance_file:
        instance_file = "scripts/cpcs/temp/instance_file"
        with open(instance_file, "w") as f:
            for _ in range(j):
                f.write(single_problem + "\n")

    results = {
        'nogroup': run_multiple_unfiltered(timeout, instance_file, j, num_cores, 'nogroup', n),
        'group-check': run_multiple_unfiltered(timeout, instance_file, j, num_cores, 'group-check', n)
    }

    with open(output_file_path, "w") as f:
        f.write(json.dumps(results))


def exec_mallob(num_cores, timeout_seconds, num_jobs, stop_after, instance_file, identifier_group_nogroup):
    return subprocess.check_output(f'mpirun -np {num_cores} --bind-to core build/mallob '
                                   f'-jwl={timeout_seconds} -v=2 -c=1 -ajpc={num_jobs} -ljpc={2 * num_jobs} -J={stop_after} '
                                   f'-job-desc-template={instance_file} '
                                   f'-job-template=scripts/cpcs/input/job-{identifier_group_nogroup}.json '
                                   f'-client-template=templates/client-template.json', shell=True)


def run_once(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, stop_after=None):
    if not stop_after:
        stop_after = num_jobs

    output = exec_mallob(num_cores, timeout_seconds, num_jobs, stop_after, instance_file, identifier_group_nogroup)

    filtered = [float(line.split(" ")[4]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]
    return sum(filtered) / len(filtered) if len(filtered) >= stop_after else 2 * timeout_seconds


def run_multiple(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, n, stop_after=None):
    results = []
    for i in range(n):
        print(f"{datetime.datetime.now()} - {i+1}/{n} - {identifier_group_nogroup}")
        result = run_once(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, stop_after)
        print(result)
        results.append(result)
    return results


def run_once_unfiltered(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, stop_after=None):
    if not stop_after: stop_after = num_jobs

    output = exec_mallob(num_cores, timeout_seconds, num_jobs, stop_after, instance_file, identifier_group_nogroup)

    # filtered = [float(line.split(" ")[4]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]
    filtered = [float(line.split(" ")[0]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]
    while len(filtered) < stop_after: filtered.append(2 * timeout_seconds)
    return filtered


def run_multiple_unfiltered(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, n, stop_after=None):
    results = []
    for i in range(n):
        print(f"{datetime.datetime.now()} - {i+1}/{n} - {identifier_group_nogroup}")
        result = run_once_unfiltered(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, stop_after)
        print(result)
        results.append(result)
    return results


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", help="Input a single Problem file (will get duplicated)")
    parser.add_argument("-i", help="Input Instance File (multiple instances)")
    parser.add_argument("-o", help="Output File", required=True)
    parser.add_argument("-n", help="Tests to perform", type=int, default=20)
    parser.add_argument("-j", help="Num jobs to run with", type=int, default=2)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=4)
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=1000)
    args = parser.parse_args()
    main(args.p, args.i, args.o, args.n, args.j, args.c, args.t)
