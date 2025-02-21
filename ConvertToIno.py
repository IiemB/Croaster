import os

def convert_main():
    cwd = os.getcwd()
    
    source_file = f"{cwd}/src/main.cpp"
    target_directory = f"{cwd}/croaster/"
    
    
    if not os.path.exists(source_file):
        print("Error: Source file does not exist.")
        return

    if not os.path.exists(target_directory):
        os.makedirs(target_directory)

    target_file = os.path.join(target_directory, "Croaster.ino")

    with open(source_file, "r") as src, open(target_file, "w") as dest:
        for line in src:
            dest.write(line)

    print(f"File saved at: {target_file}")
    
def convert_croaster():
    cwd = os.getcwd()
    
    source_file = f"{cwd}/include/Croaster.h"
    target_directory = f"{cwd}/croaster/"
    
    
    if not os.path.exists(source_file):
        print("Error: Source file does not exist.")
        return

    if not os.path.exists(target_directory):
        os.makedirs(target_directory)

    target_file = os.path.join(target_directory, "Croaster.h")

    with open(source_file, "r") as src, open(target_file, "w") as dest:
        for line in src:
            dest.write(line)

    print(f"File saved at: {target_file}")


convert_main()
convert_croaster()
