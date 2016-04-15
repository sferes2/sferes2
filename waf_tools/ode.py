#! /usr/bin/env python
# encoding: utf-8
# JB Mouret - 2009

"""
Quick n dirty ODE detection
"""

import commands
from waflib.Configure import conf
import re

def options(opt):
    pass


@conf
def check_ode(conf):
    env = conf.env
    conf.start_msg('Checking for ODE')
    try:
        ret = conf.find_program('ode-config')
    except:
        conf.end_msg('not found', 'RED')
        return 0
    conf.end_msg('ok')
    res = commands.getoutput('ode-config --cflags --libs')
# conf.parse_flags seems to do not parse anything at all.......

    res=re.split(" |\n",res)
#ugly handmade parser:
    for word in res:
        if word.startswith('-I'):
            if env['INCLUDES_ODE']:
                env['INCLUDES_ODE'].append(word[2:])
            else:
                env['INCLUDES_ODE']=word[2:]
        if word.startswith('-L'):
            if env['LIBPATH_ODE']:
                env['LIBPATH_ODE'].append(word[2:])
            else:
                env['LIBPATH_ODE']=word[2:]
        if word.startswith('-l'):
            if env['LIB_ODE']:
                env['LIB_ODE'].append(word[2:])
            else:
                env['LIB_ODE']=word[2:]

        if word.startswith('-D'):
            if env['CXXFLAGS_ODE']:
                env['CXXFLAGS_ODE'].append(word)
            else:
                env['CXXFLAGS_ODE']=word

    return 1
