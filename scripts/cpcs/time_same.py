import argparse
import os
import time


def main(input_file_path, output_file_path, n):
    num_group_members = 4
    instance_file = "scripts/cpcs/output/timing_instace_file.txt"
    with open(instance_file, "w") as f:
        for _ in range(num_group_members):
            f.write(input_file_path + "\n")

    for i in range(n):
        print(f"Solve {i}/{n}")
        start = time.time()
        os.system(f'/usr/lib64/openmpi/bin/mpirun -np 6 build/mallob -c=1 -ajpc=3 -ljpc=6 -J=3 \
                        -job-desc-template={instance_file} \
                        -job-template=scripts/cpcs/input/job-template.json \
                        -client-template=scripts/cpcs/input/client-template.json -pls=0 \
                        > /dev/null && pkill mallob')
        end = time.time()
        print(end - start)

        with open(output_file_path, "a") as f:
            f.write(f"{end - start}\n")


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input Instance", required=True)
    parser.add_argument("-o", help="Output File", required=True)
    parser.add_argument("-n", help="Tests to perform", type=int, default=20)
    args = parser.parse_args()
    main(args.i, args.o, args.n)
