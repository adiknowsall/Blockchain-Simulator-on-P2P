import networkx as nx
import random
import matplotlib.pyplot as plt
import sys

# print("Enter seed:")
seed_inp = int(sys.argv[1])
random.seed(seed_inp)

def gen(m,M, n):
    global seed_inp
    tmp = []
    for i in range(n):
        tmp.append(random.randint(m,M))
    return tmp

n = int(sys.argv[2])
min_degree = max(2,int(n/6))
max_degree = min(n-1, int(n/2.5))
if(max_degree<min_degree):
    max_degree = min_degree
# print(min_degree)
# print(max_degree)
seq = gen(min_degree, max_degree, n)
while(not nx.is_graphical(seq)):
    # print(seq)
    seq = gen(min_degree, max_degree, n)

# print(seq)
G = nx.random_degree_sequence_graph(seq, seed=seed_inp)
# pos = nx.spring_layout(G, seed=seed_inp)
# print(G)
# print(pos)
# nx.draw(G, pos)
# plt.show()
# print("lol")
for e in G.edges:
    print("{} {}".format(e[0],e[1]))
# print(G)