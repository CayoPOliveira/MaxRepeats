from tests import pizzachili
import argparse
import subprocess
# import os


def run_eGap(mem_limit, file):
    path = "external/egap/"
    subprocess.run("make", cwd=path)
    subprocess.run(["./eGap", "--lcp", "--mem",
                    str(mem_limit), f"../../{file}"], cwd=path)


def get_args():
    parser = argparse.ArgumentParser(
        description='Process command line arguments')

    # Define the arguments
    parser.add_argument(
        '-1', dest='type1', action='store_true', help='Compute repetitions of type 1')
    parser.add_argument(
        '-2', dest='type2', action='store_true', help='Compute repetitions of type 2')
    parser.add_argument('-stl', dest='use_stl',
                        action='store_true', help='Use STL stack in memory RAM')
    parser.add_argument('-stxxl', dest='use_stxxl',
                        action='store_true', help='Use STXXL stack in disk')
    parser.add_argument('-m', '--mem', dest='mem_limit',
                        type=int, help='Memory limit in MB')
    parser.add_argument('-f', '--file', dest='file',
                        type=str, help='File path')

    # Set default values for the arguments
    parser.set_defaults(type1=False, type2=False,
                        use_stl=False, use_stxxl=False,
                        mem_limit=4096)

    # Parse the arguments
    args = parser.parse_args()

    # Print the values of the arguments
    print("Compute repeats type 1:", args.type1)
    print("Compute repeats type 2:", args.type2)
    print("Use STL stack in memory RAM:", args.use_stl)
    print("Use STXXL stack in disk:", args.use_stxxl)
    print("Memory limit in MB:", args.mem_limit)
    print("File path:", args.file)

    return args


def main():
    pizzachili.pizza()
    args = get_args()
    run_eGap(args.mem_limit, args.file)


if __name__ == "__main__":
    main()
