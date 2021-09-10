import matplotlib.pyplot as plt
import networkx as nx
# from networkx import Graph
# import pydot
from networkx.drawing.nx_pydot import graphviz_layout
import sys

seed_inp = int(sys.argv[1])
name = sys.argv[2]

l=[]
# print("Enter num peers: ")
peer_id = 0
with open("./Visual/tree"+str(peer_id)+".txt", "r") as f:
    l = [(t[0],t[1]) for t in [x.strip().split(":") for x in f.readlines()]]

s= set([x[0] for x in l] + [x[1] for x in l])
s = list(s)
s.sort()
d = {s[i]: i for i in range(len(s))}
k = [i for i in range(len(s))]
l = [(d[x[0]], d[x[1]]) for x in l]

T = nx.DiGraph()
T.add_nodes_from(k)
T.add_edges_from(l)
tmppos = graphviz_layout(T, prog="dot", root = k[0])
pos = {node: (-t[1], t[0]) for node,t in tmppos.items()}

s = {i: {"name": s[i]} for i in range(len(s))}
nx.set_node_attributes(T, s)
nx.draw(T, pos, with_labels=True, labels = nx.get_node_attributes(T,'name'), node_size = 50, font_size = 2)
plt.savefig("./Visual/blockchains/"+name+".png", dpi = 400)
plt.clf()

