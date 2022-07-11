Sferes on clusters (OAR)
========================
.. highlight:: c++

OAR Scheduler (ResiBots / Inria cluster)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*This information should work for most OAR clusters but some details might be specific to Inria's ResiBots cluster (hal*).

*Inria users: you need to ask JBM for an account* 

Introduction to OAR
-------------------

OAR is a resource and task manager (also called a batch scheduler) for HPC clusters. We are using this one because it also used on Grid'5000 (Inria large cluster) and because it is very flexible. As a scheduler, OAR will take your job (your program) and send it to one of the nodes according to the availability. If no node is available, it will automatically launch it as soon as a slot is available.

The user documentation for OAR is available here: http://oar.imag.fr/docs/2.5/#ref-user-docs

Here is a summary of commands:
    - login to the main node: ``ssh hal01`` (use your Inria's password and login; if outside of Inria, you need the VPN)
    - list the current jobs: ``oarstat`` or use the web interfaces:  http://hal01.loria.fr/drawgantt-svg/ or http://hal01.loria.fr/monika (VPN needed)
    - kill a job: ``oardel your_job_number`` (as listed by ``oarstat``)
    - general command to submit a job: ``oarsub``; example: ``oarsub -l /nodes=1/core=16,walltime=00:30:00 build/debug/examples/ex_nsga2``
    - this will run ex_nsga2 one 1 node with 16 cores for 30 minutes (the job will be killed after 30 minutes)
        - INRIA: *you need to run everything from /nfs/hal01/your_user/ and NEVER from your home directory or any local directory (e.g., /tmp).* This is because your program and the data are not copied between nodes: it is launched from a shared disk (/nfs/hal01).
        - for more complex submissions, ``oarsub`` can take a shell script as an input (useful for LD_LIBRARY_PATH and other settings). Example:


.. code:: shell

    #!/bin/bash
    #OAR -l /core=24/nodes=1,walltime=270:00:00
    #OAR -n hexa_duty_text
    #OAR -O stdout.%jobid%.log
    #OAR -E stderr.%jobid%.log
    export LD_LIBRARY_PATH=''
    exec  /nfs/hal01/jmouret/data/maps_hexapod//hexa_duty_text/exp_0/hexa_duty_text 


    
- This script can be submitted like this: ``oarsub -d the_directory_to_launch -S your_script_name``
- To see the output of a job: ``cat OAR.{job-number}.stdout`` and ``cat OAR.{job-number}.stderr``


Using sferes with OAR
----------------------

Sferes has a high-level interface to make it easy to submit jobs to OAR.

First, you need to specify your experiment in yaml file:

.. code:: json

    {
        "email" : "user@example.com",
        "wall_time" : "24:00:00",
        "nb_runs": 3,
        "nb_cores":24,
        "bin_dir": "/nfs/hal01/jmouret/git/sferes2/build/examples/",
        "res_dir": "/nfs/hal01/jmouret/git/sferes2/data/",
        "exps" : ["ex_nsga2", "ex_ea"]
    }

where:
    - ``email`` is not used on ResiBots's cluster for now
    - ``wall_time`` is the time allocated to your experiment; please keep in mind that asking for too much (e.g., 1 day when you need 1 hour) will make your job less likely to be scheduled soon (because it will have a lower priority and because we keep some nodes for short jobs only)
    - ``nb_runs`` is the number of *replicates* of *each* experiment; in this example, `ex_nsga2` will be run 3 times, and `ex_ea` will be run 3 times
    - ``bin_dir`` is where to find the binaries that you want to run
    - ``res_dir`` is where to store the results (they will be organized by experiment, then by replicate)
    - ``exps`` is the list of binaries (program, experiments) that need to be run

You then submit this json file with ``waf``: ``./waf --oar path_to_your_json``

If everything works well, one job for each replicate should be scheduled. A typical output is:

.. code::

    WARNING [oar]: MPI not supported yet
    LD_LIBRARY_PATH=''
    executing:oarsub -d /nfs/hal01/jmouret/git/sferes2/data//ex_nsga2/exp_0 -S /nfs/hal01/jmouret/git/sferes2/data//ex_nsga2/exp_0/ex_nsga2_0.job
    [24hour QUEUE] This job is routed into the medium queue[ADMISSION RULE] Modify resource description with type constraints
    OAR_JOB_ID=737172
    oarsub returned:0
    executing:oarsub -d /nfs/hal01/jmouret/git/sferes2/data//ex_ea/exp_0 -S /nfs/hal01/jmouret/git/sferes2/data//ex_ea/exp_0/ex_ea_0.job
    [24hour QUEUE] This job is routed into the medium queue[ADMISSION RULE] Modify resource description with type constraints
    OAR_JOB_ID=737173
    oarsub returned:0
    executing:oarsub -d /nfs/hal01/jmouret/git/sferes2/data//ex_nsga2/exp_1 -S /nfs/hal01/jmouret/git/sferes2/data//ex_nsga2/exp_1/ex_nsga2_1.job
    [24hour QUEUE] This job is routed into the medium queue[ADMISSION RULE] Modify resource description with type constraints
    OAR_JOB_ID=737174
    oarsub returned:0
    executing:oarsub -d /nfs/hal01/jmouret/git/sferes2/data//ex_ea/exp_1 -S /nfs/hal01/jmouret/git/sferes2/data//ex_ea/exp_1/ex_ea_1.job
    [24hour QUEUE] This job is routed into the medium queue[ADMISSION RULE] Modify resource description with type constraints
    OAR_JOB_ID=737175
    oarsub returned:0
    executing:oarsub -d /nfs/hal01/jmouret/git/sferes2/data//ex_nsga2/exp_2 -S /nfs/hal01/jmouret/git/sferes2/data//ex_nsga2/exp_2/ex_nsga2_2.job
    [24hour QUEUE] This job is routed into the medium queue[ADMISSION RULE] Modify resource description with type constraints
    OAR_JOB_ID=737176
    oarsub returned:0
    executing:oarsub -d /nfs/hal01/jmouret/git/sferes2/data//ex_ea/exp_2 -S /nfs/hal01/jmouret/git/sferes2/data//ex_ea/exp_2/ex_ea_2.job
    [24hour QUEUE] This job is routed into the medium queue[ADMISSION RULE] Modify resource description with type constraints
    OAR_JOB_ID=737177
    oarsub returned:0

You can check that your jobs are scheduled with `oarstat`. If you do not see them, this is usually because there is an error (e.g., they did a segmentation fault, or the binary did not run because there is a missing library, etc.). To know the error, check the error file in the `data` directory. For instance:

.. code:: shell

    # for the error messages:
    cat data/ex_nsga2/exp_0/stderr.737172.log
    
    # for the output of your program:
    cat data/ex_nsga2/exp_0/stdout.737172.log
    
    # to know how the job was launched:
    data/ex_nsga2/exp_0/ex_nsga2_0.job


This is what you should have in the ``data`` directory once all the jobs are finished:

.. code:: shell

    data/
    data/ex_ea
    data/ex_ea/exp_2
    data/ex_ea/exp_2/stderr.737177.log
    data/ex_ea/exp_2/ex_ea_2.job
    data/ex_ea/exp_2/ex_ea
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_110000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_150000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_85000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_15000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_10000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/status
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_135000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_95000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_90000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_160000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_165000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_65000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_115000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_155000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_50000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_0
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/bestfit.dat
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_180000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_195000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_25000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_140000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_45000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_105000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_30000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_40000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_200000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_145000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_100000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_70000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_130000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_175000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_80000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_120000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_190000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_60000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_20000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_55000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_125000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_75000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_170000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_35000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_185000
    data/ex_ea/exp_2/ex_ea_2019-04-30_11_41_39_129952/gen_5000
    data/ex_ea/exp_2/stdout.737177.log
    data/ex_ea/exp_0
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_110000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_150000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_85000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_15000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_10000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/status
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_135000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_95000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_90000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_160000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_165000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_65000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_115000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_155000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_50000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_0
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/bestfit.dat
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_180000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_195000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_25000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_140000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_45000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_105000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_30000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_40000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_200000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_145000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_100000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_70000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_130000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_175000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_80000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_120000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_190000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_60000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_20000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_55000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_125000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_75000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_170000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_35000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_185000
    data/ex_ea/exp_0/ex_ea_2019-04-30_11_41_25_100343/gen_5000
    data/ex_ea/exp_0/ex_ea
    data/ex_ea/exp_0/ex_ea_0.job
    data/ex_ea/exp_0/stderr.737173.log
    data/ex_ea/exp_0/stdout.737173.log
    data/ex_ea/exp_1
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_110000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_150000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_85000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_15000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_10000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/status
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_135000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_95000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_90000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_160000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_165000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_65000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_115000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_155000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_50000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_0
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/bestfit.dat
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_180000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_195000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_25000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_140000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_45000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_105000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_30000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_40000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_200000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_145000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_100000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_70000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_130000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_175000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_80000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_120000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_190000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_60000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_20000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_55000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_125000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_75000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_170000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_35000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_185000
    data/ex_ea/exp_1/ex_ea_2019-04-30_11_41_39_185934/gen_5000
    data/ex_ea/exp_1/stdout.737175.log
    data/ex_ea/exp_1/ex_ea_1.job
    data/ex_ea/exp_1/ex_ea
    data/ex_ea/exp_1/stderr.737175.log
    data/ex_nsga2
    data/ex_nsga2/exp_2
    data/ex_nsga2/exp_2/ex_nsga2
    data/ex_nsga2/exp_2/stderr.737176.log
    data/ex_nsga2/exp_2/stdout.737176.log
    data/ex_nsga2/exp_2/ex_nsga2_2.job
    data/ex_nsga2/exp_0
    data/ex_nsga2/exp_0/ex_nsga2
    data/ex_nsga2/exp_0/stderr.737172.log
    data/ex_nsga2/exp_0/ex_nsga2_0.job
    data/ex_nsga2/exp_0/stdout.737172.log
    data/ex_nsga2/exp_1
    data/ex_nsga2/exp_1/ex_nsga2
    data/ex_nsga2/exp_1/ex_nsga2_1.job
    data/ex_nsga2/exp_1/stdout.737174.log
    data/ex_nsga2/exp_1/stderr.737174.log
