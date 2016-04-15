#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty mpi detection
"""

import os, glob, types
from waflib.Configure import conf

def options(opt):
	opt.add_option("--no-mpi",
		       default=False, action='store_true',
		       help='disable mpi', dest='no_mpi')
	opt.add_option('--mpi', type='string', help='path to mpi', dest='mpi')


@conf
def check_mpi(conf):
	opt = conf.options

	conf.env['LIB_MPI'] = ''
	conf.env['MPI_FOUND'] = False
	if conf.options.no_mpi :
		return
	if conf.options.mpi:
		conf.env.INCLUDES_MPI = conf.options.mpi + '/include'
		conf.env.LIBPATH_MPI = conf.options.mpi + '/lib'
	else:
		conf.env.INCLUDES_MPI = ['/usr/include/mpi', '/usr/local/include/mpi', '/usr/include', '/usr/local/include']
		conf.env.LIBPATH_MPI = ['/usr/lib', '/usr/local/lib', '/usr/lib/openmpi']

	try:
		conf.start_msg('Checking for MPI include')
		res = conf.find_file('mpi.h', conf.env.INCLUDES_MPI)
		conf.end_msg('ok')
		conf.env['MPI_FOUND'] = True
		conf.env.LIB_MPI = ['mpi_cxx','mpi']
	except:
		conf.end_msg('Not found', 'RED')
	return 1

def detect(conf):
	return detect_mpi(conf)
