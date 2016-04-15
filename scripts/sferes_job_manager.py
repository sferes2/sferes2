#!/usr/bin/env python
import sys, os
import subprocess, shlex
import commands
import json
import time
import signal
import glob
import random
def create_dirs(exps, res_dir, nb_reps):
    for e in exps:
        for i in range(0, nb_reps):
             directory = res_dir + "/" + str(e) + "/exp_" + str(i)
             print sys.argv[0], "-> directory ", directory,
             try:
                 os.makedirs(directory)
                 print " created"
             except Exception, ex:
                 print " not created => ", ex

def status_file(res_dir, e, i): return res_dir + "/" + str(e) + "/exp_" + str(i) + "/status"

def list_open_exps(exps, res_dir, nb_reps):
    open_exps = []
    for e in exps:
        for i in range(0, nb_reps):
           status_fname = status_file(res_dir, e, i)
           if os.path.exists(status_fname):
               f = open(status_fname, 'r')
               status = f.read()
               if status == 'interrupted':
                   open_exps += [(e, i, status)]
           else:
               open_exps += [(e, i, 'ready')]
    return open_exps

def last_gen_file(res_dir, exp):
     d = res_dir + "/" + exp[0] + "/exp_" + str(exp[1])
     print d
     l = glob.glob(d + "/gen_*")
     l.sort(lambda x,y: int(x.split('_')[-1]) - int(y.split('_')[-1]))
     return l[-1], int(l[-1].split('_')[-1])

def sig_handler(signum, frame, process, exp, res_dir):
    print sys.argv[0], 'Signal handler called with signal', signum
    # we kill our experiment (this is unfortunate!)
    process.send_signal(signum)
    print sys.argv[0], ' -> exiting (signal sent to the child)'
    exit(0)

def unlock(res_dir): os.remove(res_dir + '/lock')

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

     if (len(open_exps) == 0):
         print 'all experiments are finished! Good job.'
         unlock(res_dir)
         return


     # adopt the experiment
     # if we say 'random' on the command line, then we take one of the
     # random open experiment (the goal is to give some computation time to
     # each of the experiments, instead of a more sequential approach).
     if 'random' in sys.argv:
         print sys.argv[0], '-> policy: random'
         exp = open_exps[random.randint(0, len(open_exps) - 1)]
     elif 'first' in sys.argv:
         print sys.argv[0], '-> policy: first'
         exp = open_exps[0]
     else: # fair attribution
         print sys.argv[0], '-> policy: fair'
         exp = None
         best_gen = 1e20
         for e in open_exps:
             print e
             if e[2] == 'ready':
                 exp = e
                 break
             if e[2] == 'interrupted':
                gen_file, gen = last_gen_file(res_dir, e)
                print 'gen:', gen
                if gen < best_gen:
                    best_gen = gen
                    exp = e

     print sys.argv[0], "adopted experiment:", exp

     ### launch the experiment

     if exp[2] == "ready": # new experiment
        to_execute = bin_dir + "/" + exp[0] + " -d " + res_dir + "/" + exp[0] + "/exp_" + str(exp[1])
     elif exp[2] == "interrupted": # we need to resume
        # find the most recent gen file
        gen_file, gen = last_gen_file(res_dir, exp)
        print sys.argv[0], "gen file:", gen_file
        to_execute = bin_dir + "/" + exp[0] + " -r " + gen_file + " -d " + res_dir + "/" + exp[0] + "/exp_" + str(exp[1])
     else:
        print 'unknown status', exp[2]
        unlock()
        return
     print sys.argv[0], "launching:", to_execute
     process = subprocess.Popen(shlex.split(to_execute))

     ### if we are interrupted, we send the signal to the child
     def handler(x, y) : sig_handler(x, y, process, exp, res_dir)
     signal.signal(signal.SIGINT, handler)
     signal.signal(signal.SIGQUIT, handler)
     # we do not trap sigkill (should we?)

     #remove the lock
     unlock(res_dir)
     print sys.argv[0],"lock removed, waiting for child"
     process.wait()

main(sys.argv[1])
