#!/usr/bin/env python
# encoding: utf-8

"""
Quick n dirty tbb detection
"""

from waflib.Configure import conf
import sferes

def options(opt):
    opt.add_option('--tbb', type='string', help='path to Intel TBB', dest='tbb')


@conf
def check_tbb(self, *k, **kw):
    if self.options.tbb:
        includes_tbb = [self.options.tbb + '/include']
        libpath_tbb = [self.options.tbb + '/lib']
    else:
        includes_tbb = ['/usr/local/include', '/usr/include', '/opt/intel/tbb/include']
        libpath_tbb = ['/usr/local/lib/', '/usr/lib', '/opt/intel/tbb/lib', '/usr/lib/x86_64-linux-gnu/']

    self.start_msg('Checking Intel TBB includes (optional)')
    incl = ''
    lib = ''
    try:
        res = self.find_file('tbb/parallel_for.h', includes_tbb)
        incl = res[:-len('tbb/parallel_for.h')-1]
        self.end_msg(incl)
    except:
        self.end_msg('Not found in %s' % str(includes_tbb), 'YELLOW')
        return

    self.start_msg('Checking Intel TBB libs (optional)')
    try:
        res, lib = sferes.check_lib(self, 'libtbb', libpath_tbb)
        self.end_msg(lib)
    except:
        self.end_msg('Not found in %s' % str(libpath_tbb), 'YELLOW')
        return

    self.env.LIBPATH_TBB = [lib]
    self.env.LIB_TBB = ['tbb']
    self.env.INCLUDES_TBB = [incl]
    self.env.DEFINES_TBB = ['USE_TBB']
