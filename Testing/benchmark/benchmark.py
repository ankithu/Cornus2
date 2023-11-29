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
import concurrent.futures
import requests

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
def make_request(url, body):
    headers = {
    'content-type': 'application/x-www-form-urlencoded',
    }
    params = {
    'config': body,
    }
    print(url, params, headers)
    start = time.time()
    response = requests.post(url, data=params, headers=headers)
    end= time.time()
    return end-start

def generate_transactions_from_file(test_file, all_participants):
    transactions = []
    with open(test_file, 'r') as file:
        request_list = json.load(file)

    for request in request_list:
        coordinator = request["coordinator"]
        participants = request["participants"]
        if coordinator in participants:
            print("INVALID TEST CASE: coordinator cannot also be a participant")
            exit(1)
        if coordinator >= len(all_participants) or coordinator < 0:
            print("INVALID TEST CASE: coordinator index out of range")
            exit(1)
        for participant in participants:
            if participant >= len(all_participants) or participant < 0:
                print("INVALID TEST CASE: participant index out of range")
                exit(1)
        assert all_participants[coordinator]["host_num"]==coordinator
        body = all_participants[coordinator]["address"] + " "
        for i in range(len(participants)):
            assert all_participants[participants[i]]["host_num"]==participants[i]
            body = body + all_participants[participants[i]]["address"] + " "

        #adding 1 because the external port is one above the internal ports
        transactions.append(("http://"+all_participants[coordinator]["external_address"] +"/TRANSACTION", body))
    return transactions

def delete_files(directory):
    try:
        files = os.listdir(directory)
        for file in files:
            file_path = os.path.join(directory, file)
            if os.path.isfile(file_path):
                os.remove(file_path)
        print("All files deleted successfully.")
    except OSError:
        print("Error occurred while deleting files.")
        return
def start_nodes(args,build):
    
    #Initial Conditions
    nodes=args.nodes
    assert nodes>2 and nodes<50
    port=8000
    port_stride=2
    host="localhost"
    DBMS_host="localhost"
    DBMS_port=9000
    config_directory="./config"
    DBMS_file="./DBMS/DBMS"
    Cornus_files=["./Cornus/Cornus","./Cornus2/Cornus"]
    timeout=args.timeout
    f=args.f
    participants=[]
    commands=[]

    #Generate Participant and Configurations
    print("Creating Configurations...")
    for node in range(0,nodes):
        participant={}
        participant["port"]=port
        participant["host"]=host
        participant["host_num"]=node
        participant["address"]= f'{host}:{port}'
        participant["external_address"]= f'{host}:{port+100}'
        participants.append(participant)
        port=port+port_stride #Iterate Port Selection
    #Create Configs
    delete_files(config_directory)
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
    #Load test cases from file/randomly generate
    transactions=generate_transactions_from_file(args.test,participants)
    
    #Run test cases on clients
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.clients) as executor:
        futures = []
        total=0
        n=0
        times=[]
        for transaction in transactions:
            futures.append(executor.submit(make_request, url=transaction[0], body=transaction[1]))

        for future in concurrent.futures.as_completed(futures):
            print(f'Time: {future.result()}')
            times.append(future.result())
        time.sleep(5)
        print(times)
    #Collate results
    
    
    

def launch_server(commands):
    pool = multiprocessing.Pool(processes=len(commands))
    pool.map(run_executable, commands)
    pool.close()
    pool.join()

# python3 benchmark.py --nodes 3 --clients 4 -v2 --num 1000
# python3 benchmark.py --nodes 3 --clients 1 --test ./test1.json -v2
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
    start_nodes(args,build)
    print(args)