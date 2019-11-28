import sys, os
import subprocess
import stat

json_ok = True
try:
   import simplejson
except:
   json_ok = False
   print("WARNING simplejson not found some function may not work")

import glob

# check if a lib exists for both osx (darwin) and GNU/linux
def check_lib(self, name, path):
    if self.env['DEST_OS']=='darwin':
        libname = name + '.dylib'
    else:
        libname = name + '.so'
    res = self.find_file(libname, path)
    lib = res[:-len(libname)-1]
    return res, lib

def create_variants(bld, source, use,
                    uselib, variants, includes=". ../ ../../",
                    cxxflags='',
                    target=''):
    source_list = source.split()
    if not target:
        tmp = source_list[0].replace('.cpp', '')
    else:
        tmp = target
    for v in variants:
        deff = []
        suff = ''
        for d in v.split(' '):
            suff += d.lower() + '_'
            deff.append(d)
        bin_fname = tmp + '_' + suff[0:len(suff) - 1]
        bld.program(features='cxx',
                    source=source,
                    target=bin_fname,
                    includes=includes,
                    uselib=uselib,
                    cxxflags=cxxflags,
                    use=use,
                    defines=deff)

def create_exp(name):
   ws_tpl = """
#! /usr/bin/env python
def build(bld):
    bld.program(features = 'cxx',
            source = '@exp.cpp',
            includes = '. ../../',
            uselib = 'TBB BOOST EIGEN PTHREAD MPI',
            use = 'sferes2',
            target = 'example')
"""
   os.mkdir('exp/' + name)
   os.system("cp examples/ex_ea.cpp exp/" + name + "/" + name + ".cpp")
   wscript = open('exp/' + name + "/wscript", "w")
   wscript.write(ws_tpl.replace('@exp', name))


def parse_modules():
   if (not os.path.exists("modules.conf")):
      return []
   mod = open("modules.conf")
   modules = []
   for i in mod:
      if i[0] != '#' and len(i) != 1:
         modules += ['modules/' + i[0:len(i)-1]]
   return modules

def _sub_script(tpl, conf_file):
    if 'LD_LIBRARY_PATH' in os.environ:
        ld_lib_path = os.environ['LD_LIBRARY_PATH']
    else:
        ld_lib_path = "''"
    print('LD_LIBRARY_PATH=' + ld_lib_path)
     # parse conf
    conf = simplejson.load(open(conf_file))
    exps = conf['exps']
    nb_runs = conf['nb_runs']
    res_dir = conf['res_dir']
    bin_dir = conf['bin_dir']
    wall_time = conf['wall_time']
    use_mpi = "false"
    try:
        use_mpi = conf['use_mpi']
    except:
        use_mpi = "false"
    try:
        nb_cores = conf['nb_cores']
    except:
        nb_cores = 1
    try:
        args = conf['args']
    except:
        args = ''
    email = conf['email']
    if (use_mpi == "true"):
        ppn = '1'
        mpirun = 'mpirun'
    else:
 #      nb_cores = 1;
        ppn = "8"
        mpirun = ''

    fnames = []
    for i in range(0, nb_runs):
        for e in exps:
            directory = res_dir + "/" + e + "/exp_" + str(i)
            try:
                os.makedirs(directory)
            except:
                print("WARNING, dir:" + directory + " not be created")
            subprocess.call('cp ' + bin_dir + '/' + e + ' ' + directory, shell=True)
            fname = directory + "/" + e + "_" + str(i) + ".job"
            f = open(fname, "w")
            f.write(tpl
                    .replace("@exp", e)
                    .replace("@email", email)
                    .replace("@ld_lib_path", ld_lib_path)
                    .replace("@wall_time", wall_time)
                    .replace("@dir", directory)
                    .replace("@nb_cores", str(nb_cores))
                    .replace("@ppn", ppn)
                    .replace("@exec", mpirun + ' ' + directory + '/' + e + ' ' + args))
            f.close()
            os.chmod(fname, stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)
            fnames += [(fname, directory)]
    return fnames


def qsub(conf_file):
    tpl = """#!/bin/sh
#? nom du job affiche
#PBS -N @exp
#PBS -o stdout
#PBS -b stderr
#PBS -M @email
# maximum execution time
#PBS -l walltime=@wall_time
# mail parameters
#PBS -m abe
# number of nodes
#PBS -l nodes=@nb_cores:ppn=@ppn
#PBS -l pmem=5200mb -l mem=5200mb
export LD_LIBRARY_PATH=@ld_lib_path
exec @exec
"""
    fnames = _sub_script(tpl, conf_file)
    for (fname, directory) in fnames:
        s = "qsub -d " + directory + " " + fname
        print("executing:" + s)
        retcode = subprocess.call(s, shell=True, env=None)
        print("qsub returned:" + str(retcode))


def oar(conf_file):
    tpl = """#!/bin/bash
#OAR -l /nodes=1/core=@nb_cores,walltime=@wall_time
#OAR -n @exp
#OAR -O stdout.%jobid%.log
#OAR -E stderr.%jobid%.log
export LD_LIBRARY_PATH=@ld_lib_path
exec @exec
"""
    print('WARNING [oar]: MPI not supported yet')
    fnames = _sub_script(tpl, conf_file)
    for (fname, directory) in fnames:
        s = "oarsub -d " + directory + " -S " + fname
        print("executing:" + s)
        retcode = subprocess.call(s, shell=True, env=None)
        print("oarsub returned:" + str(retcode))
