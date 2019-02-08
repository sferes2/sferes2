#!/usr/bin/env python
# encoding: utf-8

"""
Quick n dirty pthread detection
"""

from waflib.Configure import conf
import sferes

@conf
def check_pthread(self, *k, **kw):
    libpath = ['/usr/local/lib/', '/usr/lib', '/opt/intel/tbb/lib', '/usr/lib/x86_64-linux-gnu/']
    self.start_msg('Checking pthread')
    lib = ''
    try:
        res, lib = sferes.check_lib(self, 'libpthread', libpath)
        self.end_msg(lib)
    except:
        self.end_msg('Not found in %s' % str(libpath), 'YELLOW')
        return

    self.env.LIBPATH_PTHREAD = [lib]
    self.env.LIB_PTHREAD = ['pthread']
