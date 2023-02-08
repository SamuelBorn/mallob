import os.path
import shutil
import random
import argparse
import numpy as np


def main(num_variables, overlap, n):
    common_num_clauses, varied_num_clauses = round(4.17 * overlap * num_variables), round(4.17 * (1 - overlap) * num_variables)
    p = create_problem(num_variables, common_num_clauses)

    os.makedirs(f"scripts/cpcs/temp", exist_ok=True)
    for i in range(n):
        with open(f"scripts/cpcs/temp/diversified-{i}.cnf", "w") as f:
            f.write(p + create_clauses(num_variables, varied_num_clauses))

    return [f"scripts/cpcs/temp/diversified-{i}.cnf" for i in range(n)]


def create_problem(num_variables, num_clauses):
    out = "p cnf " + str(num_variables) + " " + str(num_clauses) + "\n"
    out += create_clauses(num_variables, num_clauses)
    return out


def create_clauses(num_variables, num_clauses):
    out = ""
    for c in range(num_clauses):
        literals = []
        for i in range(3):
            lit = 0
            while lit == 0 or lit in literals or -lit in literals:
                lit = random.randrange(-num_variables, num_variables + 1)
            literals += [lit]
        for lit in literals:
            out += str(lit) + " "
        out += "0\n"
    return out


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-v", help="Num vars in problem", type=int, default=1000)
    parser.add_argument("-o", help="Overlap between problems", type=float, default=0.99)
    parser.add_argument("-n", help="Number of problem instances to create", type=int, default=4)
    args = parser.parse_args()
    main(args.v, args.o, args.n)
