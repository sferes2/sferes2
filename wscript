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
    opt.add_option('--tests', type='string', help='compile tests or not', dest='tests')


    for i in modules:
        print 'options for module : [' + i + ']'
        opt.recurse(i)

    for i in glob.glob('exp/*'):
        print 'options for exp : [' + i + ']',
        try:
            opt.recurse(i)
            print 'ok'
        except:
            print 'none'


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
    if not conf.options.cpp11 or conf.options.cpp11 == 'yes':
        common_flags += '-std=c++11 '

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

    # sdl (optional)
    sdl = conf.check_cfg(package='sdl',
                   args='--cflags --libs',
                   msg="Checking for SDL (optional)",
                   uselib_store='SDL',
                   mandatory=False)
    if sdl: common_flags += '-DUSE_SDL '

    conf.env['CCDEFINES_SDL_gfx']=['_GNU_SOURCE=1', '_REENTRANT']
    conf.env['CPPPATH_SDL_gfx']=['/usr/include/SDL']
    conf.env['LIBPATH_SDL_gfx']=['/usr/lib']
    conf.env['CXXDEFINES_SDL_gfx']=['_GNU_SOURCE=1', '_REENTRANT']
    conf.env['LIB_SDL_gfx']=['SDL_gfx']
    conf.env['HAVE_SDL_gfx']=1

    conf.env['LIB_PTHREAD']=['pthread']


    # eigen 3 (optional)
    conf.load('eigen')
    conf.check_eigen()

    # ode (optional)
    conf.load('ode')
    conf.check_ode()

    # gsl (optional)
    conf.check_cfg(package='gsl',
                   args='--cflags --libs',
                   msg="Checking for GSL (optional)",
                   uselib_store='GSL',
                   mandatory=False)

    # bullet (optional)
    conf.env['LIB_BULLET'] = ['bulletdynamics', 'bulletcollision', 'bulletmath']
    if conf.options.bullet :
        conf.env['LIBPATH_BULLET'] = conf.options.bullet + '/lib'
        conf.env['CPPPATH_BULLET'] = conf.options.bullet + '/src'

    # osg (optional)
    conf.env['LIB_OSG'] = ['osg', 'osgDB', 'osgUtil',
                           'osgViewer', 'OpenThreads',
                           'osgFX', 'osgShadow']


    # Mac OS specific options
    if conf.options.apple and conf.options.apple == 'yes':
        common_flags += ' -Wno-gnu-static-float-init '

    conf.env['LIB_TCMALLOC'] = 'tcmalloc'
    conf.env['LIB_PTMALLOC'] = 'ptmalloc3'

    conf.env['LIB_EFENCE'] = 'efence'
    conf.env['LIB_BZIP2'] = 'bz2'
    conf.env['LIB_ZLIB'] = 'z'

    conf.env['LIBPATH_OPENGL'] = '/usr/X11R6/lib'
    conf.env['LIB_OPENGL'] = ['GL', 'GLU', 'glut']

    if conf.options.rpath:
        conf.env.append_value("LINKFLAGS", "--rpath="+conf.options.rpath)

    # modules
    for i in modules:
        print 'configuring module: ', i
        conf.recurse(i)

    if conf.options.exp:
        for i in conf.options.exp.split(','):
            print 'configuring for exp: ' + i
            conf.recurse('exp/' + i)

    # link flags
    if conf.options.libs:
        conf.env.append_value("LINKFLAGS", "-L" + conf.options.libs)

    if conf.options.includes :
        common_flags += " -I" + conf.options.includes + ' '
    if conf.env['MPI_ENABLED']:
        common_flags += '-DMPI_ENABLED '
    if not conf.env['TBB_ENABLED']:
        common_flags += '-DNO_PARALLEL '
    if conf.env['EIGEN_FOUND']:
        common_flags += '-DEIGEN3_ENABLED '

    common_flags += "-DSFERES_ROOT=\"" + os.getcwd() + "\" "

    conf.env['CXXFLAGS'] = common_flags.split(' ')
    conf.env['SFERES_ROOT'] = os.getcwd()

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
    waf_unit_test.summary(bld)
    # if tfail > 0:
    #     bld.fatal("Build failed! Some tests failed!")

def build(bld):
    v = commands.getoutput('git rev-parse HEAD')
    bld.env['CXXFLAGS'].append("-DVERSION=\"(const char*)\\\""+v+"\\\"\"")

    if bld.options.debug:
        bld.env['CXXFLAGS'] += debug_flags.split(' ')
    else:
        bld.env['CXXFLAGS'] += opt_flags.split(' ')

    print ("Entering directory `" + os.getcwd() + "'")
    bld.recurse('sferes examples')
    if bld.options.tests and bld.options.tests == 'yes':
        bld.recurse('tests')
    if bld.options.exp:
        for i in bld.options.exp.split(','):
            print 'Building exp: ' + i
            bld.recurse('exp/' + i)
    for i in modules:
        print 'Building module: ' + i
        bld.recurse(i)

    bld.add_post_fun(summary)

def shutdown (ctx):
    if ctx.options.create_exp:
        sferes.create_exp(ctx.options.create_exp)
    if ctx.options.qsub:
        sferes.qsub(ctx.options.qsub)
    if ctx.options.oar:
        sferes.oar(ctx.options.oar)
