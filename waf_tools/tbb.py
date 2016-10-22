#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2014

"""
Quick n dirty tbb detection
"""

from waflib.Configure import conf
from waflib import Utils, Logs

def options(opt):
    opt.add_option('--tbb', type='string', help='path to Intel TBB', dest='tbb')


@conf
def check_tbb(self, *k, **kw):
    if self.options.tbb:
        includes_tbb = [self.options.tbb + '/include']
        libpath_tbb = [self.options.tbb + '/lib']
    else:
        includes_tbb = ['/usr/local/include', '/usr/include', '/opt/intel/tbb/include']
        libpath_tbb = ['/usr/local/lib/', '/usr/lib', '/opt/intel/tbb/lib']

    self.start_msg('Checking Intel TBB includes (optional)')
    try:
        res = self.find_file('tbb/parallel_for.h', includes_tbb)
        index = includes_tbb.index(res[:-len('tbb/parallel_for.h')-1])
        includes_tbb = [includes_tbb[index]]
        self.end_msg('ok')
        if Logs.verbose:
            Logs.pprint('CYAN', '	path : %s' % includes_tbb[0])
    except:
        self.end_msg('not found', 'YELLOW')
        return

    self.start_msg('Checking Intel TBB libs (optional)')
    try:
        res = self.find_file('libtbb.so', libpath_tbb)
        index = libpath_tbb.index(res[:-len('libtbb.so')-1])
        libpath_tbb = [libpath_tbb[index]]
        self.end_msg('ok')
        if Logs.verbose:
            Logs.pprint('CYAN', '	path : %s' % libpath_tbb[0])
            Logs.pprint('CYAN', '	libs : [\'tbb\']')
    except:
        self.end_msg('not found', 'YELLOW')
        return

    self.env.LIBPATH_TBB = libpath_tbb
    self.env.LIB_TBB = ['tbb']
    self.env.INCLUDES_TBB = includes_tbb
    self.env['TBB_ENABLED'] = True
