#!/usr/bin/env python
import sys, os
import subprocess, shlex
import commands
import json
import time
import signal
import glob

def create_dirs(exps, res_dir, nb_reps):
    for e in exps:
        for i in range(0, nb_reps):
             directory = res_dir + "/" + str(e) + "/exp_" + str(i)
             print "directory ", directory,
             try:
                 os.makedirs(directory)
                 print " created"
             except Exception, ex:
                 print " not created => ", ex

def status_file(res_dir, e, i): return res_dir + "/" + str(e) + "/exp_" + str(i) + "/status"

def change_status(res_dir, e, i, status):
    status_fname = status_file(res_dir, e, i)
    f = open(status_fname, "w+")
    f.write(status)
    f.close()

def list_open_exps(exps, res_dir, nb_reps):
    open_exps = []
    for e in exps:
        for i in range(0, nb_reps):
           status_fname = status_file(res_dir, e, i)
           if os.path.exists(status_fname):
               f = open(status_fname, 'r')
               status = f.read()
               if status == 'ready' or status == 'interrupted':
                   open_exps += [(e, i, status)]
           else:
               change_status(res_dir, e, i, "ready")
               open_exps += [(e, i, 'ready')]
    return open_exps

def last_gen_file(res_dir, exp):
     d = res_dir + "/" + exp[0] + "/exp_" + str(exp[1])
     print d
     l = glob.glob(d + "/gen_*")
     l.sort(lambda x,y: int(x.split('_')[-1]) - int(y.split('_')[-1]))
     return l[-1]

def sig_handler(signum, frame, process, exp, res_dir):
    print sys.argv[0], 'Signal handler called with signal', signum
    # we kill our experiment (this is unfortunate!)
    # we mark our experiment as interrupted
    process.send_signal(signum)
    change_status(res_dir, exp[0], exp[1], 'interrupted')
    print sys.argv[0], ' -> exiting (signal sent to the child)'
    exit(0)

def main(conf_file):
     # load the file
     conf = json.load(open(conf_file))
     res_dir = conf['res_dir']

     #lock the res directory
     if not os.path.exists(res_dir):
         os.makedirs(res_dir)
     while os.path.exists(res_dir + "/lock"):
         sys.stdout.write(".")
         sys.stdout.flush()
         time.sleep(1)
     open(res_dir + "/lock", "w")
     sys.stdout.write('\n')

     # ready
     bin_dir = conf['bin_dir']
     last_file = conf['last_file']
     exps = conf['exps']
     nb_reps = conf['replicates']
     print sys.argv[0], "running ", exps, "with", nb_reps, "replicates"

     # create dirs if they do not exist
     create_dirs(exps, res_dir, nb_reps)

     # list open experiments
     open_exps = list_open_exps(exps, res_dir, nb_reps)
     print sys.argv[0], "open experiments:", open_exps

     # adopt the experiment
     exp = open_exps[0]
     change_status(res_dir, exp[0], exp[1], 'running')

     print sys.argv[0], "STATUS:", exp[2]

     if exp[2] == "ready": # new experiment
        to_execute = bin_dir + "/" + exp[0] + " -d " + res_dir + "/" + exp[0] + "/exp_" + str(exp[1])
     elif exp[2] == "interrupted": # we need to resume
        # find the most recent gen file
        gen_file = last_gen_file(res_dir, exp)
        print sys.argv[0], "gen file:", gen_file
        to_execute = bin_dir + "/" + exp[0] + " -r " + gen_file + " -d " + res_dir + "/" + exp[0] + "/exp_" + str(exp[1])
     print sys.argv[0], "launching:", to_execute
     process = subprocess.Popen(shlex.split(to_execute))

     def handler(x, y) : sig_handler(x, y, process, exp, res_dir)
     signal.signal(signal.SIGINT, handler)
     signal.signal(signal.SIGQUIT, handler)
     # we do not trap sigkill yet

     #remove the lock
     os.remove(res_dir + '/lock')

     print sys.argv[0],"lock removed, waiting for child"
     process.wait()

main(sys.argv[1])
