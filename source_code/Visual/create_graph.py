import matplotlib.pyplot as plt
import networkx as nx
from networkx import Graph
import pydot
from networkx.drawing.nx_pydot import graphviz_layout
import sys


seed_inp = int(sys.argv[1])
name = sys.argv[2]

l=[]
with open("./Visual/network.txt", "r") as f:
    l = [x.strip() for x in f.readlines()]

n = int(l[0])
l = l[1:]
nodes_attr = [(i, {"fast": "F"+str(i) if int(l[i])==1 else "S"+str(i)}) for i in range(n)]
tuple_list = [(int(x[0]),int(x[1]),float(x[2])) for x in [x.split(",") for x in l[n+1:]]]

G = nx.Graph()
G.add_nodes_from(nodes_attr)
G.add_weighted_edges_from(tuple_list)

labels = dict([((u,v,),d['weight'])
            for u,v,d in G.edges(data=True)])
node_labels = nx.get_node_attributes(G, 'fast') 
# labels = nx.get_edge_attributes(G,'weight')
pos = nx.spring_layout(G, seed=seed_inp)

nx.draw(G, pos, node_size = 70, width = [0.2]*len(labels))
nx.draw_networkx_labels(G, pos, font_size = 4, labels = node_labels)
# nx.draw_networkx_edge_labels(G, pos,  edge_labels=labels, label_pos=0.3, font_size=3)
plt.savefig("./Visual/networks/"+name, dpi = 400)
plt.clf()
# plt.show()