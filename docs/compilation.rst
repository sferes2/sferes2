Compilation
=============

Dependencies
---------------


Ubuntu / Debian
^^^^^^^^^^^^^^^^

We list here the Ubuntu package names:

-  [required] libboost-all-dev >= 1.35 (including test, serialization, graph,
  program_options, filesystem); see [[http://www.boost.org]]; >
   http://www.boost.org;
-  [required] python >= 3.x
-  [required] g++ >= 4.2 or clang (defaut on mac);
-  [optional but highly recommended] libtbb-dev; see
   http://www.threadingbuildingblocks.org/
-  [optional but highly recommended] libeigen3-dev ; see
   http://eigen.tuxfamily.org/
-  [optional but recommended] python-simplejson;
-  [optional] libboost-mpi-dev , if you need the optional MPI support
   (Warning: the Ubuntu packages of boost are not compiled with MPI
   support; you will have to compile boost yourself to use MPI).
-  [optional] OpenMPI or another MPI implementation (tested with IBM MPI
   and OpenMPI)
-  [optional] libgoogle-perftools-dev.

.. code:: shell

    sudo apt-get install libboost-dev libboost-test-dev libboost-filesystem-dev libboost-program-options-dev libboost-graph-parallel-dev python g++ libtbb-dev libeigen3-dev python-simplejson libgoogle-perftools-dev

OSX (brew)
^^^^^^^^^^

To install MPI support with brew:

.. code:: shell

    brew -v install  boost --with-mpi --without-single

Compilation
---------------

Simple compilation
^^^^^^^^^^^^^^^^^^^^

Assuming that everything has been properly installed you should be able
to compile sferes2 by typing (in the main sferes directory):

.. code:: shell

    ./waf configure
    ./waf build

You should then run the unit tests:

.. code:: shell

    ./waf --tests

If everything is green, sferes2 is working! If not, please send an
e-mail to the authors with as many details as possible about your
problem.

Configuration and details
^^^^^^^^^^^^^^^^^^^^^^^^^

Sferes2 employs the waf build system (http://code.google.com/p/waf/)
instead of autoconf/automake and make. Waf employs files named *wscript*
to describe the compilation process in python.

The main wscript of sferes2 accepts the following configuration options:

-  –-boost-includes=BOOSTINCLUDES : path to the boost directory where the
   includes are e.g. /usr/local/include/boost-1_35`
-  –-boost-libs=BOOSTLIBS: path to the directory where the boost libs are
   e.g. /usr/local/lib
-  --eigen=EIGEN: path to eigen (if not in the default directory)
-  –-no-mpi: disable mpi (MPI is automatically disabled if no MPI
   implementation is found)
- --cpp14=CPP14: force / disable c++-14 compilation
-  –-mpi=MPI: path to MPI [e.g. /usr/lib/openmpi ]
-  –-tbb=TBB: path to TBB if it's not installed in the default directory
-  –-rpath: set an optional rpath (useful in case of link failure)
-  --debug=yes/no: compile with debugging symbols

To use these options, pass them to the waf script during the
configuration step. For example:

.. code:: shell

    ./waf configure --mpi /usr/lib/openmpi/

or, to disable MPI:

.. code:: shell

    ./waf configure --no-mpi

The configure options used during the last call of the "configure"
command are in the file build/configure.options.

Optional modules
--------

The previous compilation provided you with a minimal sferes install.
Several modules are available here: https://github.com/sferes2 

Before using any of these modules in your experiment, you need to clone these
repositories in the 'modules' directory of sferes.

Then, create a file named "modules.conf" at the root of sferes
install (where waf is). You should then list the modules you want to use
in this file, with one module per line.

For example, if you want to use the modules cartpole and nn2, your
modules.conf file should look like:

.. code:: shell

    cartpole
    nn2

You should then recompile sferes with the updated file :

.. code:: shell

    ./waf build

This should compile all the add-on modules.


You can then run the tests by using waf check:

.. code:: shell

    ./waf check

