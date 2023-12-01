"""
running the script:
python3 measure_DBMS_latency.py localhost:9000

"""

import time
import argparse
import requests
import concurrent.futures

def make_request(url):
    headers = {
    'content-type': 'text/plain',
    }
    start = time.time()
    failed = False
    try:
        response = requests.post(url, data="COMMIT", headers=headers)
    except: 
        failed = True
        print("CONNECTION REFUSED: ", url)
    end = time.time()
    latency = end - start
    return (latency, failed)

def send_requests_concurrent(transactions):
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []

        for transaction in transactions:
            futures.append(executor.submit(make_request, url=transaction))

        total = 0
        n = 0
        for future in concurrent.futures.as_completed(futures):
            if not future.result()[1]:
                total = total + future.result()[0]
                n = n + 1
        return total / n

def benchmark(dbmshostport):
    n = 6
    transactions = []
    for i in range(n):
        url = "http://localhost:9000/LOG_WRITE/" + str(i) + "/1/TRANSACTION"
        transactions.append(url)
    average = send_requests_concurrent(transactions)
    print("Average: ", average)



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('dbms', help="host:port of the dbms")
    args = parser.parse_args()
    benchmark(args.dbms)


    
