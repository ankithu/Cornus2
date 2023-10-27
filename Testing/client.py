"""
Purpose: Makes a series of client transaction requests and prints the responses

Running the script: python3 client.py test1.json nodes.json True

The test file is an array where each element is a transaction request of the following form: 
{ "coordinator": index int, "participants": [index int, ...], "operation": string }
The index int refers to the corresponding element in the nodes file. 

The nodes file is an array where each element is a string of the form "hostname:port"

The boolean should be True for concurrent requests and False for sequential requests.

"""

import argparse
import json
import requests
import concurrent.futures



def generate_transactions(test_file, nodes_file):
    transactions = []
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
        
        body = nodes[coordinator] + " "
        for i in range(len(participants)):
            body = body + nodes[participants[i]] + " "


        transactions.append(("http://"+nodes[coordinator] +"/TRANSACTION", body))
    return transactions

def make_request(url, body):
    response = requests.post(url, data=body)
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
    parser.add_argument('concurrent', choices=('True','False', 'true', 'false'))
    args = parser.parse_args()

    transactions = generate_transactions(args.test, args.nodes)
    if (args.concurrent.lower() == "true"):
        send_requests_concurrent(transactions)
    else:
        send_requests_sequential(transactions)