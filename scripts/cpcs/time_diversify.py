import json
import os
import shutil
import random
import argparse
import numpy
import time
import diversify


def main(input_file_path, output_file_path, step_size, tests_per_percentage, threshold):
    with open(output_file_path, "w") as f:
        f.write("{}")  # write empty json for easy handling

    for overlap in numpy.arange(1, threshold, -step_size):
        results = []
        for i in range(tests_per_percentage):

            diversified = diversify.main(input_file_path, overlap, 3)
            instance_file = "scripts/cpcs/output/timing_instace_file.txt"
            with open(instance_file, "w") as f:
                for div in diversified:
                    f.write(div + "\n")

            print(f"Solve {overlap} - {i}/{tests_per_percentage}: ", end="")
            start = time.time()
            os.system(f'/usr/lib64/openmpi/bin/mpirun -np 6 build/mallob -c=1 -ajpc=3 -ljpc=6 -J=3 \
                        -job-desc-template={instance_file} \
                        -job-template=scripts/cpcs/input/job-template.json \
                        -client-template=scripts/cpcs/input/client-template.json -pls=0 \
                        > /dev/null && pkill mallob')
            end = time.time()
            results.append(end - start)
            print(end - start)

        with open(output_file_path, "r") as f:
            output = json.load(f)
            output[overlap] = results
        with open(output_file_path, "w") as f:
            f.write(json.dumps(output))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance", required=True)
    parser.add_argument("-o", help="Output File", required=True)
    parser.add_argument("-s", help="Percentage Step Size of Clauses to delete", type=float, default=0.01)
    parser.add_argument("-n", help="Tests to perform per clauses to delete", type=int, default=20)
    parser.add_argument("-d", help="Max Percentage to clauses to delete", type=float, default=0.8)
    args = parser.parse_args()
    main(args.i, args.o, args.s, args.n, args.d)
