#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty eigen3 detection
"""

import os, glob, types
from waflib.Configure import conf
from waflib import Utils, Logs


def options(opt):
	opt.add_option('--eigen', type='string', help='path to eigen', dest='eigen')


@conf
def check_eigen(conf):
	conf.env['EIGEN_FOUND'] = False
	conf.start_msg('Checking for Eigen')
	includes_check = ['/usr/include/eigen3', '/usr/local/include/eigen3', '/usr/include', '/usr/local/include']
	if conf.options.eigen:
		includes_check = [conf.options.eigen]
		conf.env.INCLUDES_EIGEN = [conf.options.eigen]
	else:
		if 'CPPFLAGS' in os.environ:
			includes_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']

	try:
		res = conf.find_file('Eigen/Core', includes_check)
		index = includes_check.index(res[:-len('Eigen/Core')-1])
		conf.env.INCLUDES_EIGEN = [includes_check[index]]
		conf.end_msg('ok')
		if Logs.verbose:
			Logs.pprint('CYAN', '	path : %s' % includes_check[index])
		conf.env['EIGEN_FOUND'] = True
	except:
		conf.end_msg('Not found', 'RED')
	return 1
