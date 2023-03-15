import json

import compare
import subprocess
import sys
import argparse


def main(instances, output, time_limit, cores, jobs):
    results = {
        'nogroup-2': run_mallob_and_get_finish_times(instances, time_limit, cores, 2, "nogroup"),
        'group-check-2': run_mallob_and_get_finish_times(instances, time_limit, cores, 2, "group-check"),
        'nogroup-3': run_mallob_and_get_finish_times(instances, time_limit, cores, 3, "nogroup"),
        'group-check-3': run_mallob_and_get_finish_times(instances, time_limit, cores, 3, "group-check"),
        'nogroup-4': run_mallob_and_get_finish_times(instances, time_limit, cores, 4, "nogroup"),
        'group-check-4': run_mallob_and_get_finish_times(instances, time_limit, cores, 4, "group-check"),
        'nogroup-5': run_mallob_and_get_finish_times(instances, time_limit, cores, 5, "nogroup"),
        'group-check-5': run_mallob_and_get_finish_times(instances, time_limit, cores, 5, "group-check")
    }

    with open(output, "w") as f:
        f.write(json.dumps(results))


def run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, group_nogroup):
    output = subprocess.check_output(f'mpirun -np {cores} --bind-to core build/mallob '
                                     f'-T={time_limit} -v=2 -c=1 -ajpc={jobs} -ljpc={2 * jobs} -J=60 '
                                     f'-job-desc-template={instances} '
                                     f'-job-template=scripts/cpcs/input/job-{group_nogroup}.json '
                                     f'-client-template=templates/client-template.json', shell=True)

    return [float(line.split(" ")[0]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance File (multiple instances)", default="scripts/cpcs/input/instances_settlers_09")
    parser.add_argument("-o", help="Output File", default="scripts/cpcs/output/finish_times.json")
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=5)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=4)
    parser.add_argument("-j", help="Num jobs to run with", type=int, default=3)
    args = parser.parse_args()
    main(args.i, args.o, args.t, args.c, args.j)
