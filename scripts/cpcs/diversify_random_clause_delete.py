import os.path
import shutil
import random
import argparse


def main(input_file_path, n, d):
    file_name = os.path.basename(input_file_path)
    output_dir_path = os.path.join(os.path.dirname(input_file_path), "delete_diversified")
    os.makedirs(output_dir_path, exist_ok=True)

    for i in range(n):
        with open(input_file_path, "r") as input_file, open(output_dir_path + f"/diversified-{i}-{file_name}", "w") as output_file:
            lines = input_file.readlines()
            p = lines[0]
            a = lines[-1]
            clauses = lines[1:-1]
            new_num_clauses = int((1 - d) * len(clauses))

            temp_p = p.split(" ")
            temp_p[-1] = f"{new_num_clauses}\n"
            new_p = " ".join(temp_p)

            output_file.write(new_p)
            output_file.writelines(random.sample(clauses, new_num_clauses))
            output_file.write(a)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", help="Input File Path")
    parser.add_argument("-d", "--delete", help="Percentage of clauses that should be deleted", type=float)
    parser.add_argument("-n", "--number", help="Number of problem instances to create", type=int)
    args = parser.parse_args()
    main(args.input, args.number, args.delete)
