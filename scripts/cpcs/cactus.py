import json

import compare
import subprocess
import sys
import argparse
from datetime import datetime


def main(instances, output, time_limit, cores, jobs):
    output = 'scripts/cpcs/output/benchmark/'

    #for instance in ['agricola', 'caldera', 'caldera-split', 'data-network', 'flashfill', 'nurikabe', 'settlers', 'snake', 'termes']:
    for instance in ['agricola', 'caldera']:
        results = {
            '4 solver, 0 checker': run_mallob_and_get_finish_times('scripts/cpcs/benchmark/' + instance, time_limit, cores, jobs, "nogroup", 4, 0),
            '4 solver, 1 checker': run_mallob_and_get_finish_times('scripts/cpcs/benchmark/' + instance, time_limit, cores, jobs, "group-check", 4, 1)
        }
        with open(output + instance, "w") as f:
            f.write(json.dumps(results))


def run_mallob_and_get_finish_times(instances, time_limit, cores, jobs, group_nogroup, threads, ecct):
    print(f'Started: {datetime.now()}, {group_nogroup}, t={threads}, ecct={ecct}')
    output = subprocess.check_output(f'mpirun -np {cores} --bind-to core --map-by ppr:{cores}:node:pe={threads} build/mallob '
                                     f'-T={time_limit} -v=2 -J=80 -ajpc={jobs} -t={threads} -ecct={ecct} '
                                     f'-job-desc-template={instances} '
                                     f'-job-template=scripts/cpcs/input/job-{group_nogroup}.json '
                                     f'-client-template=templates/client-template.json', shell=True)

    # exclusive mpirun -np 8 --bind-to core --map-by ppr:8:node:pe=8 build/mallob -T=1200 -v=4 -J=1 -ajpc=3 -t=8 -ecct=1 -job-desc-template=scripts/cpcs/input/instance_file_4.txt -job-template=scripts/cpcs/input/job-group-check.json -client-template=templates/client-template.json | grep -i "cpcs\|exited\|error\|warn\|solution\|over-due\|RESPONSE_TIME\|am idle"

    lines = [line for line in output.decode("utf-8").split("\n") if "RESPONSE_TIME" in line]
    exec_times = []
    for line in lines:
        try:
            exec_time = float(line.split(" ")[0])
            exec_times.append(exec_time)
        except:
            try:
                exec_time = float(line.split(" ")[1])
                exec_times.append(exec_time)
            except:
                print('could not convert the following line: ', line)

    return exec_times


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance File (multiple instances)", default="scripts/cpcs/input/instances_settlers_09")
    parser.add_argument("-o", help="Output File", default="scripts/cpcs/output/finish_times")
    parser.add_argument("-t", help="Timeout in seconds", type=int, default=5)
    parser.add_argument("-c", help="Num cores to execute on", type=int, default=2)
    parser.add_argument("-j", help="Num jobs to run with", type=int, default=2)
    args = parser.parse_args()
    main(args.i, args.o, args.t, args.c, args.j)
