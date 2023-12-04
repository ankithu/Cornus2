import numpy as np

#just copied from results spreadsheet
v2_1 = np.array([0.004068265,0.003699785])
v2_2 = np.array([0.012882253,0.000785606])
v2_3 = np.array([0.000958377,0.026906])

v2_m = np.array([0.01798705,0.005975725])

nodes = ["Cornus 1", "Cornus 2"]
X_axis = np.arange(len(nodes)) 

import matplotlib.pyplot as plt
plt.bar(X_axis, v2_1 * 1000, color=(1, 0, 0, 1), edgecolor='black', label="Sending VoteReq")   
plt.bar(X_axis, v2_2 * 1000, bottom=(v2_1*1000), color=(1, 0, 1, 1),  edgecolor='black', label="Receiving Votes")
plt.bar(X_axis, v2_3 * 1000, bottom=(v2_1 + v2_2)*1000, color=(0, 0, 1, 1), edgecolor='black', label="Sending Decision")
plt.bar(X_axis, 0.6, width=0.9, bottom=(v2_m * 1000)-0.3, color=(0, 1, 0, 1), edgecolor='black', label="Client Latency End") 

plt.xticks(X_axis, nodes) 
plt.xlabel("Protocol Version")
plt.ylim([0, 32])
plt.ylabel("Median Transaction Latency (ms)")
plt.title("Transaction Latency Breakdown")
plt.legend()
plt.show()

