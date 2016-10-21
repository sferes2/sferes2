#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2014

"""
Quick n dirty tbb detection
"""
import os
from waflib.Configure import conf


def options(opt):
    opt.add_option('--tbb', type='string', help='path to Intel TBB', dest='tbb')


@conf
def check_tbb(self, *k, **kw):
    includes_tbb = []
    libpath_tbb = []
    if self.options.tbb:
        includes_tbb += [self.options.tbb + '/include']
        libpath_tbb += [self.options.tbb + '/lib']
    else:
        includes_tbb += [path[2:] for path in self.env['CPPFLAGS'] if path[0:2] == '-I']
        if 'CPPFLAGS' in os.environ:
            includes_tbb += [path[2:] for path in os.environ['CPPFLAGS'].split() if path[0:2] == '-I']
        if 'LD_LIBRARY_PATH' in os.environ:
            libpath_tbb += os.environ['LD_LIBRARY_PATH'].split(":")
        libpath_tbb += ['/usr/local/lib/', '/usr/lib', '/opt/intel/tbb/lib']

    self.start_msg('Checking Intel TBB includes')
    try:
        self.find_file('tbb/parallel_for.h', includes_tbb)
        self.end_msg('ok')
    except:
        self.end_msg('not found', 'YELLOW')
        return

    self.env.LIBPATH_TBB = libpath_tbb
    self.env.LIB_TBB = ['tbb']
    self.env.INCLUDES_TBB = includes_tbb
    self.env['TBB_ENABLED'] = True
