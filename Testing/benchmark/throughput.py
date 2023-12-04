"""
To run:
python3 parser.py --file output.txt
"""

import re
import argparse
import numpy as np

def clean_txid_lines(txid_lines):
    cleaned_lines = []
    for line in txid_lines:
        cleaned_line = re.sub(r'[^0-9,\.]', '', line)
        cleaned_lines.append(cleaned_line)
    return cleaned_lines

def extract_numbers_after_first_comma(cleaned_lines):
    extracted_numbers = []
    for line in cleaned_lines:
        # Find the decimal number after the first comma using regular expression
        match = re.search(r',([\d.]+)', line)
        if match:
            extracted_numbers.append(float(match.group(1)))
    return extracted_numbers

def extract_dates(lines):
    extracted_numbers = []
    for line in lines:
        # Find the decimal number after the first comma using regular expression
        match = re.search(r'cur_time: .+', line)
        if match:
            full = match.group()
            match = re.search(r'[0-9][0-9]:[0-9][0-9]:[0-9][0-9]', full)
            extracted_numbers.append(match.group())
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

        cleaned_lines = clean_txid_lines(txid_lines)
        #print("\nCleaned lines:")
        #for cleaned_line in cleaned_lines:
        #    print(cleaned_line)

        extracted_numbers = extract_numbers_after_first_comma(cleaned_lines)
        extracted_dates = extract_dates(txid_lines)
        print(extracted_dates)
        print("\nExtracted internal total latencies:")
        print(np.median(extracted_numbers),np.mean(extracted_numbers),np.std(extracted_numbers), np.percentile(extracted_numbers, 99))
        print("total: ", len(extracted_numbers))
        print("median: ", np.median(extracted_numbers))
        print("mean: ",np.mean(extracted_numbers))
        print("standard deviation: ",np.std(extracted_numbers))
        print("99%: ",np.percentile(extracted_numbers, 99))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    #Setup
    parser.add_argument('--file', required=True,help="Output file to run the parser on")

    args = parser.parse_args()
    parse(args.file)
    
