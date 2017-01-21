FROM ubuntu:16.04
RUN apt-get -y update
RUN apt-get install -y libboost-dev
RUN apt-get install -y libboost-test-dev
RUN apt-get install -y libboost-filesystem-dev
RUN apt-get install -y libboost-program-options-dev
RUN apt-get install -y libboost-graph-parallel-dev
RUN apt-get install -y libboost-thread-dev
RUN apt-get install -y libboost-regex-dev
RUN apt-get install -y python
RUN apt-get install -y g++
RUN apt-get install -y libeigen3-dev
RUN apt-get install -y python-simplejson
RUN apt-get install -y libboost-mpi-dev
RUN apt-get install -y openmpi-common
RUN apt-get install -y openmpi-bin
RUN apt-get install -y openmpi-bin
RUN apt-get install -y libgoogle-perftools-dev
RUN apt-get install -y git
RUN apt-get install -y wget
RUN mkdir /deps && cd /deps/ && wget https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb2017_20161128oss_src.tgz && tar zxvf tbb2017_20161128oss_src.tgz && cd tbb2017_20161128oss && make && ln -s build/linux_intel64_gcc_cc5.4.0_libc2.23_kernel3.13.0_release lib
ENV LD_LIBRARY_PATH /deps/tbb2017_20161128oss/lib
RUN mkdir /git && cd /git && git clone https://github.com/sferes2/sferes2.git && cd sferes2 && ./waf configure --tbb=/deps/tbb2017_20161128oss && ./waf build

