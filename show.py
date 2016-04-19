import networkx as nx
import pygraphviz as pgv

if "__main__" == __name__:
    g = pgv.AGraph()
    path = r".\test-case\case3"
    for line in open(path + '\\topo.csv'):
        i, s, d, w = line.split(',')
        g.add_edge(s, d, label=w)
    for line in open(path + '\\demand.csv'):
        src, dst, inc = line.split(',')
        g.get_node(src).attr['color'] = 'red'
        g.get_node(dst).attr['color'] = 'red'
        for i in inc.split('|'):
            g.get_node(i).attr['color'] = 'blue'
    g.layout('dot')
    g.draw('1.svg')