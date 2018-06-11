# This code doesn't work.  It bombs with a traceback.  Still working on it ...
#
# --- Andrew D. Culhane, Sagecor Solutions, aculhane@sagecor.com

# This will eventually simulate a full mesh, aka an all-to-all network.  The
# Python code is partly cribbed from the script pymerlin.py.

import sst

# The mapping function is used to connect routers to one another.

# Each of N routers has N-1 ports, to connect to all N-1 of the other routers.
# Routers don't connect to themselves.

# So if j<i, then i connects to j on its own jth port.
# But if j>i, then i connects to j on its own (j-1)th port.

# Routers are number 0 to N-1.  The router-to-router connections take up
# ports 0 through N-2 on all of the routers.  Port N-1 connects to the
# endpoint.

def mapping (i,j):
    if j<i :
        return j
    elif j==i :
        print 'Error.  No router connects to itself.'
        return -1
    else :
        return j-1


# Create dictionaries of default parameters.

default_rtr_parameters = dict() ; default_ept_parameters = dict()

N = input('How many nodes in this all-to-all network? ')

# The parameter dictionaries below are cribbed directly from merlin and from
# an SST tutorial I studied.

default_rtr_parameters = {'topology' : 'merlin.all2all',
                          'all2all:shape' : str(N),
                          'all2all:width' : '1'
                          'all2all:local_ports' : 1,
                          'input_buf_size' : '2KB',
                          'output_buf_size': '2KB',
                          'input_latency' : '25ps',
                          'output_latency' : '25ps',
                          'xbar_bw' : '51.2GB/s',
                          'link_bw' : '25.6GB/s',
                          'flit_size' : '72B',
                          'num_ports' : N,
                          'debug' : 0,
#                          'xbar_arg' : 'merlin.xbar_arb_lru',
#                          'checkerboard' : '1',
#                          'num_messages' : str(N),
                          'num_peers' : N}

default_ept_parameters = default_rtr_parameters.copy()

# The copy gives it parameters it doesn't need, but SST should (I think) just
# ignore those.

default_latency = '25ps'

# Create the routers and endpoints.  Keep lists of both.

routers = list() ; endpoints = list()

for i in range(N) :

    rtrName = 'rtr.'+str(i)
    rtr = sst.Component(rtrName,'merlin.hr_router')
    rtr.addParams (default_rtr_parameters)
    rtr.addParam ('id',i)
    routers.append(rtr)

    eptName = 'ept.'+str(i)
    ept = sst.Component(eptName,'merlin.test_nic')
    ept.addParams (default_ept_parameters)      # stats line doesn't work
    ept.addParam('id',i)
    endpoints.append(ept)

# Create the links.

# I've tried multiple approaches to the links.  Since they appear to be
# bidirectional in the mesh topology, I'm assuming they are.

# I will end up with a total of N(N+1)/2 links.  That comes from:

# N links connecting each router to its endpoint
# N-1 links connecting router 0 to all the others
# N-2 links connecting router 1 to all the others but router 0 (already done)
# ...
# 1 link connecting router N-2 to router N-1

# That's the sum of 1 + 2 + ... + N-1 + N.  That equals N(N+1)/2.

# My link naming conventions below are cribbed from mesh.  I don't think the
# names matter.

for i in range(N) :             # loop on routers
                                # start by connnecting nic
    nicLinkName = 'nic'+str(i)+':0'
    nLink = sst.Link(nicLinkName)
    nLink.connect ((endpoints[i],'rtr',default_latency),
                                # next generate links to other routers
    for j in range(i+1,N) :
        rtrLinkName = 'rtr.'+str(i)+':'+str(j)+':0'
        outbound = mapping(i,j) ; inbound = mapping(j,i)
        rLink = sst.Link(rtrLinkName)
        rLink.connect ((routers[i],'port%d'%outbound,default_latency),
                        (routers[j],'port%d'%inbound,default_latency))

# a few lines about stats.  These aren't really even relevant yet, since
# the simulation bombs during initialization.

statLevel = 16      # cribbed again
sst.setStatisticLoadLevel(statLevel)
sst.enableAllStatisticsForAllComponents(['type':'sst.AccumulatorStatistic'})
