#!/usr/bin/env python
#===============================================================================
# bfs-gen-graph.py [-n, -m, -s, -d]
#===============================================================================
#    
#  -h --help     Display this message
#  -v --verbose  Verbose mode
#   
#  This script generates a random connected graph. A graph of n nodes is
#  grown by attaching new nodes each with m edges that are preferentially
#  attached to existing nodes with high degree. 
#  
#  Command line options:
#    -n: number of nodes in graph
#    -m: degree of each edge
#    -s: optional integer seed
#    -d: visualize the generated data
#    
#  Author : Shreesha Srinath 
#  Date   : May 21, 2012
     
import argparse
import sys
import matplotlib.pyplot as plt
import networkx as nx
import random
     
#-------------------------------------------------------------------------------
# Command line processing
#-------------------------------------------------------------------------------
     
class ArgumentParserWithCustomError(argparse.ArgumentParser):
  def error( self, msg = "" ):
    if ( msg ): print("\n ERROR: %s" % msg)
    print("")
    file = open( sys.argv[0] )
    for ( lineno, line ) in enumerate( file ):
     if ( line[0] != '#' ): sys.exit(msg != "")
     if ( (lineno == 2) or (lineno >= 4) ): print( line[1:].rstrip("\n"))

def parse_cmdline():
  p = ArgumentParserWithCustomError( add_help=False )
  p.add_argument( "-v", "--verbose", action="store_true" )
  p.add_argument( "-h", "--help",    action="store_true" )
  p.add_argument( "-n", "--nodes",   action="store" )
  p.add_argument( "-m", "--edges",   action="store" )
  p.add_argument( "-s", "--seed",    action="store" )
  p.add_argument( "-d", "--disp",    action="store_true" )
  opts = p.parse_args()
  if opts.help: p.error()
  return opts

#-------------------------------------------------------------------------------
# Main
#-------------------------------------------------------------------------------

def main():
  opts = parse_cmdline()

  no_of_nodes = int(opts.nodes)
  no_of_edges = int(opts.edges)
  
  if( opts.seed ):
    seed = int(opts.seed)
  else: 
    seed = random.randint(1,100)

  display     = bool(opts.disp)

  # Generate a random base graph
  g= nx.barabasi_albert_graph(no_of_nodes,no_of_edges,seed)

  # Generate a weighted random graph
  g1 = nx.Graph()

  edge_labels = {}
  for e in g.edges_iter():
    #g1.add_edge(*e,weight=random.randint(1,10))
    g1.add_edge(*e,weight=1)

  if( display ):
    pos=nx.spring_layout(g1,weight='weight') 
    nx.draw(g1,pos)
    edge_labels=dict([((u,v,),d['weight']) for u,v,d in g1.edges(data=True)]) 
    nx.draw_networkx_edge_labels(g1,pos,edge_labels=edge_labels) 
    plt.savefig("bfs.png")

  # Write to Source File
  fp = open("bfs-src.dat", "w")
  
  i = 0
  total = 0
  
  fp.write("bfs::Node h_graph_nodes [] = {\n")
  for n in g1.nodes_iter():
    fp.write("  {%d,%d},\n"%(total,g1.degree(n)))
    i = i+1
    total=total+g1.degree(n)
  fp.write("};\n")

  fp.write("\nint no_of_nodes = %d;\n"%i)

  i = 0

  fp.write("\nbfs::Edge h_graph_edges [] = {\n")
  for n in g1.nodes_iter():
    for e in g1.edges_iter(n):
      data=g1.get_edge_data(*e)
      fp.write("  {%d,%d},\n"%(e[1],data.values()[0]))
      i = i+1
  fp.write("};\n")

  fp.write("\nint edges_list_size = %d;\n"%i)

  fp.write("\nint source = 0;\n")

  fp.close()

  # Write to Ref File
  fp = open("bfs-ref.dat", "w")

  dist=nx.single_source_shortest_path_length(g1,0)

  fp.write("int ref [] = {\n")
  for key,value in dist.items():
    fp.write("  %d,\n"%(value))
  fp.write("};\n")
  
  fp.close()


main()
