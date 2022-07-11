#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty ssrc (kd-tree library) detection
"""

import os, glob, types
from waflib.Configure import conf
from waflib import Utils, Logs


def options(opt):
	opt.add_option('--kdtree', type='string', help='path to ssrckdtree (KD-tree)', dest='kdtree')


@conf
def check_kdtree(conf):
	conf.env['KDTREE_FOUND'] = False
	conf.start_msg('Checking for ssrc kdtree (KD-tree)')
	includes_check = ['/usr/include', '/usr/local/include']
	if conf.options.kdtree:
		includes_check = [conf.options.kdtree]
		conf.env.INCLUDES_KDTREE = [conf.options.kdtree]
	else:
		if 'CPPFLAGS' in os.environ:
			includes_check += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']

	try:
		res = conf.find_file('ssrc/spatial/kd_tree.h', includes_check)
		index = includes_check.index(res[:-len('ssrc/spatial/kd_tree.h')-1])
		conf.env.INCLUDES_KDTREE = [includes_check[index]]
		conf.end_msg('ok')
		if Logs.verbose:
			Logs.pprint('CYAN', '	path : %s' % includes_check[index])
		conf.env['KDTREE_FOUND'] = True
	except:
		conf.end_msg('Not found', 'RED')
	return 1
