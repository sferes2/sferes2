#! /usr/bin/env python
#| This file is a part of the sferes2 framework.
#| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC)
#| Main contributor(s): Jean-Baptiste Mouret, mouret@isir.fr
#|
#| This software is a computer program whose purpose is to facilitate
#| experiments in evolutionary computation and evolutionary robotics.
#|
#| This software is governed by the CeCILL license under French law
#| and abiding by the rules of distribution of free software.  You
#| can use, modify and/ or redistribute the software under the terms
#| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
#| following URL "http://www.cecill.info".
#|
#| As a counterpart to the access to the source code and rights to
#| copy, modify and redistribute granted by the license, users are
#| provided only with a limited warranty and the software's author,
#| the holder of the economic rights, and the successive licensors
#| have only limited liability.
#|
#| In this respect, the user's attention is drawn to the risks
#| associated with loading, using, modifying and/or developing or
#| reproducing the software by the user in light of its specific
#| status of free software, that may mean that it is complicated to
#| manipulate, and that also therefore means that it is reserved for
#| developers and experienced professionals having in-depth computer
#| knowledge. Users are therefore encouraged to load and test the
#| software's suitability as regards their requirements in conditions
#| enabling the security of their systems and/or data to be ensured
#| and, more generally, to use and operate it in the same conditions
#| as regards security.
#|
#| The fact that you are presently reading this means that you have
#| had knowledge of the CeCILL license and that you accept its terms.


import sys
sys.path.insert(0, './waf_tools')
import commands

VERSION=commands.getoutput('git rev-parse HEAD')
if "No such " in VERSION or "fatal:" in VERSION:
    VERSION="0.x"
APPNAME='sferes2'

srcdir = '.'
blddir = 'build'

import copy
import os, glob, types
import sferes
from waflib.Build import BuildContext
from waflib.Tools import waf_unit_test
from waflib import Logs
modules = sferes.parse_modules()

opt_flags = '-O3 -DNDEBUG'
debug_flags = '-O0 -ggdb3 -DDBG_ENABLED'

def options(opt):
    # tools
    opt.load('compiler_cxx boost waf_unit_test')
    opt.load('tbb')
    opt.load('mpi')
    opt.load('eigen')

    # sferes specific
    opt.add_option('--bullet', type='string', help='path to bullet', dest='bullet')
    opt.add_option('--apple', type='string', help='enable apple support', dest='apple')
    opt.add_option('--rpath', type='string', help='set rpath', dest='rpath')
    opt.add_option('--includes', type='string', help='add an include path, e.g. /home/mandor/include', dest='includes')
    opt.add_option('--libs', type='string', help='add a lib path, e.g. /home/mandor/lib', dest='libs')
    opt.add_option('--cpp11', type='string', help='force c++-11 compilation [--cpp11=yes]', dest='cpp11')

    # exp commands
    opt.add_option('--create', type='string', help='create a new exp', dest='create_exp')
    opt.add_option('--exp', type='string', help='exp to build', dest='exp')
    opt.add_option('--qsub', type='string', help='config file (json) to submit to torque', dest='qsub')
    opt.add_option('--oar', type='string', help='config file (json) to submit to oar', dest='oar')

    # debug flags
    opt.add_option('--debug', type='string', help='compile with debugging symbols', dest='debug')

    # tests
    opt.add_option('--tests',  action='store_true', default=False, help='compile tests or not', dest='tests')
    opt.add_option('--tests-verbose', action='store_true', default=False, help='display the output of the unit tests', dest='tests_verbose')

    opt.logger = Logs.make_logger(blddir + 'options.log', 'mylogger')

    for i in modules:
        opt.start_msg('Command-line options for module [%s]' % i)
        try:
            opt.recurse(i)
            opt.end_msg('OK')
        except:
            opt.end_msg(' -> no option found', 'YELLOW')

    for i in glob.glob('exp/*'):
        opt.start_msg('Command-line options for exp [%s]' % i)
        try:
            opt.recurse(i)
            opt.end_msg(' -> OK')
        except:
            opt.end_msg(' -> no option found', 'YELLOW')


