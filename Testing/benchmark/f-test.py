import numpy as np

#just copied from results spreadsheet
dbms_10_v2 = np.array([7.207155,
7.63707,
7.743045,
8.144555,
8.33561,
8.861895])
dbms_2_v2 = np.array([7.22443,
7.45217,
7.757545,
8.022995,
8.55149,
8.957065])

nodes = [1,2,3,4,5,6]

import matplotlib.pyplot as plt
plt.plot(nodes, dbms_10_v1 * 1000, marker='o')
plt.plot(nodes, dbms_2_v1 * 1000, marker='o')
plt.plot(nodes, dbms_2_v2 * 1000, marker='o')
plt.plot(nodes, dbms_10_v2 * 1000, marker='o')

plt.xlabel("Number of Nodes")
plt.ylim([0, 20])
plt.ylabel("Median Request Latency (ms)")
plt.title("Median Request Latency vs Number of Nodes")
plt.legend(["Cornus 1, 10ms DBMS", "Cornus 1, 2ms DBMS", "Cornus 2, 2ms DBMS", "Cornus 2, 10ms DBMS"], loc="best")
plt.show()