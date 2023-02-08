import os.path
import shutil


def main():
    base_file_name = "roverg10"
    input_dir = "../../instances/incremental/" + base_file_name
    output_dir = "../../instances/incremental-accumulated/" + base_file_name
    num_instances = 6

    shutil.copyfile(input_dir + "-0.cnf", output_dir + "-0.cnf")

    for i in range(1, num_instances):
        with open(output_dir + f"-{i - 1}.cnf", "r") as last_acc, open(input_dir + f"-{i}.cnf", "r") as to_acc, open(output_dir + f"-{i}.cnf", "w") as next_acc:
            old_p = to_acc.readline()
            new_p_split = old_p.split(" ")

            old_clauses = last_acc.readlines()[1:-1]  # ignore a + p of old
            new_clauses = to_acc.readlines()

            new_p_split[-1] = f"{len(old_clauses) + len(new_clauses) - 1}"
            new_p = " ".join(new_p_split)

            next_acc.write(new_p+"\n")
            next_acc.writelines(old_clauses)
            next_acc.writelines(new_clauses)


if __name__ == '__main__':
    main()
