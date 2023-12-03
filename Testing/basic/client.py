"""
Purpose: Makes a series of client transaction requests and prints the responses

Running the script: python3 client.py test1.json nodes.json -c

The test file is an array where each element is a transaction request of the following form: 
{ "coordinator": index int, "participants": [index int, ...], "operation": string }
The index int refers to the corresponding element in the nodes file. 

The nodes file is an array where each element is a string of the host config path

The boolean should be True for concurrent requests and False for sequential requests.

"""

import argparse
import json
import requests
import concurrent.futures
import time

def getHostPort(confFile):
    with open(confFile, 'r') as file:
        conf = json.load(file)
    if conf["port"] % 2 != 0:
        print("INVALID .config file: internal ports must be even!")
        exit(1)
    return conf["host"] + ":" + str(conf["port"])

def getHostPortPlus1(confFile):
    with open(confFile, 'r') as file:
        conf = json.load(file)
    return conf["host"] + ":" + str(conf["port"] + 100)

def generate_transactions(test_file, nodes_file):
    transactions = []
    with open(test_file, 'r') as file:
        request_list = json.load(file)

    with open(nodes_file, 'r') as file:
        nodes = json.load(file)

    for request in request_list:
        print(request)
        coordinator = request["coordinator"]
        participants = request["participants"]
        if coordinator in participants:
            print("INVALID TEST CASE: coordinator cannot also be a participant")
            exit(1)
        if coordinator >= len(nodes) or coordinator < 0:
            print("INVALID TEST CASE: coordinator index out of range")
            exit(1)
        for participant in participants:
            if participant >= len(nodes) or participant < 0:
                print("INVALID TEST CASE: participant index out of range")
                exit(1)
        
        body = getHostPort(nodes[coordinator]) + " "
        for i in range(len(participants)):
            body = body + getHostPort(nodes[participants[i]]) + " "

        #adding 1 because the external port is one above the internal ports
        transactions.append(("http://"+getHostPortPlus1(nodes[coordinator]) +"/TRANSACTION", body))
    return transactions

def make_request(url, body):
    headers = {
    'content-type': 'application/x-www-form-urlencoded',
    }
    params = {
    'config': body,
    }
    #print(url, params, headers)
    start = time.time()
    response = requests.post(url, data=params, headers=headers)
    end = time.time()
    print(end - start)
    return response

def send_requests_sequential(transactions):
    for transaction in transactions:
        response = make_request(transaction[0], transaction[1])
        print(response, response.text)


def send_requests_concurrent(transactions):
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []

        for transaction in transactions:
            futures.append(executor.submit(make_request, url=transaction[0], body=transaction[1]))

        for future in concurrent.futures.as_completed(futures):
            print(future.result(), future.result().text)



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('test', help="File containing JSON requests")
    parser.add_argument('nodes', help="File containing list of nodes host:port")
    parser.add_argument('-c','--concurrent',action="store_true",help="Enable concurrent requests" )
    args = parser.parse_args()

    time.sleep(4)
    print("ready to go.")

    transactions = generate_transactions(args.test, args.nodes)
    if (args.concurrent):
        send_requests_concurrent(transactions)
    else:
        send_requests_sequential(transactions)