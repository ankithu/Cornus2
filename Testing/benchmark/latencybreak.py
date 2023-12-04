"""
To run:
python3 parser.py --file output.txt
"""

import re
import argparse
import numpy as np


def extract_coord(lines):
    extracted_numbers = []
    for line in lines:
        # Find the decimal number after the first comma using regular expression
        match = re.search(r'start-coordinator', line)
        if match:
            extracted_numbers.append(line)
    return extracted_numbers

def extract_times(lines):
    extracted_numbers = []
    for line in lines:
        match = re.findall(r'[0-9]\.[0-9]+[e][+-][0-9]+|[0-9]\.[0-9]+', line)

        extracted_numbers.append(match)
    return extracted_numbers

def extract_txid_lines(file_path):
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()
            txid_lines = [line.strip() for line in lines if line.startswith("txid")]
        return txid_lines
    except FileNotFoundError:
        print(f"File not found: {file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

def parse(file_path):
    txid_lines = extract_txid_lines(file_path)

    if txid_lines:
        #print("Original lines starting with 'txid':")
        #for line in txid_lines:
        #    print(line)

        cleaned_lines = extract_coord(txid_lines)

        times = extract_times(cleaned_lines)
        latencies = []
        for time in times:
            tmp = []
            tmp.append(float(time[0]))
            total = 0
            for i in range(1, len(time) - 1):
                total = total + float(time[i])
            tmp.append(total)
            tmp.append(float(time[len(time)-1]))
            latencies.append(tmp)
        medians = np.median(latencies, axis=0)
        output = ""
        for med in medians:
            output = output + str(med) + " "
        print(output)





if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    #Setup
    parser.add_argument('--file', required=True,help="Output file to run the parser on")

    args = parser.parse_args()
    parse(args.file)
    
