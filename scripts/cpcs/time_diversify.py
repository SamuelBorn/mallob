import os
import shutil
import random
import argparse
import numpy
import time


def main(output_file_path, step_size, tests_per_percentage, threshold):
    for del_percentage in numpy.arange(0.84, 1-threshold, -step_size):
        for i in range(tests_per_percentage):
            os.system(f"python3 scripts/cpcs/diversify_random_clause_delete.py -i instances/incremental-accumulated/roverg10-3.cnf -n 3 -d {del_percentage}")
            print(f"Testing p={del_percentage} i={i}/{tests_per_percentage}: ", end="")
            start = round(time.time() * 1000)
            os.system('/usr/lib64/openmpi/bin/mpirun -np 3 build/mallob -v=4 -c=1 -ajpc=3 -ljpc=6 -J=3 \
-job-desc-template=scripts/cpcs/input/instance_file.txt \
-job-template=scripts/cpcs/input/job-template.json \
-client-template=scripts/cpcs/input/client-template.json -pls=0 > /dev/null')
            end = round(time.time() * 1000)
            duration_ms = end - start
            with open(output_file_path, "a") as f:
                f.write(f"{del_percentage} {duration_ms}\n")
            print(f"{duration_ms}")
            os.system("pkill mallob")


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", help="OUTPUT File", required=True)
    parser.add_argument("-s", help="Percentage Step Size of Clauses to delete", type=float, default=0.01)
    parser.add_argument("-n", help="Tests to perform per clauses to delete", type=int, default=10)
    parser.add_argument("-d", help="Max Percentage to clauses to delete", type=float, default=0.3)
    args = parser.parse_args()
    main(args.o, args.s, args.n, args.d)
