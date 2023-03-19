def main():
    problem_name = "termes"
    with open(problem_name, "w") as f:
        for i in range(61):
            f.write(f"../madagascar/domains/sat/{problem_name}/termes-termes-00684-0128-4x4x8-random_towers_4x4_8_1_6.{i:03d}.cnf\n")


if __name__ == '__main__':
    main()
