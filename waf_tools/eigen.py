#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty eigen3 detection
"""

import os, glob, types
from waflib.Configure import conf


def options(opt):
	opt.add_option('--eigen', type='string', help='path to eigen', dest='eigen')


@conf
def check_eigen(conf):
	conf.env['EIGEN_FOUND'] = False
	conf.env.INCLUDES_EIGEN = []
	conf.start_msg('Checking for Eigen')
	if conf.options.eigen:
		conf.env.INCLUDES_EIGEN += [conf.options.eigen]
	else:
		if 'CPPFLAGS' in os.environ:
			conf.env.INCLUDES_EIGEN += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
			conf.env.INCLUDES_EIGEN += ['/usr/include/eigen3',
										'/usr/local/include/eigen3',
										'/usr/include', '/usr/local/include']
	try:
		res = conf.find_file('Eigen/Core', conf.env.INCLUDES_EIGEN)
		conf.end_msg('ok')
		conf.env['EIGEN_FOUND'] = True
	except:
		conf.end_msg('Not found', 'RED')
	return 1
