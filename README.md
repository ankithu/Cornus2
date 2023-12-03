# Growing Corn(us) Faster

by Ankith Udupa, Anurag Bangera, Luke Lesh

# Directory Structure

## Cornus

### Cornus/include/config

This folder contains files that handle configuration of each node on start up.

### Cornus/include/lib

This folder contains external libraries that we make use of such as http and json.

### Cornus/include/messaging

This folder handles the GlobalMessageHandler, our custom implemented TCP library, and other files related to messaging. The GlobalMessageHandler is the primary driver for each node that forwards messages to the appropriate transaction handler (Coordinator, Participant, Replicator, or Committer). We opted to implement our own TCP library because publically available HTTP libraries did not offer the multithread and performance guarantees that we needed.

### Cornus/transactionHandlers

This folder contains the primary implementations of the paper version of Cornus and our new version of Cornus. In the paper version of Cornus, there are two types of transaction handler: Coordinator and Participant. Our new version has four types: Coordinator2, Participant2, Replicator, and Committer. It also includes some parent classes (Sender and TransactionHandler) which we use for common functionality inheritance.

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
