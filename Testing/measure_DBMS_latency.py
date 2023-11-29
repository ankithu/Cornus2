import time
import argparse
import requests

def benchmark(dbmshostport):
    total = 0
    n = 1000
    for i in range(n):
        url = "http://" + dbmshostport + "/LOG_WRITE/0/0/0"
        headers = {
        'content-type': 'text/plain',
        }
        start = time.time()
        response = requests.post(url, data="COMMIT", headers=headers)
        end = time.time()
        latency = end - start
        total = total + latency
    average = total / n
    print("Average: ", average)



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('dbms', help="host:port of the dbms")
    args = parser.parse_args()
    benchmark(args.dbms)


    
