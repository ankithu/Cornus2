"""
Purpose: Makes a series of client transaction requests and prints the responses

Running the script: python3 client.py test1.json nodes.json

The test file is an array where each element is a transaction request of the following form: 
{ "coordinator": index int, "participants": [index int, ...], "operation": string }

The nodes file is an array where each element is a string of the form "hostname:port"

The index int refers to the corresponding element in the nodes file. 
"""

import argparse
import json
import requests



def run_test(test_file, nodes_file):
    with open(test_file, 'r') as file:
        request_list = json.load(file)

    with open(nodes_file, 'r') as file:
        nodes = json.load(file)

    for request in request_list:
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
        
        body = {"coordinator": nodes[coordinator], "participants": []}
        for i in range(len(participants)):
            body["participants"].append(nodes[participants[i]])

        response = requests.post("http://"+nodes[coordinator] +"/TRANSACTION", json=body)
        print(response)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('test', help="File containing JSON requests")
    parser.add_argument('nodes', help="File containing list of nodes host:port")
    args = parser.parse_args()

    run_test(args.test, args.nodes)