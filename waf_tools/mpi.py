#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty mpi detection
"""

import os, glob, types
from waflib.Configure import conf
from waflib import Utils, Logs

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
	includes_check = []
	libs_check = []
	if conf.options.no_mpi :
		return
	if conf.options.mpi:
		includes_check = [conf.options.mpi + '/include']
		libs_check = [conf.options.mpi + '/lib']
	else:
		includes_check = ['/usr/include/mpi', '/usr/local/include/mpi', '/usr/include', '/usr/local/include']
		libs_check = ['/usr/lib', '/usr/local/lib', '/usr/lib/openmpi']
		if 'MPI_ROOT' in os.environ:
			includes_check += [os.environ['MPI_ROOT'] +"/include", os.environ['MPI_ROOT'] +"/include64"]
		elif 'I_MPI_ROOT' in os.environ:
			includes_check += [os.environ['I_MPI_ROOT'] +"/include", os.environ['I_MPI_ROOT'] +"/include64"]
		elif 'CPPFLAGS' in os.environ:
			includes_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
		if 'LD_LIBRARY_PATH' in os.environ:
			libs_check += os.environ['LD_LIBRARY_PATH'].split(":")

	try:
		conf.start_msg('Checking for MPI include (optional)')
		res = conf.find_file('mpi.h', includes_check)
		i_index = includes_check.index(res[:-len('mpi.h')-1])
		conf.end_msg('ok')

		if Logs.verbose:
			Logs.pprint('CYAN', '	path : %s' % includes_check[i_index])

		conf.start_msg('Checking for MPI libs (optional)')
		lib_paths = []
		libs  = ['libmpi.so']
		for l in libs:
			res = conf.find_file(l, libs_check)
			index = libs_check.index(res[:-len(l)-1])
			if libs_check[index] not in lib_paths:
				lib_paths += [libs_check[index]]
		anta_libs = ['mpi_cxx', 'mpicxx']
		found = False
		ii = -1
		for i in range(len(anta_libs)):
			l = 'lib'+anta_libs[i]+'.so'
			try:
				res = conf.find_file(l, libs_check)
				index = libs_check.index(res[:-len(l)-1])
				if libs_check[index] not in lib_paths:
					lib_paths += [libs_check[index]]
				found = True
				ii = i
				break
			except:
				continue
		if not found:
			conf.end_msg('Not found', 'RED')
			return 1
		conf.end_msg('ok')

		if Logs.verbose:
			Logs.pprint('CYAN', '	paths : %s' % lib_paths)
			Logs.pprint('CYAN', '	libs : %s' % ['mpi_cxx','mpi'])

		conf.env.INCLUDES_MPI = includes_check[i_index]
		conf.env.LIBPATH_MPI = lib_paths
		conf.env['MPI_FOUND'] = True
		conf.env.LIB_MPI = [anta_libs[ii],'mpi']
	except:
		conf.end_msg('Not found', 'RED')
	return 1

def detect(conf):
	return detect_mpi(conf)
