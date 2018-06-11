// -*- mode: c++ -*-

// Copyright 2009-2017 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2017, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

// preprocessor flag changed by ADC

#ifndef COMPONENTS_MERLIN_TOPOLOGY_ALL2ALL_H
#define COMPONENTS_MERLIN_TOPOLOGY_ALL2ALL_H

#include <sst/core/elementinfo.h>
#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/params.h>

#include <string.h>

#include "sst/elements/merlin/router.h"

// Because this is cribbed from mesh, I've retained all the uses of "dimensions,"
// even though I'm requiring it to be 1. -- ADC

namespace SST {
namespace Merlin {

class topo_all2all_event : public internal_router_event {
public:
    int dimensions;
    int routing_dim;
    int* dest_loc;

    topo_all2all_event() {}
    topo_all2all_event(int dim) {	dimensions = dim; routing_dim = 0; dest_loc = new int[dim]; }
    virtual ~topo_all2all_event() { delete[] dest_loc; }
    virtual internal_router_event* clone(void) override
    {
        topo_all2all_event* tte = new topo_all2all_event(*this);
        tte->dest_loc = new int[dimensions];
        memcpy(tte->dest_loc, dest_loc, dimensions*sizeof(int));
        return tte;
    }

    void serialize_order(SST::Core::Serialization::serializer &ser)  override {
        internal_router_event::serialize_order(ser);
        ser & dimensions;
        ser & routing_dim;

        if ( ser.mode() == SST::Core::Serialization::serializer::UNPACK ) {
            dest_loc = new int[dimensions];
        }

        for ( int i = 0 ; i < dimensions ; i++ ) {
            ser & dest_loc[i];
        }
    }

protected:

private:
    ImplementSerializable(SST::Merlin::topo_all2all_event)

};


class topo_all2all_init_event : public topo_all2all_event {
public:
    int phase;

    topo_all2all_init_event() {}
    topo_all2all_init_event(int dim) : topo_all2all_event(dim), phase(0) { }
    virtual ~topo_all2all_init_event() { }
    virtual internal_router_event* clone(void) override
    {
        topo_all2all_init_event* tte = new topo_all2all_init_event(*this);
        tte->dest_loc = new int[dimensions];
        memcpy(tte->dest_loc, dest_loc, dimensions*sizeof(int));
        return tte;
    }

    void serialize_order(SST::Core::Serialization::serializer &ser)  override {
        topo_all2all_event::serialize_order(ser);
        ser & phase;
    }

private:
    ImplementSerializable(SST::Merlin::topo_all2all_init_event)

};


class topo_all2all: public Topology {

public:

    SST_ELI_REGISTER_SUBCOMPONENT(
        topo_all2all,
        "merlin",
        "all2all",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "One-dimensional all2all topology object",
        "SST::Merlin::Topology")

    SST_ELI_DOCUMENT_PARAMS(
        {"all2all:shape",        "For all2all, only one dimension is allowed, so shape must be an integer in string format."},
        {"all2all:width",        "This will be 1 while Im' trying to get all2all working."},
        {"all2all:local_ports",  "Number of endpoints attached to each router."}
    )


private:
    int router_id;
    int* id_loc;

    int dimensions;
    int* dim_size;
    int* dim_width;

    int (* port_start)[2]; // port_start[dim][direction: 0=pos, 1=neg]

    int num_local_ports;
    int local_port_start;

public:
    topo_all2all(Component* comp, Params& params);
    ~topo_all2all();

    virtual void route(int port, int vc, internal_router_event* ev);
    virtual internal_router_event* process_input(RtrEvent* ev);

    virtual void routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts);
    virtual internal_router_event* process_InitData_input(RtrEvent* ev);

    virtual PortState getPortState(int port) const;
    virtual int computeNumVCs(int vns);
    virtual int getEndpointID(int port);

protected:
    virtual int choose_multipath(int start_port, int num_ports, int dest_dist);

private:
    void idToLocation(int id, int *location) const;
    void parseDimString(const std::string &shape, int *output) const;
    int get_dest_router(int dest_id) const;
    int get_dest_local_port(int dest_id) const;


};

}
}

#endif // COMPONENTS_MERLIN_TOPOLOGY_ALL2ALL_H
