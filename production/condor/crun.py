#!/usr/bin/env python3
# Run gridpack to NANO on condor

import os
import sys
import subprocess
import socket

hostname = socket.gethostname()
print(f"Detected host {hostname}")
if "cmslpc" in hostname:
    host = "cmslpc"
elif "lxplus" in hostname:
    host = "lxplus"
elif "uscms" in hostname:
    host = "cmsconnect"
else:
    raise ValueError("Unknown host {}".format(hostname))

def make_proxy(proxy_path):
    os.system("voms-proxy-init -voms cms -out {} -valid 72:00".format(proxy_path))

def get_proxy_lifetime(proxy_path):
    import subprocess
    lifetime = float(subprocess.check_output("voms-proxy-info -timeleft -file {}".format(proxy_path), shell=True).strip())
    print("Proxy remaining lifetime: {}".format(lifetime))
    return lifetime

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Run HcalNano on condor")
    parser.add_argument("cfg", type=str, help="Path to cmsRun cfg (relative or absolute)")
    parser.add_argument("--extraArgs", type=str, default="", help="Extra cmsRun args, in quotes (besides inputFiles and outputFile, see dedicated args for those)")
    parser.add_argument("--inputFiles", "-i", type=str, help="Input files, comma-separated")
    parser.add_argument("--inputTxt", "-I", type=str, help="Path to text file containing input files (newline-separated)")
    parser.add_argument("--outputFile", "-o", type=str, default="hcalnano.root", help="Name of output file")
    parser.add_argument("--workdir", "-d", type=str, default="jobs", help="Name for submit/output directory (note timestamp is always added)")
    parser.add_argument("--outEOS", type=str, help="Transfer files to EOS instead of back to AFS")
    parser.add_argument("--outcp", type=str, help="Transfer output files with cp")
    parser.add_argument("--gfalcp", type=str, help="Transfer output files with gfalcp")
    parser.add_argument("--njobs", type=int, default=1, help="Number jobs")
    parser.add_argument("--max_nthreads", type=int, default=4, help="Maximum number of threads (reduce if condor priority is a problem)")
    parser.add_argument("--mem", type=int, default=7900, help="Memory to request")
    parser.add_argument("--overwrite", "-f", action="store_true", help="Force overwrite outputs")
    parser.add_argument("--no_retar", "-t", action="store_true", help="Don't recreate CMSSW tarball (saves time, but be careful)")
    args = parser.parse_args()

    import datetime
    current_datetime = datetime.datetime.now()
    job_ts = current_datetime.strftime("%y%m%d_%H%M%S")

    # Check fragment exists
    cfg_abspath = os.path.abspath(args.cfg)
    if not os.path.isfile(cfg_abspath):
        raise ValueError("Couldn't find cfg at {}".format(cfg_abspath))
    print("Using cmsRun cfg at {}".format(cfg_abspath))
    cfg_basename = os.path.basename(cfg_abspath)

    # Check proxy, make new one if necessary
    proxy_path = os.path.expandvars("$HOME/private/x509up")
    if not os.path.isfile(proxy_path):
        make_proxy(proxy_path)
    elif get_proxy_lifetime(proxy_path) < 24: # Require proxy with >24h left
        make_proxy(proxy_path)
    else:
        print("Using existing x509 proxy:")
        os.system("voms-proxy-info -file {}".format(proxy_path))

    input_files = []
    if args.inputFiles and args.inputTxt:
        raise ValueError("Incompatible arguments inputFiles and inputTxt")
    if args.inputFiles:
        input_files = args.inputFiles.split(",")
    elif args.inputTxt:
        with open(args.inputTxt, "r") as f:
            for line in f:
                input_files.append(line.strip())
    # Make sure EOS prefix is included
    if host == "lxplus":
        eos_prefix_inputs = "root://eoscms.cern.ch/"
    elif host == "cmslpc":
        eos_prefix_inputs = "root://cmseos.fnal.gov"
    elif host == "cmsconnect":
        eos_prefix_inputs = "root://cmseos.fnal.gov"
    else:
        raise ValueError("Unable to determine EOS prefix")
    for i, input_file in enumerate(input_files):
        if input_file[:7] == "/store":
            new_input_file = f"{eos_prefix_inputs}{input_file}"
            input_files[i] = new_input_file

    output_file = args.outputFile
    if not output_file[-5:] == ".root":
        output_file += ".root"
    output_file = output_file.replace(".root", ".subjob$1.root")

    # Check OS
    #if args.os:
    #    if not args.os in ["SLCern6", "CentOS7"]:
    #        raise ValueError("--os must be SLCern6 or CentOS7.")

    # For args.outEOS, make sure it's formatted correctly, and make sure output dir exists
    if args.outEOS:
        if args.outEOS[:6] != "/store" and args.outEOS[:5] != "/user":
            raise ValueError("Argument --outEOS must start with /store or /user (you specified --outEOS {})".format(args.outEOS))
        #if not os.path.isdir("/eos/uscms/{}".format(args.outEOS)):
        #    raise ValueError("Output EOS directory does not exist! (you specified --outEOS {}_".format(args.outEOS))
        if not args.outEOS[-1] == "/":
            args.outEOS += "/"

        # Determine eos prefix
        if args.outEOS[:6] == "/store" and host == "lxplus":
            eos_prefix_outputs = "root://eoscms.cern.ch/"
        elif args.outEOS[:5] == "/user" and host == "lxplus":
            eos_prefix_outputs = "root://eosuser.cern.ch/"
        elif host == "cmslpc":
            eos_prefix_outputs = "root://cmseos.fnal.gov"
        elif host == "cmsconnect":
            eos_prefix_outputs = "root://cmseos.fnal.gov"
        else:
            raise ValueError("Unable to determine EOS prefix")

        # Create output directory
        if host == "lxplus" or host == "cmslpc":
            import subprocess
            subp = subprocess.Popen("eos {} ls {}".format(eos_prefix_outputs, args.outEOS).split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = subp.communicate()
            if subp.returncode == 0:
                print("WARNING : EOS output directory {} already exists! Writing to existing directory, but be careful.".format(args.outEOS))
            else:
                print("Creating EOS output directory {}".format(args.outEOS))
                print("eos {} mkdir -p {}".format(eos_prefix_outputs, args.outEOS))
                subp = subprocess.Popen("eos {} mkdir {}".format(eos_prefix_outputs, args.outEOS).split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = subp.communicate()
                print(stdout)
                print(stderr)
        elif host == "cmsconnect":
            # Use gfal instead of eos
            gfal_dir = f"gsiftp://cmseos-gridftp.fnal.gov//eos/uscms/{args.outEOS}"
            import subprocess
            subp = subprocess.Popen(f"gfal-ls {gfal_dir}".split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = subp.communicate()
            if subp.returncode == 0:
                print("WARNING : EOS output directory {} already exists! Writing to existing directory, but be careful.".format(args.outEOS))
            else:
                print("Creating EOS output directory {}".format(args.outEOS))
                subp = subprocess.Popen(f"gfal-mkdir -p {gfal_dir}".split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = subp.communicate()
                print(stdout)
                print(stderr)

    # Create and move to working directory
    csub_dir = f"{args.workdir}"
    #if os.path.isdir(csub_dir) and not args.overwrite:
    #    raise ValueError("Working directory {} already exists! Specify -f to overwrite".format(csub_dir))
    os.system("mkdir -pv {}".format(csub_dir))
    cwd = os.getcwd()
    os.chdir("{}".format(csub_dir))

    # Create script that runs on the worker node
    with open(f"runOnCondor_{job_ts}.sh", 'w') as run_script:
        run_script.write("#!/bin/bash\n")
        run_script.write("ls -lrth\n")
        run_script.write("pwd\n")

        # Do everything in a subfolder (prevents accidental writebacks of output files)
        run_script.write("mkdir work\n")
        run_script.write(f"mv {cfg_basename} work\n")
        run_script.write("cd work\n")

        # Input file selection (one file per job)
        # TODO: consider writing a more flexible job splitting algorithm
        run_script.write(f"INPUTFILES=({' '.join(input_files)})\n")

        # cmsRun command
        run_script.write(f"cmsRun {cfg_basename} inputFiles=${{INPUTFILES[$1]}} outputFile={output_file} {args.extraArgs} 2>&1\n")

        # Output file handling
        if args.outEOS:
            run_script.write("xrdcp -p -f *root {}/{} \n".format(eos_prefix_outputs, args.outEOS))
        elif args.outcp:
            run_script.write("mkdir -pv {} \n".format(args.outcp))
            run_script.write("cp *root {} \n".format(args.outcp))
        elif args.gfalcp:
            run_script.write("echo \"Starting gfal-cp from $PWD\n\"")
            run_script.write("echo \"Contents of current directory:\n\"")
            run_script.write("ls -lrth \n")
            run_script.write("scram unsetenv\n")
            run_script.write("for FILENAME in ./*root; do\n")
            run_script.write("   echo \"Copying $FILENAME\"\n")
            run_script.write("   env -i bash -l -c \"export X509_USER_PROXY=$_CONDOR_SCRATCH_DIR/x509up; gfal-copy -f -p -v -t 180 file://$PWD/$FILENAME '{}/$FILENAME' 2>&1\"\n".format(args.gfalcp))
            run_script.write("done\n")
        else:
            run_script.write("mv *root $_CONDOR_SCRATCH_DIR\n")
        run_script.write("ls -lrth\n")
        run_script.write("pwd\n")
        run_script.write("ls -lrth $_CONDOR_SCRATCH_DIR\n")

    files_to_transfer = [cfg_abspath]
    csub_command = f"csub runOnCondor_{job_ts}.sh -t tomorrow --cmssw \
--mem {args.mem} \
--nCores {args.max_nthreads} \
-F {','.join(files_to_transfer)} \
--queue_n {len(input_files)} \
-x $HOME/private/x509up \
--timestamp {job_ts}"
    if args.no_retar:
        csub_command += " --no_retar"
    '''
    if not args.os:
        # Infer OS from campaign
        if "RunII" in args.campaign and not "UL" in args.campaign:
            job_os = "SLCern6"
        elif "UL" in args.campaign:
            job_os = "CentOS7"
        elif args.campaign == "NANOGEN":
            job_os = "SLCern6"
        else:
            print "Unable to infer OS from campaign {}. Using CC7.".format(args.campaign)
            job_os = "CentOS7"
    else:
        job_os = args.os
    print("Using OS {}".format(job_os))
    csub_command += " --os {}".format(job_os)
    '''
    os.system(csub_command)

    os.chdir(cwd)