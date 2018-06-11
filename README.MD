The Contents:

- all2all.py, linkControl.cc, Makefile, Makefile.am, and Makefile.in, which go in the merlin directory

- all2all.cc and all2all.h, which go in the topology subdirectory of merlin

--------------

Background:

I'm trying to add an all2all topology to merlin.  I'm using sst version 7.2.0, with openmpi 1.8.8.  The
platform is my account on pythonanywhere.

         NOTE: I tried installing sst version 8.0.0, with openmpi 2.1.3.
         I got an entirely different error, which appeared to be from
         openmpi itself.  In the spirit of One Thing At A Time, I want
         to address my current problem first.

With the above files copied into the appropriate directories in sst, I remake the elements portion of sst,
by following just the elements part of the detailed installation instructions.  I have that as a script
called redoElements.sh, which looks like this (/home/drculhane is the home directory on my account):

# SST_ELEMENTS_HOME and SST_CORE_HOME are defined in .bashrc as specified in install instructions
cd /home/drculhane/scratch/sst-elements-library-7.2.0
./configure --prefix=$SST_ELEMENTS_HOME --with-sst-core=$SST_CORE_HOME ; wait
make all ; wait
make install

--------------

The Error:

When I run my all2all.py configuration script, I get a traceback.  The error occurs after lines 162 and 163 of
linkControl.cc, which are:

ev = rtr_link->recvInitData();
init_ev = dynamic_cast<RtrInitEvent*>(ev);

After experiencing the traceback a few times, I added a check immediately after these lines to see if init_ev was NULL.
It is, as shown below:

sst all2all.py

How many nodes in this all-to-all network? 5
Inside LinkControl::init with phase = 0
Inside LinkControl::init with phase = 0
Inside LinkControl::init with phase = 0
Inside LinkControl::init with phase = 0
Inside LinkControl::init with phase = 0
Inside LinkControl::init with phase = 1
NULL ptr as init_ev.  The next line of code will fail.
[giles-liveconsole2:13848] *** Process received signal ***
[giles-liveconsole2:13848] Signal: Segmentation fault (11)
[giles-liveconsole2:13848] Signal code: Address not mapped (1)
[giles-liveconsole2:13848] Failing at address: 0x50
[giles-liveconsole2:13848] [ 0] /lib/x86_64-linux-gnu/libpthread.so.0(+0x10330)[0x7ff1633ee330]
[giles-liveconsole2:13848] [ 1] sst(_ZNK3SST5UnitseqERKS0_+0xa)[0x4c9d2a]
[giles-liveconsole2:13848] [ 2] sst(_ZNK3SST11UnitAlgebragtERKS0_+0x22)[0x4cb8d2]
[giles-liveconsole2:13848] [ 3] /home/drculhane/local/sstelements-7.2.0/lib/sst-elements-library/libmerlin.so(_ZN3SST6Merlin11LinkControl4initEj+0x9d)[0x7ff15cb4be9d]
[giles-liveconsole2:13848] [ 4] /home/drculhane/local/sstelements-7.2.0/lib/sst-elements-library/libmerlin.so(_ZN3SST6Merlin3nic4initEj+0x15)[0x7ff15cb618c5]
[giles-liveconsole2:13848] [ 5] sst(_ZN3SST10Simulation10initializeEv+0x64)[0x4b9714]
[giles-liveconsole2:13848] [ 6] sst[0x462b8a]
[giles-liveconsole2:13848] [ 7] sst(main+0xeeb)[0x4562ab]
[giles-liveconsole2:13848] [ 8] /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf5)[0x7ff1619a6f45]
[giles-liveconsole2:13848] [ 9] sst[0x4625de]
[giles-liveconsole2:13848] *** End of error message ***
Segmentation fault

This always happens during the first use of LinkControl::init in phase 1, regardless of how many nodes
I tell the script to instantiate.

It's obvious that the event list which recvInitData() pulls from is empty.  But I don't see why other
topologies fill it, and mine doesn't (I've run tests with the mesh topology, with no problems).

---------------

About the configuration script:

I'm creating a network of N merlin.hr_routers, each with 1 merlin.test_nic connected to it.

Because the topology is all to all, each router will have exactly N links:

    1 to its own test_nic
    N-1 to all of the other routers

Because the router-to-router links are assumed to be bidirectional (they seem to be in the mesh
topology), there's no need to create both 2to3 and 3to2.  So the total number of separate links
will be:

    N rtr to nic links
    N-1 rtr to rtr links for router 0
    N-2 rtr to rtr links for router 1
    N-3 rtr to rtr links for router 2
    ...
    1 rtr to rtr link for router N-2
    0 rtr to rtr links for router N-1

The sum is 1+2+...+(N-1)+N = (N)(N+1)/2

-----------------

About port numbering:

Because mesh used the highest port number to connect to the nic, I'm doing the same.

    For every router, port # N-1 connects to the 'rtr' port of the nic.

That leaves me N-2 ports to connect to the routers:

    if j<i, then router i connects to router j through i's port j
    if j>i, then router i connects to router j through i's port j-1

NOTE: I tried connecting port i to the nic (so it was 0 on rtr 0, 1 on rtr 1, etc.),
but I was getting the same error I'm still getting.  I changed to this approach,
just in case the "connect nic to highest port number" as done in mesh was required.