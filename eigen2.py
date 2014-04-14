#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty eigen2 detection
"""

import os, glob, types
import Options, Configure

def detect_eigen2(conf):
	env = conf.env
	opt = Options.options

	conf.env['LIB_EIGEN2'] = ''
	conf.env['EIGEN2_FOUND'] = False
	if Options.options.no_eigen2:
		return 0
	if Options.options.eigen2:
		conf.env['CPPPATH_EIGEN2'] = [Options.options.eigen2]
		conf.env['LIBPATH_EIGEN2'] = [Options.options.eigen2]
	else:
		conf.env['CPPPATH_EIGEN2'] = ['/usr/include/eigen2', '/usr/local/include/eigen2', '/usr/include', '/usr/local/include']
		conf.env['LIBPATH_EIGEN2'] = ['/usr/lib', '/usr/local/lib']

	res = Configure.find_file('Eigen/Core', conf.env['CPPPATH_EIGEN2'])
	conf.check_message('header','Eigen/Core', (res != '') , res)
	if (res == '') :
		return 0
	conf.env['EIGEN2_FOUND'] = True
	return 1

def detect(conf):
	return detect_eigen2(conf)

def set_options(opt):
	opt.add_option('--eigen2', type='string', help='path to eigen2', dest='eigen2')
	opt.add_option('--no-eigen2', type='string', help='disable eigen2', dest='no_eigen2')
