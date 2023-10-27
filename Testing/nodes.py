"""
Purpose: Starts up Cornus nodes at the host:port specified in the input file

Running the script: python3 nodes.py nodes.json

The nodes file is an array where each element is a string of the form "hostname:port"
"""

import argparse
import json
import requests
import multiprocessing
import subprocess
import signal
import sys

def run_executable(command):
    process = subprocess.Popen(command, shell=True)
    process.wait()

def start_nodes(nodes_file):
    with open(nodes_file, 'r') as file:
        nodes = json.load(file)
    
    commands = []
    for i in range(len(nodes)):
        host, port = nodes[i].split(":")
        commands.append("./../Cornus/build/Cornus " + str(i) + " " + host + " " + port)
    pool = multiprocessing.Pool(processes=len(commands))
    pool.map(run_executable, commands)
    pool.close()
    pool.join()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('nodes', help="File containing list of nodes host:port")
    args = parser.parse_args()

    start_nodes(args.nodes)