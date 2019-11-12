import igraph
from igraph import *
file = open("datos.txt","r")
lab=""
conta=0
g = Graph.Tree(1,2)
data = file.readlines()
for line in data:
    g.add_vertices(1)
    g.vs[conta]["label"]=line
    conta+=1

for i in range(int(conta/2)-1):
    g.add_edges([(i,(i+1)*2)])
    g.add_edges([(i,((i+1)*2)-1)])

if(((int(conta/2)-1)*2) +1 < conta):
    g.add_edges([(int(conta/2),int(conta-1))])

layout = g.layout('rt')
plot(g,layout=layout,bbox=(700,700))
