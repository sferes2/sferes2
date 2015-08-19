import sys, os
import subprocess
import commands

json_ok = True
try:
   import simplejson
except:
   json_ok = False
   print "WARNING simplejson not found some function may not work"

import glob
#import xml.etree.cElementTree as etree
import Options



def create_variants(bld, source, uselib_local, target, 
                    uselib, variants, includes=". ../../", 
                    cxxflags='',
                    json=''):
   # the basic one
   #   tgt = bld.new_task_gen('cxx', 'program')
   #   tgt.source = source
   #   tgt.includes = includes
   #   tgt.uselib_local = uselib_local
   #   tgt.uselib = uselib
   #   tgt.target = target
   # the variants
   c_src = bld.path.abspath() + '/'
   for v in variants:
      # create file
      suff = ''
      for d in v.split(' '): suff += d.lower() + '_'
      tmp = source.replace('.cpp', '')
      src_fname = tmp + '_' + suff[0:len(suff) - 1] + '.cpp'
      f = open(c_src + src_fname, 'w')
      f.write("// THIS IS A GENERATED FILE - DO NOT EDIT\n")
      for d in v.split(' '): f.write("#define " + d + "\n")
      f.write("#line 1 \"" + c_src + source + "\"\n")
      code = open(c_src + source, 'r')
      for line in code: f.write(line)
      bin_name = src_fname.replace('.cpp', '')
      bin_name = os.path.basename(bin_name)
      # create build
      tgt = bld.new_task_gen('cxx', 'program')
      tgt.source = src_fname
      tgt.includes = includes
      tgt.uselib_local = uselib_local
      tgt.uselib = uselib
      tgt.target = bin_name
      tgt.cxxflags = cxxflags


def create_exp(name):
   ws_tpl = """
#! /usr/bin/env python
def build(bld):
    obj = bld.new_task_gen('cxx', 'program')
    obj.source = '@exp.cpp'
    obj.includes = '. ../../'
    obj.uselib_local = 'sferes2'
    obj.uselib = ''
    obj.target = '@exp'
    obj.uselib_local = 'sferes2'
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

def qsub(conf_file):
   tpl = """
#! /bin/sh
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
   if os.environ.has_key('LD_LIBRARY_PATH'):
      ld_lib_path = os.environ['LD_LIBRARY_PATH']
   else:
      ld_lib_path = "''"
   home = os.environ['HOME']
   print 'LD_LIBRARY_PATH=' + ld_lib_path
    # parse conf
   conf = simplejson.load(open(conf_file))
   exps = conf['exps']
   nb_runs = conf['nb_runs']
   res_dir = conf['res_dir']
   bin_dir = conf['bin_dir']
   wall_time = conf['wall_time']
   use_mpi = "false"
   try: use_mpi = conf['use_mpi']
   except: use_mpi = "false"
   try: nb_cores = conf['nb_cores']
   except: nb_cores = 1
   try: args = conf['args']
   except: args = ''
   email = conf['email']
   if (use_mpi == "true"): 
      ppn = '1'
      mpirun = 'mpirun'
   else:
      nb_cores = 1; 
      ppn = '8'
      mpirun = ''
   
   for i in range(0, nb_runs):
      for e in exps:
         directory = res_dir + "/" + e + "/exp_" + str(i) 
         try:
            os.makedirs(directory)
         except:
            print "WARNING, dir:" + directory + " not be created"
         subprocess.call('cp ' + bin_dir + '/' + e + ' ' + directory, shell=True)
         fname = home + "/tmp/" + e + "_" + str(i) + ".job"
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
         s = "qsub -d " + directory + " " + fname
         print "executing:" + s
         retcode = subprocess.call(s, shell=True, env=None)
         print "qsub returned:" + str(retcode)
