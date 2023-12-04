# Growing Corn(us) Faster

by Ankith Udupa, Anurag Bangera, Luke Lesh

# Directory Structure

## Cornus

### Cornus/include/config

This folder contains code for data structures that read and maintain the configuration of nodes and transaction handlers.

### Cornus/include/lib

This folder contains external libraries that we make use of such as http and json. It also contains our custom timer library that allows for efficient collection of timestamp breakdowns across a transaction.

### Cornus/include/messaging

This folder handles the GlobalMessageHandler, our custom-implemented TCP library, and other files related to messaging. The GlobalMessageHandler is the primary driver for each node that forwards messages to the appropriate transaction handler (Coordinator, Participant, Replicator, or Committer). We opted to implement our own TCP library because publically available HTTP libraries did not offer the multithread and performance guarantees that we needed. It also contains a test directory which contains test programs for the TCP library. 

### Cornus/transactionHandlers

This folder contains the primary implementations of the paper version of Cornus and our new version of Cornus. In the paper version of Cornus, there are two types of transaction handlers: Coordinator and Participant. Our new version has four types: Coordinator2, Participant2, Replicator, and Committer. It also includes some parent classes (Sender and TransactionHandler) which we use for common functionality inheritance.

### Cornus/worker

This folder contains the work that the application would do to decide how to vote and what action to do on committing. We have provided a trivial worker that always votes yes for benchmarking.

## DBMS

### DBMS/include

This folder contains our simulated DBMS implementation that is capable of performing writes, reads, and log once. It is contacted over HTTP and includes a configurable delay so that it can be made to mimic real DBMS like Azure or Redis.

## Testing

### Testing/basic

This folder contains python scripts and basic test cases that were used to debug both the paper and new versions of the Cornus Protocol.

### Testing/benchmark

This folder contains python scripts used to perform the benchmarking presented in our final report.

# How to run Cornus and benchmarks

Note: our testing and benchmarking were done on a Ubuntu 20 machine with a relatively large amount of computing resources. Performance and compatibility beyond this has not been tested. 

## Compilation

### Compiling Cornus

From the root directory:

```
$ cd Cornus
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug/Release ..
$ make
```

### Compiling the Simulated DBMS

From the root directory:

```
$ cd DBMS
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug/Release ..
$ make
```

### Running Cornus

You can run a single Cornus node manually with the following command (from the Cornus/build directory)

```
$ ./Cornus path/to/config
```

A Cornus config file is a JSON file that should be formated as follows:

```
{
    "host" : "localhost",
    "others" : [{"host_id" : "localhost:8002"},
                {"host_id" : "localhost:8004"},
                {"host_id" : "localhost:8006"},
                {"host_id" : "localhost:8008"},
                {"host_id" : "localhost:8010"}
                ],
    "dbms_address" : "localhost:9000",
    "host_num" : 0,
    "port" : 8000,
    "timeout_millis": 500,
    "f": 1
}
```

There are 6 config files already in the Cornus project directory that can be used for 6 hosts all on the same machine.

### Running the DBMS

You can run the DBMS manually with the following command (from the DBMS/build directory)

```
$ ./DBMS <DBMS port> <DBMS Delay(Default 10ms)> <DBMS host(Default localhost)>
```

### Running a local benchmark

To run a local benchmark first, set the 1-way network delay to be half of the desired network RTT delay with the following command (note that we only tested this on linux):

```
$ sudo tc qdisc add dev lo root handle 1:0 netem delay 0.25msec
```

To restore you can run:

```
$ sudo tc qdisc del dev lo root
```

Before we can run the benchmarking script we need to send the binaries for Cornus and the DBMS into locations that the bechmarking script can run them from.

Edit line 20 and 21 `Cornus/include/messaging/GlobalMessageHandler.hpp` to enable either the Paper or New version of the protocol then compile. Then copy the binary from the build directory into either `Testing/benchmark/Conrus` or `Testing/benchmark/Cornus2` depending on which one you have compiled (you will probably want to do this for both).

Now navigate to Testing/benchmark. You can run the benchmarking script to start up a specified number of Cornus nodes in the desired configuration and send requests from a specified number of concurrent clients.

```
$ python3 benchmark.py --nodes <num_nodes> --clients <num_clients> -f <f> <-v2> --num <num_requests>
```

The raw output of this script will just be all of the nodes' logs, so it may be more useful to redirect the output into a file for analysis later.

We've created log parsing scripts for the specific graphs and experiments we ran in the Testing/benchmark directory and visualization scripts in specific test directories (Testing/benchmark/Client-Test for example). Feel free to run them, or create your own parsing and visualization scripts.

