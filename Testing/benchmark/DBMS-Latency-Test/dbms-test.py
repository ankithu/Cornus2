import numpy as np

#just copied from results spreadsheet
v2 = np.array([0.0057994,0.00602738,0.00637176,0.00612049,0.00601458,0.006157405,0.00586526])
v1 = np.array([0.007433615,0.008317945,0.00990359,0.01179255,0.014124,0.01623365,0.0176299])

nodes = [0,1,2,4,6,8,10]

import matplotlib.pyplot as plt
plt.plot(nodes, v1 * 1000, marker='o')
plt.plot(nodes, v2 * 1000, marker='o')

plt.xlabel("DBMS Latency (ms)")
plt.ylim([0, 20])
plt.ylabel("Median Request Latency (ms)")
plt.title("Median Request Latency vs DBMS Latency")
plt.legend(["Cornus 1", "Cornus 2"], loc="best")
plt.show()