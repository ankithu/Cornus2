import numpy as np

#just copied from results spreadsheet
dbms_10_v1 = np.array([0.0161815, 0.01639775, 0.0173035, 0.01798705])
dbms_10_v2 = np.array([0.00394347,0.00458876,0.005261685,0.005975725])
dbms_2_v1 = np.array([0.008028265, 0.008183765, 0.009004265, 0.00943633])
dbms_2_v2 = np.array([0.00393072, 0.004533115, 0.00535017, 0.006125165])

nodes = [3, 4, 6, 8]

import matplotlib.pyplot as plt
plt.plot(nodes, dbms_10_v1 * 1000, marker='o')
plt.plot(nodes, dbms_2_v1 * 1000, marker='o')
plt.plot(nodes, dbms_2_v2 * 1000, marker='o')
plt.plot(nodes, dbms_10_v2 * 1000, marker='o')

plt.xlabel("Number of Nodes")
plt.ylim([0, 20])
plt.ylabel("Median Request Latency (ms)")
plt.title("Median Request Latency vs number of Nodes")
plt.legend(["10ms delay Cornus 1", "2ms delay Cornus 1", "2ms delay Cornus 2", "10ms delay Cornus 2"], loc="best")
plt.show()