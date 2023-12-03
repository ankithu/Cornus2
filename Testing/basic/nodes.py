"""
Purpose: Starts up Cornus nodes at the host:port specified in the input file

Running the script: python3 nodes.py nodes.json

The nodes file is an array where each element is a string of the host config path
"""

import argparse
import json
import requests
import multiprocessing
import subprocess
import signal
import sys

clion = False
BUILD_DIR_NAME = "cmake-build-debug" if clion else "build"

def run_executable(command):
    process = subprocess.Popen(command, shell=True)
    process.wait()

def get_node_start_commands(nodes_file):
    with open(nodes_file, 'r') as file:
        nodes = json.load(file)
    
    commands = [f"./../../DBMS/{BUILD_DIR_NAME}/DBMS 9000"]
    for node in nodes:
        commands.append(f"./../../Cornus/{BUILD_DIR_NAME}/Cornus " + node)
    return commands

def start_nodes(nodes_file):
    commands = get_node_start_commands(nodes_file)
    pool = multiprocessing.Pool(processes=len(commands))
    pool.map(run_executable, commands)
    pool.close()
    pool.join()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('nodes', help="File containing list of nodes host:port")
    args = parser.parse_args()

    start_nodes(args.nodes)
