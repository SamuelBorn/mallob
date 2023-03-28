def main():
    number = 7
    domain = "data-network"
    with open(domain + f"_p{number:02d}", "w") as f:
        for i in range(60):
            f.write(f"../madagascar/domains/sat/{domain}/data-network-p44-3-88-small-network-26.{i:03d}.cnf\n")


if __name__ == '__main__':
    main()
