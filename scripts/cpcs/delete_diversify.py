import os.path
import shutil
import random
import argparse
import numpy as np


def main(input_file_path, clauses_to_delete, n):
    file_name = os.path.basename(input_file_path)
    output_dir_path = os.path.join(os.path.dirname(input_file_path), "delete_diversified")
    os.makedirs(output_dir_path, exist_ok=True)

    for i in range(n):
        with open(input_file_path, "r") as input_file, open(output_dir_path + f"/diversified-{i}-{file_name}", "w") as output_file:
            lines = input_file.readlines()
            p = lines[0]
            a = lines[-1]
            clauses = lines[1:-1]
            num_clauses_to_keep = round(len(clauses) - clauses_to_delete)

            temp_p = p.split(" ")
            temp_p[-1] = f"{num_clauses_to_keep}\n"
            new_p = " ".join(temp_p)

            output_file.write(new_p)
            output_file.writelines(random.sample(clauses, num_clauses_to_keep))
            output_file.write(a)

    return [output_dir_path + f"/diversified-{i}-{file_name}" for i in range(n)]


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", help="Input File Path", required=True)
    parser.add_argument("-d", help="Num clauses to delete", type=float, required=True)
    parser.add_argument("-n", help="Number of problem instances to create", type=int, required=True)
    args = parser.parse_args()
    main(args.i, args.o, args.n)
