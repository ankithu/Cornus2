import numpy as np

#just copied from results spreadsheet
v2_1 = np.array([0.00222901,0.002710745,0.0031047,0.003699785])
v1_1 = np.array([0.002301035,0.002417575,0.003403315,0.004068265])
v2_2 = np.array([0.0008047775,0.000801056,0.000772762,0.000785606])
v1_2 = np.array([0.0129553885,0.012916153,0.012893124,0.012882253])
v2_3 = np.array([0.0263873,0.0264423,0.02664575,0.026906])
v1_3 = np.array([0.0008946715,0.000897472,0.0009401675,0.000958377])

v2_m = np.array([0.00394347,0.00458876,0.005261685,0.005975725])
v1_m = np.array([0.0161815,0.01639775,0.0173035,0.01798705])

nodes = [3,4,6,8]
X_axis = np.arange(len(nodes)) 

import matplotlib.pyplot as plt
plt.bar(X_axis - 0.2, v1_1 * 1000, 0.4, color=(1, 0, 0, 0.4), hatch="//", edgecolor='black', label="Cornus 1, Sending VoteReq") 
plt.bar(X_axis - 0.2, v1_2 * 1000, 0.4,bottom=(v1_1*1000), color=(1, 0, 0, 0.7), hatch="--", edgecolor='black', label="Cornus 1, Receiving Votes") 
plt.bar(X_axis - 0.2, v1_3 * 1000, 0.4,bottom=(v1_1 + v1_2 )*1000, color=(1, 0, 0, 1), hatch="xx", edgecolor='black', label="Cornus 1, Sending Decision")
plt.bar(X_axis + 0.2, v2_1 * 1000, 0.4, color=(0, 0, 1, 0.4), hatch="//", edgecolor='black', label="Cornus 2, Sending VoteReq")   
plt.bar(X_axis + 0.2, v2_2 * 1000, 0.4, bottom=(v2_1*1000), color=(0, 0, 1, 0.7), hatch="--", edgecolor='black', label="Cornus 2, Receiving Votes")
plt.bar(X_axis + 0.2, v2_3 * 1000, 0.4, bottom=(v2_1 + v2_2)*1000, color=(0, 0, 1, 1), hatch="xx", edgecolor='black', label="Cornus 2, Sending Decision")
plt.bar(X_axis - 0.2, 0.6, 0.4, bottom=(v1_m * 1000)-0.3, color=(0, 1, 0, 1), edgecolor='black', label="Client Latency End") 
plt.bar(X_axis + 0.2, 0.6, 0.4, bottom=(v2_m * 1000)-0.3, color=(0, 1, 0, 1), edgecolor='black')  

plt.xticks(X_axis, nodes) 
plt.xlabel("Number of Nodes")
plt.ylim([0, 50])
plt.ylabel("Median Transaction Latency (ms)")
plt.title("Transaction Latency Breakdown")
plt.legend()
plt.show()

