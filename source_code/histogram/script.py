l = []

with open("./lists.txt", "r") as f:
    l = f.readlines()


c = [l[i][l[i].find('[')+1:l[i].find(']')] for i in range(len(l))]
c = [x.split(",") for x in c]
c = [[int(y.strip()) for y in x if y != ' '] for x in c]

from matplotlib import pyplot as plt
import numpy as np

i = 0

a=np.array([1169, 4998, 3178, ]
)

print(len(a))
# Creating histogram
fig, ax = plt.subplots(1,1, figsize =(10, 7))
ax.hist(a, bins = 50)

# Show plot
plt.savefig("plot"+str(i)+".png")
plt.clf()
# i+=1