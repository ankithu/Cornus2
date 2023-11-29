from mininet.cli import CLI
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.log import setLogLevel
import json
from itertools import combinations
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath('__file__')), '..', 'Testing'))
from nodes import get_node_start_commands
from typing import List, Dict

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
        node_file = "nodes.json" #TODO use argument
        test_conf = "mininet_test1.json" #TODO use argument
        output_dir = "mininet_configs" #TODO use argument
        
        nodes = get_node_start_commands(node_file)

        # Add hosts and switches
        self.cornus_system_hosts = [(self.addHost(f'host{i}', ip = f'10.0.0.{i}'), cmd) for i, cmd in enumerate(nodes)]
        PORT = 8000
        self.dbms_host, self.dbms_command = self.cornus_system_hosts[0]
        ips = [f'10.0.0.{i}' for i in range(1, len(self.cornus_system_hosts))]

        all_hosts = [{"host_id":f'{ip}:{PORT}'} for ip in ips]


        for i, ip in enumerate(ips):
            gen_config(i, ip, PORT, all_hosts, f"10.0.0.0:{9000}", test_conf, f"{output_dir}/host{i}.config")
        

        s = self.addSwitch('s1')
        for host, _ in self.cornus_system_hosts:
            self.addLink(host, s)
            self.addLink(s, host)

        #add host for client
        self.client_host = self.addHost(f'host{len(self.cornus_system_hosts)}', ip=f'10.0.0.{len(self.cornus_system_hosts)}')
        self.addLink(self.client_host, s)
        self.addLink(s, self.client_host)


        
        #for host, command in hosts:


        # switches = []
        # for fr, to in combinations(hosts, r=2):
        #     fr_hs, fr_nm, _ = fr
        #     to_hs, to_nm, _ = to
            
        #     switches.append(self.addSwitch(f's{fr_nm}{to_nm}'))
        #     self.addLink(fr_hs, switches[-1])
        #     self.addLink(switches[-1], to_hs)
            


if __name__ == '__main__':
    setLogLevel( 'debug' )

    # Create data network
    topo = CornusTopology()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
           autoStaticArp=True)

    # Run network
    net.start()


    #for host in topo.cornus_system_hosts:



    CLI( net )
    net.stop()