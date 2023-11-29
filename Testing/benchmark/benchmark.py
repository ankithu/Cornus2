#TODO: Make benchmarking able to run off a simple 
# generate a random pass with tunable parameters(which cornus version, number of nodes, average nodes invloved in transactions, number of concurrent clients)
# time all transactions, select number of transactions to benchmark
import argparse
import os
import json
import multiprocessing
import subprocess
import signal
import threading
import time
participants=[] # List of participants by hostId
timeout=0 #Timeout of the system
f=1      # Failure
clion = False
BUILD_DIR_NAME = "cmake-build-debug" if clion else "build"

def random_test_generator():
    return

def run_executable(command):
    process = subprocess.Popen(command, shell=True)
    process.wait()
def start_nodes(nodes,build,timeout,f):
    assert nodes>2 and nodes<50
    
    #Initial Conditions
    port=8000
    host="localhost"
    DBMS_host="localhost"
    DBMS_port=9000
    config_directory="./config"
    DBMS_file="./DBMS/DBMS"
    Cornus_files=["./Cornus/Cornus","./Cornus2/Cornus"]

    participants=[]
    commands=[]

    #Generate Participant Configurations
    print("Creating Configurations...")
    for node in range(0,nodes):
        participant={}
        participant["port"]=port
        participant["host"]=host
        participant["host_num"]=node
        participant["address"]= f'{host}:{port}'
        participants.append(participant)
        port=port+2 #Iterate Port Selection
    #Create Configs
    #Delete existing files
    try:
        files = os.listdir(config_directory)
        for file in files:
            file_path = os.path.join(config_directory, file)
            if os.path.isfile(file_path):
                os.remove(file_path)
        print("All files deleted successfully.")
    except OSError:
        print("Error occurred while deleting files.")
        return
    #Start DBMS command
    commands.append(f'{DBMS_file} {DBMS_port}')

    #Create New Configs
    for participant in participants:
        config={}
        config["others"]=[]
        for exclude in participants:
            if exclude["address"]==participant["address"]:
                continue
            config["others"].append({"host_id":exclude["address"]})
        config["host"]=host
        config["dbms_address"]=f'{DBMS_host}:{DBMS_port}'
        config["host_num"]=participant["host_num"]
        config["port"]=participant["port"]
        config["timeout_millis"]=timeout
        config["f"]=f
        #Write to file
        config_file=f'{config_directory}/host{config["host_num"]}.json'
        with open(config_file, "w") as write_file:
            json.dump(config, write_file,indent=1)
        commands.append(Cornus_files[build]+" "+config_file) #Launch server instance command
    
    print("Configs created. Starting Servers...")
    servers=threading.Thread(target=launch_server, args=(commands,)).start() #Kind of sus but need to launch a thread so the subprocesses close on keyboard escape
    time.sleep(1) #Make sure servers start up
    print("Servers Started")
    
    
    
    

def launch_server(commands):
    pool = multiprocessing.Pool(processes=len(commands))
    pool.map(run_executable, commands)
    pool.close()
    pool.join()

# python3 benchmark.py --nodes 3 --clients 4 -v2 --num 1000
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    #Setup
    parser.add_argument('--nodes', required=True,type=int,help="Number of nodes to launch")
    parser.add_argument('-v2',action="store_true", help="Enable Cornus2 protocol")
    #DBMS
    parser.add_argument('--DBMSport', required=False,type=int,help="DBMS port")
    
    
    #Protocol Config
    parser.add_argument('-f',default=1,type=int,help="Number of nodes which can fail")
    parser.add_argument('-timeout',default=500,type=int,help="Timeout in milliseconds")

    #Setup
    parser.add_argument('--clients', required=True,type=int,help="Number of concurrent client nodes to launch")
    parser.add_argument('--results', required=False,default="output.txt", help="Output file for results")
    #Set Test params
    parser.add_argument('--test', required=False,help="File containing JSON requests")
    #Random Test params
    parser.add_argument('--seed', required=False,help="Seed for random generation")
    parser.add_argument('--num', required=False,type=int,help="Number of tests")
    #parser.add_argument('--tsize', required=False,help="Average size of transactions")


    args = parser.parse_args()
    build=0
    if(args.v2):
        build=1
    start_nodes(args.nodes,build,args.timeout,args.f)
    print(args)