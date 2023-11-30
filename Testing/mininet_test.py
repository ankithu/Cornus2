from mininet.cli import CLI
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.log import setLogLevel, info
from mininet.util import pmonitor
import json
from itertools import combinations
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath('__file__')), '..', 'Testing'))
from nodes import get_node_start_commands
from typing import List, Dict
from threading import Thread
from time import sleep

NODE_FILE = "mininet_nodes.json" #TODO use argument
TEST_CONF = "mininet_test1.json" #TODO use argument
CONF_OUTPUT_DIR = "mininet_configs" #TODO use argument
TEST_FILE = "test1.json"
LINK_DELAY_MS = 10


def gen_config(idx: int, host: str, port: int, all_hosts: List[Dict[str, str]], dbms_address: str, test_conf_file_name: str, output_file: str) -> None:
    with open(test_conf_file_name, 'r') as file:
        conf = json.load(file)
    d = dict()
    d["f"] = conf["f"]
    d["dbms_address"] = dbms_address
    d["timeout_millis"] = conf["timeout_millis"]
    d["host"] = host
    d["port"] = port
    d["host_num"] = idx
    d["others"] = [h for h in all_hosts if h["host_id"] != f"{host}:{port}"]
    with open(output_file, 'w') as file:
        json.dump(d, file)



class CornusTopology( Topo ):

    def __init__( self, **opts ):
        Topo.__init__(self, **opts)

        
        nodes = get_node_start_commands(NODE_FILE)

        # Add hosts and switches
        self.cornus_system_hosts = [(self.addHost(f'host{i}', ip = f'10.0.0.{i}'), cmd) for i, cmd in enumerate(nodes)]
        PORT = 8000
        self.dbms_host, self.dbms_command = self.cornus_system_hosts[0]
        ips = [f'10.0.0.{i}' for i in range(1, len(self.cornus_system_hosts))]

        all_hosts = [{"host_id":f'{ip}:{PORT}'} for ip in ips]


        for i, ip in enumerate(ips):
            gen_config(i, ip, PORT, all_hosts, f"10.0.0.0:{9000}", TEST_CONF, f"{CONF_OUTPUT_DIR}/host{i}.config")
        

        s = self.addSwitch('s1')
        for host, _ in self.cornus_system_hosts:
            self.addLink(host, s, delay=LINK_DELAY_MS, bw=1000)

        #add host for client
        self.client_host = self.addHost(f'host{len(self.cornus_system_hosts)}', ip=f'10.0.0.{len(self.cornus_system_hosts)}')
        self.addLink(self.client_host, s, delay=LINK_DELAY_MS, bw=1000)
            

if __name__ == '__main__':
    setLogLevel( 'info' )

    # Create data network
    topo = CornusTopology()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
           autoStaticArp=True)

    # Run network
    net.start()

    #start dbms
    dbms_node = net.get("host0")
    popens = {}
    info(topo.dbms_command)
    popens[dbms_node] = dbms_node.popen(f'{topo.dbms_command} 10.0.0.0')

    #start cornus nodes
    for i in range(1, len(topo.cornus_system_hosts)):
        cornus_node = net.get(f"host{i}")
        popens[cornus_node] = cornus_node.popen(f'{topo.cornus_system_hosts[i][1]}')

    #CLI(net)

    client_node = net.get(f"host{len(topo.cornus_system_hosts)}")
    #client_node.startShell()
    popens[client_node] = client_node.popen(f'python3 -u client.py {TEST_FILE} {NODE_FILE} -c')
    
    for host, line in pmonitor( popens ):
        if host:
            info( "<%s>: %s" % ( host.name, line ) )

    CLI( net )
    net.stop()