def configure(conf):
    # log configure options
    fname = blddir + '/configure.options'
    args = open(fname, 'a')
    for i in sys.argv:
        args.write(i + ' ')
    args.write("\n")
    args.close()

    conf.load('compiler_cxx')

    common_flags = "-D_REENTRANT -Wall -fPIC -ftemplate-depth-1024 -Wno-sign-compare -Wno-deprecated  -Wno-unused "
    common_flags += "-DSFERES_ROOT=\"" + os.getcwd() + "\" "
    conf.env['CXXFLAGS'] += common_flags.split(' ')

    conf.env['SFERES_ROOT'] = os.getcwd()

    # link flags
    if conf.options.libs:
        conf.env.append_value("LINKFLAGS", "-L" + conf.options.libs)



    # boost
    conf.load('boost')
    conf.check_boost(lib='serialization filesystem system unit_test_framework program_options graph mpi thread regex',
                     min_version='1.35')
    # tbb
    conf.load('tbb')
    conf.check_tbb()

    # mpi.h
    conf.check_mpi()

    # boost mpi
    if (len(conf.env['LIB_BOOST_MPI']) != 0 and conf.env['MPI_FOUND']):
        conf.env['MPI_ENABLED'] = True
    else:
        conf.env['MPI_ENABLED'] = False

    # eigen 3 (optional)
    conf.load('eigen')
    conf.check_eigen()

    if conf.options.rpath:
        conf.env.append_value("LINKFLAGS", "--rpath="+conf.options.rpath)

    # modules
    for i in modules:
        Logs.info('Configuring for module: [%s]' % i),
        try:
            conf.recurse(i)
            Logs.info('%s -> ok' % i)
        except:
            Logs.warn(' %s -> no configuration found' % i, 'YELLOW')

    if conf.options.exp:
        for i in conf.options.exp.split(','):
            Logs.info('Configuring for exp [%s]' %i)
            try:
                conf.recurse('exp/' + i)
                Logs.info('%s -> ok' % i)
            except:
                Logs.warn('%s -> no configuration found' % i, 'YELLOW')


    if not conf.options.cpp11 or conf.options.cpp11 == 'yes':
        conf.env['CXXFLAGS']  += ['-std=c++11']
    if conf.options.includes :
        conf.env['CXXFLAGS']  += ["-I" + conf.options.includes]
    if conf.env['MPI_ENABLED']:
        conf.env['CXXFLAGS']  += ['-DMPI_ENABLED']
    if not conf.env['TBB_ENABLED']:
        conf.env['CXXFLAGS']  += ['-DNO_PARALLEL']
    if conf.env['EIGEN_FOUND']:
        conf.env['CXXFLAGS']  += ['-DEIGEN3_ENABLED']

    # display flags
    def flat(list) :
        str = ""
        for i in list :
            str += i + ' '
        return str
    print '\n--- configuration ---'
    print 'compiler(s):'
    print' * CXX: ' + str(conf.env['CXX_NAME'])
    print 'boost version: ' + str(conf.env['BOOST_VERSION'])
    print 'mpi: ' + str(conf.env['MPI_ENABLED'])
    print "Compilation flags :"
    print "   CXXFLAGS : " + flat(conf.env['CXXFLAGS'])
    print "   LINKFLAGS: " + flat(conf.env['LINKFLAGS'])
    print "--- license ---"
    print "Sferes2 is distributed under the CECILL license (GPL-compatible)"
    print "Please check the accompagnying COPYING file or http://www.cecill.info/"

def summary(bld):
    lst = getattr(bld, 'utest_results', [])
    total = 0
    tfail = 0
    if lst:
        total = len(lst)
        tfail = len([x for x in lst if x[1]])
    failed_lines = []
    for (f, code, out, err) in lst:
        output = str(out).splitlines()
        for line in output:
            if ' failed' in line:
                failed_lines += [line]
            if bld.options.tests_verbose and ' passed' in line:
                Logs.info(line)
            elif bld.options.tests_verbose and ' failed' in line:
                Logs.error(line)
            elif bld.options.tests_verbose :
                print line
    waf_unit_test.summary(bld)
    if tfail > 0:
        Logs.error(str(tfail) + "/" + str(total) + " tests failed:")
        for i in failed_lines:
            Logs.error(i)
        bld.fatal("Build failed, because some tests failed!")



def build(bld):
    v = commands.getoutput('git rev-parse HEAD')
    bld.env['CXXFLAGS'].append("-DVERSION=\"(const char*)\\\""+v+"\\\"\"")

    if bld.options.debug:
        bld.env['CXXFLAGS'] += debug_flags.split(' ')
    else:
        bld.env['CXXFLAGS'] += opt_flags.split(' ')

    print ("Entering directory `" + os.getcwd() + "'")
    bld.recurse('sferes examples')
    if bld.options.tests:
        bld.recurse('tests')
    if bld.options.exp:
        for i in bld.options.exp.split(','):
            Logs.info('Building exp: ' + i)
            bld.recurse('exp/' + i)
    for i in modules:
        Logs.info('Building module: ' + i)
        bld.recurse(i)

    bld.add_post_fun(summary)

def shutdown (ctx):
    if ctx.options.create_exp:
        sferes.create_exp(ctx.options.create_exp)
    if ctx.options.qsub:
        sferes.qsub(ctx.options.qsub)
    if ctx.options.oar:
        sferes.oar(ctx.options.oar)
