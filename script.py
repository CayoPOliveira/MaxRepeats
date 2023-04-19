from tests import pizzachili
import argparse
import subprocess
import os


def run_eGap(mem_limit, file):
    path = "external/eGap/"
    subprocess.run("make", cwd=path)
    subprocess.run(["./eGap", "--lcp", "--sa", "--em","--mem",
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
    parser.add_argument(
        '-v', dest='verbose', action='store_true', help='Verbose options on repeats')
    parser.add_argument(
        '-e', '--eGap', dest='egap', action='store_true', help='Run eGap to make .sa, .lcp and .bwt files')

    # Set default values for the arguments
    parser.set_defaults(type1=False, type2=False,
                        use_stl=False, use_stxxl=False,
                        mem_limit=4096, file="", verbose=False, egap=False)

    # Parse the arguments
    args = parser.parse_args()

    # Print the values of the arguments
    if args.verbose:
        print("Compute repeats type 1:", args.type1)
        print("Compute repeats type 2:", args.type2)
        print("Use STL stack in memory RAM:", args.use_stl)
        print("Use STXXL stack in disk:", args.use_stxxl)
        print("Memory limit in KB:", args.mem_limit)
        print("File path:", args.file)
        if args.egap:
            print("Run eGap.")


    return args

def eGapPizza(args):
    print("Pizza tests running...")
    for filename in os.listdir("tests/pizza/"):
        if filename.endswith('.txt'):
            run_eGap(args.mem_limit, "tests/pizza/" + filename)

def run_repeat(args):
    print("Repeats Compile")
    subprocess.run("make")
    print("Repeats running...")
    Run = ["./repeats", f"{args.file}", f"{args.file}.bwt", f"{args.file}.2.lcp", f"{args.file}.4.sa"]
    if(args.type1): Run.append("-1")
    if(args.type2): Run.append("-2")
    if(args.use_stl): Run.append("-stl")
    if(args.use_stxxl): Run.append("-stxxl")
    if(args.mem_limit!=4096):
        Run.append("-m")
        Run.append(str(args.mem_limit))
    if(args.verbose): Run.append("-v")
    subprocess.run(Run)


def main():
    pizzachili.pizza()
    args = get_args()
    if(args.file == ""):
        eGapPizza(args)
        for filename in os.listdir("tests/pizza/"):
            if filename.endswith('.txt'):
                args.file = filename
                run_repeat(args)
    else:
        if args.egap:
            run_eGap(args.mem_limit, args.file)
        run_repeat(args)


if __name__ == "__main__":
    main()
