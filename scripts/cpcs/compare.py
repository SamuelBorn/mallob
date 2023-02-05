import argparse
import json
import os
import time


def main(input_file_path, output_file_path, n):
    num_problems = 2
    instance_file = "scripts/cpcs/output/timing_instace_file.txt"
    with open(instance_file, "w") as f:
        for _ in range(num_problems):
            f.write(input_file_path + "\n")

    results = {}
    for idx, job_template in enumerate(["scripts/cpcs/input/job-template.json", "scripts/cpcs/input/job-template-nogroup.json"]):
        results[idx] = []
        for i in range(n):
            print(f"Solve {i}/{n}: {job_template}")
            start = time.time()
            os.system(f'mpirun -np 4 build/mallob -v=1 -c=1 -ajpc={num_problems} -ljpc={2 * num_problems} -J={num_problems} \
                        -job-desc-template={instance_file} \
                        -job-template={job_template} \
                        -client-template=scripts/cpcs/input/client-template.json -pls=0 \
                        && pkill mallob')
            end = time.time()
            print(end - start)
            results[idx].append(end - start)

            with open(output_file_path, "w") as f:
                f.write(json.dumps(results))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance", required=True)
    parser.add_argument("-o", help="Output File", required=True)
    parser.add_argument("-n", help="Tests to perform", type=int, default=20)
    args = parser.parse_args()
    main(args.i, args.o, args.n)
