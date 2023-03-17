import json

import compare
import subprocess
import sys
import argparse
from datetime import datetime


def main(instances, output, time_limit, cores, jobs):
    results = {
        'without sharing':     run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, "nogroup",     8, 0),
        '7 solver, 1 checker': run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, "group-check", 7, 1),
        '6 solver, 2 checker': run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, "group-check", 6, 2),
        '5 solver, 3 checker': run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, "group-check", 5, 3),
        '4 solver, 4 checker': run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, "group-check", 4, 4),
        '2 solver, 6 checker': run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, "group-check", 2, 6),
    }

    with open(output, "w") as f:
        f.write(json.dumps(results))


def run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, group_nogroup, threads, ecct):
    print(f'Started: {datetime.now()}, {group_nogroup}, t={threads}, ecct={ecct}')
    output = subprocess.check_output(f'mpirun -np {cores} --bind-to core build/mallob '
                                     f'-T={time_limit} -v=2 -c=1 -ajpc={jobs} -ljpc={2 * jobs} -J=60 -t={threads} -ecct={ecct} '
                                     f'-job-desc-template={instances} '
                                     f'-job-template=scripts/cpcs/input/job-{group_nogroup}.json '
                                     f'-client-template=templates/client-template.json', shell=True)

    return [float(line.split(" ")[0]) for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance File (multiple instances)", default="scripts/cpcs/input/instances_settlers_09")
    parser.add_argument("-o", help="Output File", default="scripts/cpcs/output/finish_times.json")
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=5)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=2)
    parser.add_argument("-j", help="Num jobs to run with", type=int, default=2)
    args = parser.parse_args()
    main(args.i, args.o, args.t, args.c, args.j)
