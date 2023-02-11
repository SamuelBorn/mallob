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
    else:
        with open(instance_file, "r") as f:
            j = len(f.readlines())

    results = {
        'nogroup': run_multiple(timeout, instance_file, j, num_cores, 'nogroup', n),
        'group-nocheck': run_multiple(timeout, instance_file, j, num_cores, 'group-nocheck', n)
    }

    with open(output_file_path, "w") as f:
        f.write(json.dumps(results))


def run_once(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup):
    output = subprocess.check_output(f'mpirun -np {num_cores} --bind-to core build/mallob '
                                     f'-jwl={timeout_seconds} -v=2 -c=1 -ajpc={num_jobs} -ljpc={2 * num_jobs} -J={num_jobs} '
                                     f'-job-desc-template={instance_file} '
                                     f'-job-template=scripts/cpcs/input/job-{identifier_group_nogroup}.json '
                                     f'-client-template=templates/client-template.json', shell=True)

    filtered = [float(line.split(" ")[4]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]
    return sum(filtered) if len(filtered) >= num_jobs else 2 * timeout_seconds


def run_multiple(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup, n):
    results = []
    for i in range(n):
        print(f"{datetime.datetime.now()} - {i}/{n}")
        result = run_once(timeout_seconds, instance_file, num_jobs, num_cores, identifier_group_nogroup)
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
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=240)
    args = parser.parse_args()
    main(args.p, args.i, args.o, args.n, args.j, args.c, args.t)
