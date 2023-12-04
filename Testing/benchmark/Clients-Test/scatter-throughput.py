
#pasted results from benchmark/throughput.py
dbms_2ms_results = [
(248.50461799411877, 0.005928065),
(292.28131815455964, 0.0065613),
(233.30378989555263, 0.005685685),
(288.53570140323416, 0.00638656),
(98.88597713251, 0.00592976),
(280.5984516229497, 0.006199765),
(211.1262919970029, 0.005679785),
(300.08128502822024, 0.0064245299999999995),
(262.27117618167864, 0.0060076999999999995),
(168.27891322237463, 0.005715925)
]

dbms_10ms_results = [
(239.99164498846935, 0.0059794900000000005),
(293.0635548579781, 0.00668442),
(230.72908099840092, 0.00573523),
(280.96229906603287, 0.006826765),
(99.26201605396668, 0.0058473250000000004),
(263.456707542529, 0.00686908),
(205.69878621906122, 0.005966020000000001),
(296.8444734883674, 0.00654276),
(256.81585880025347, 0.00638916),
(162.68120267722165, 0.005812615)
]


dbms_2ms_results = sorted(dbms_2ms_results)
dbms_10ms_results = sorted(dbms_10ms_results)

import numpy as np
dbms_2ms_throughputs = np.array([t for t, _ in dbms_2ms_results])
dbms_10ms_throughputs = np.array([t for t, _ in dbms_10ms_results])

dbms_2ms_latencies = np.array([l for _, l in dbms_2ms_results])
dbms_10ms_latencies = np.array([l for _, l in dbms_10ms_results])

from matplotlib import pyplot as plt

plt.plot(dbms_2ms_throughputs, dbms_2ms_latencies * 1000, '-o')
plt.plot(dbms_10ms_throughputs, dbms_10ms_latencies * 1000, '-o')
plt.legend(['2ms dbms delay', '10ms dbms delay'])
plt.title("Median request latency vs. throughput for Cornus 2")
plt.xlabel('Throughput (Requests/Sec)')
plt.ylabel('Median request latency (msec)')
plt.ylim([5, 10])
plt.show()