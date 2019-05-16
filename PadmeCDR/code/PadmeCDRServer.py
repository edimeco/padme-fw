#!/usr/bin/python

import os
import sys
import time
import subprocess
import re

from Logger import Logger

class PadmeCDRServer:

    def __init__(self,source_site,destination_site,daq_server,year,date_after,date_before,mode):

        # Get position of CDR main directory from PADME_CDR_DIR environment variable
        # Default to current dir if not set
        self.cdr_dir = os.getenv('PADME_CDR_DIR',".")

        # Get source and destination sites, name of data server, and year of data taking
        self.src_site    = source_site
        self.dst_site    = destination_site
        self.daq_server  = daq_server
        self.year        = year
        self.date_after  = date_after
        self.date_before = date_before

        # Define id of this server to be used for service files
        self.server_id = ""
        if self.src_site == "DAQ":
            self.server_id += "%s_%s_%s"%(self.src_site,self.daq_server,self.dst_site)
        else:
            self.server_id += "%s_%s"%(self.src_site,self.dst_site)

        # Redefine print to send output to log file
        self.log_file = "%s/log/PadmeCDRServer_%s.log"%(self.cdr_dir,self.server_id)
        sys.stdout = Logger(self.log_file)
        sys.stderr = sys.stdout
        if mode == "i": sys.stdout.interactive = True

        print ""
        print "### PadmeCDRServer Initializing ###"
        if (self.src_site == "DAQ"):
            print "Source: %s server %s"%(self.src_site,self.daq_server)
        else:
            print "Source: %s"%self.src_site
        print "Destination: %s"%self.dst_site
        print "Year of data taking: %s"%self.year
        if self.date_after:  print "          from date: %s"%self.date_after
        if self.date_before: print "            to date: %s"%self.date_before
        print ""

        # Define file to store list of files with transfer errors
        self.transfer_error_list_file = "%s/log/transfer_error_%s.list"%(self.cdr_dir,self.server_id)

        # Create lock file
        self.lock_file = "%s/run/lock_%s"%(self.cdr_dir,self.server_id)
        if (self.create_lock_file() == "error"): exit(1)

        # Path to long-lived generic proxy file generated by calling program
        self.long_proxy_file =  self.cdr_dir+"/run/long_proxy"

        # Path to stop_cdr file: when file appears, server will remove it and gently exit
        self.stop_cdr_file = "%s/run/stop_cdr_%s"%(self.cdr_dir,self.server_id)

        # User running CDR
        self.cdr_user = os.environ['USER']

        # Path of current year rawdata wrt top daq directory
        self.data_dir = "%s/rawdata"%self.year

        # Define minimum duration for an iteration (4 hours = 14400 seconds)
        self.iteration_minimum_duration = 14400

        ############################
        ### DAQ data server data ###
        ############################

        # Access information for DAQ data server
        self.daq_user = "daq"
        self.daq_keyfile = "/home/%s/.ssh/id_rsa_cdr"%self.cdr_user

        # Path to top daq data directory on DAQ data server
        self.daq_path = "/data/DAQ"

        # Path to adler32 command on DAQ data server
        self.daq_adler32_cmd = "/home/daq/DAQ/tools/adler32"

        # Path to current_run and last_run files on DAQ data server
        self.current_run_file = "/home/daq/DAQ/run/current_run"
        self.last_run_file = "/home/daq/DAQ/run/last_run"

        # SFTP URL for rawdata on DAQ data server
        self.daq_sftp = "sftp://%s%s"%(self.daq_server,self.daq_path)

        # SSH syntax to execute a command on the DAQ data server
        self.daq_ssh = "ssh -i %s -l %s %s"%(self.daq_keyfile,self.daq_user,self.daq_server)

        ##############################
        ### KLOE tape library data ###
        ##############################

        # Access information for KLOE front end
        self.kloe_server = "fibm15"
        self.kloe_user = "pdm"
        self.kloe_keyfile = "/home/%s/.ssh/id_rsa_cdr"%self.cdr_user

        # Path to top daq data directory on KLOE front end
        self.kloe_path = "/pdm/padme/daq"

        # Path to temporary directory on KLOE front end
        self.kloe_tmpdir = "/pdm/tmp"

        # Path to adler32 command on KLOE front end
        self.kloe_adler32_cmd = "/pdm/bin/adler32"

        # SFTP URL for rawdata on KLOE front end
        self.kloe_sftp = "sftp://%s%s"%(self.kloe_server,self.kloe_path)

        # SSH syntax to execute a command on KLOE front end
        self.kloe_ssh = "ssh -i %s -l %s %s"%(self.kloe_keyfile,self.kloe_user,self.kloe_server)

        ###################################
        ### LNF and CNAF SRM sites data ###
        ###################################

        self.site_srm = {
            "LNF" : "srm://atlasse.lnf.infn.it:8446/srm/managerv2?SFN=/dpm/lnf.infn.it/home/vo.padme.org/daq",
            "CNAF": "srm://storm-fe-archive.cr.cnaf.infn.it:8444/srm/managerv2?SFN=/padmeTape/daq"
        }

        # Initialization is finished: start the main CDR loop
        self.main_loop()

    def create_lock_file(self):

        print "- Creating lock file %s"%self.lock_file

        # Check if lock file exists
        if (os.path.exists(self.lock_file)):
            if (os.path.isfile(self.lock_file)):
                pid = 0
                with open(self.lock_file,"r") as lf:
                    for ll in lf: pid = ll

                print "Lock file %s found for pid %s - checking status"%(self.lock_file,pid)

                ppinfo = os.popen("ps -p %s"%pid)
                pinfo = ppinfo.readlines()
                ppinfo.close()
                if len(pinfo)==2:
                    if pinfo[1].find("<defunct>")>-1:
                        print "There is zombie process with this pid. The PadmeCDRServer is probably dead. Proceeding cautiously..."
                    else:
                        print "ERROR - there is already a PadmeCDRServer running with pid %s"%pid
                        return "error"
                else:
                    print "No PadmeCDRServer process found. As you were..."
            else:
                print "ERROR - Lock file %s found but it is not a file"%self.lock_file
                return "error"

        # Create our own lock file
        with open(self.lock_file,"w") as lf:
            lf.write("%d\n"%os.getpid())

        return "ok"

    def remove_lock_file(self):
        print "- Removing lock file %s"%self.lock_file
        if (os.path.exists(self.lock_file)):
            if (os.path.isfile(self.lock_file)):
                os.remove(self.lock_file)
            else:
                print "WARNING - lock file %s exists but IS NOT A FILE. Will not remove it"%self.lock_file
        else:
            print "WARNING - lock file %s DOES NOT EXIST"%self.lock_file


    def renew_voms_proxy(self):

        # Make sure proxy is valid or renew it
        # WARNING: we assume that processing a file will take less than 2 hours!

        # Generate VOMS proxy using long lived generic proxy
        renew = True

        # Check if current proxy is still valid and renew it if less than 2 hours before it expires
        for line in self.run_command("voms-proxy-info"):
            #print(line.rstrip())
            r = re.match("^timeleft  \: (\d+)\:.*$",line)
            if r and int(r.group(1))>=2: renew = False

        if renew:
            print "- Proxy is missing or will expire in less than 2 hours. Renewing it"
            cmd = "voms-proxy-init --noregen --cert %s --key %s --voms vo.padme.org --valid 24:00"%(self.long_proxy_file,self.long_proxy_file)
            for line in self.run_command(cmd): print(line.rstrip())
                # Need code to handle proxy creation errors (e.g. when long-lived proxy expired)
                # In this case we should issue some message and exit the program

    def get_ongoing_run(self):

        print "Getting on-going run from DAQ server %s"%self.daq_server

        # current_run is the last started run
        cmd = "%s \'( cat %s )\'"%(self.daq_ssh,self.current_run_file)
        for line in self.run_command(cmd): current_run = line.rstrip()
        if (current_run != "" and not re.match("run_\d+_\d+_\d+",current_run)): current_run = ""

        # last_run is the last stopped run
        cmd = "%s \'( cat %s )\'"%(self.daq_ssh,self.last_run_file)
        for line in self.run_command(cmd): last_run = line.rstrip()
        if (last_run != "" and not re.match("run_\d+_\d+_\d+",last_run)): last_run = ""

        if (current_run == "" or current_run == last_run):
            self.ongoing_run = ""
        else:
            self.ongoing_run = current_run

    def get_run_list(self,site):

        run_list = []
        if (site == "DAQ"):
            run_list = self.get_run_list_daq()
        elif (site == "LNF"):
            run_list = self.get_run_list_srm("LNF")
        elif (site == "CNAF"):
            run_list = self.get_run_list_srm("CNAF")

        # No date interval specified: just return the run list
        if (self.date_after == "" and self.date_before == ""): return run_list

        # Check which runs are within the specified date interval
        run_list_ok = []
        for run in run_list:
            m = re.match("^run_\d+_(\d+)_\d+",run)
            if m:
                date_run = m.group(1)
                if ( (self.date_after == "" or date_run >= self.date_after) and (self.date_before == "" or date_run <= self.date_before) ):
                    run_list_ok.append(run)
            else:
                # Run name is unusual: just accept it
                run_list_ok.append(run)
        return run_list_ok

    def get_run_list_daq(self):

        run_list = []

        # Make sure we do not try to transfer files while they are being written
        if (self.ongoing_run != ""): print "Run %s is on-going and will not be transferred"%self.ongoing_run

        print "Getting list of runs for year %s on DAQ server %s"%(self.year,self.daq_server)
        cmd = "%s \'( cd %s/%s; ls )\'"%(self.daq_ssh,self.daq_path,self.data_dir)
        for line in self.run_command(cmd):
            run = line.rstrip()
            if (self.ongoing_run == "" or run != self.ongoing_run): run_list.append(run)

        run_list.sort()
        return run_list

    def get_run_list_srm(self,site):
        run_list = []
        print "Getting list of runs for year %s at %s"%(self.year,site)
        cmd = "gfal-ls %s/%s"%(self.site_srm[site],self.data_dir)
        for line in self.run_command(cmd):
            if re.match("^gfal-ls error: ",line):
                print "***ERROR*** gfal-ls returned error status while retrieving run list from %s"%site
                return [ "error" ]
            run_list.append(line.rstrip())
        run_list.sort()
        return run_list

    def get_file_list(self,site,run):
        if (site == "DAQ"):
            return self.get_file_list_daq(run)
        elif (site == "KLOE"):
            return self.get_file_list_kloe(run)
        elif (site == "LNF"):
            return self.get_file_list_srm("LNF",run)
        elif (site == "CNAF"):
            return self.get_file_list_srm("CNAF",run)

    def get_file_list_daq(self,run):
        print "Getting list of raw data files for run %s on DAQ server %s"%(run,self.daq_server)
        file_list = []
        cmd = "%s \'( cd %s/%s/%s; ls *.root )\'"%(self.daq_ssh,self.daq_path,self.data_dir,run)
        for line in self.run_command(cmd):
            if re.match("^ls: cannot access",line):
                print "***ERROR*** ls returned error status while retrieving file list for run %s from DAQ"%run
                return [ "error" ]
            file_list.append("%s/%s"%(run,line.rstrip()))
        file_list.sort()
        return file_list

    def get_file_list_kloe(self,run):

        # Compile regexp to extract file name (improves performance)
        re_get_rawdata_file = re.compile("^.* %s/%s/(.*\.root) .*$"%(self.kloe_path,self.data_dir))

        print "Getting list of raw data files for run %s at KLOE"%run
        file_list = []

        # First we get list of files currently on disk buffer
        cmd = "%s \'( cd %s/%s/%s; ls )\'"%(self.kloe_ssh,self.kloe_path,self.data_dir,run)
        for line in self.run_command(cmd): file_list.append("%s/%s"%(run,line.rstrip()))

        # Second we get list of files already stored on the tape library
        cmd = "%s \'( dsmc query archive %s/%s/%s/\*.root )\'"%(self.kloe_ssh,self.kloe_path,self.data_dir,run)
        for line in self.run_command(cmd):
            m = re_get_rawdata_file.match(line)
            if (m): file_list.append(m.group(1))

        # Return list after removing duplicates and sorting
        file_list = sorted(set(file_list))
        return file_list

    def get_file_list_srm(self,site,run):
        self.renew_voms_proxy()
        print "Getting list of raw data files for run %s at %s"%(run,site)
        file_list = []
        self.check_stop_cdr()
        cmd = "gfal-ls %s/%s/%s"%(self.site_srm[site],self.data_dir,run)
        for line in self.run_command(cmd):
            m = re.match("^gfal-ls error:\s+(\d+)\s+",line)
            if m:
                # If gfal-ls error is due to missing run directory, just return an empty list
                if ( m.group(1) == "2" ): break
                # Otherwise it is a real error and is reported as such
                print line.rstrip()
                print "***ERROR*** gfal-ls returned error status %s while retrieving file list from run dir %s from %s"%(m.group(1),run,site)
                return [ "error" ]
            file_list.append("%s/%s"%(run,line.rstrip()))
        file_list.sort()
        return file_list

    def get_checksum_srm(self,site,rawfile):
        a32 = ""
        cmd = "gfal-sum %s/%s/%s adler32"%(self.site_srm[site],self.data_dir,rawfile)
        for line in self.run_command(cmd):
            print line.rstrip()
            try:
                (fdummy,a32) = line.rstrip().split()
            except:
                a32 = ""
        return a32

    def get_checksum_daq(self,rawfile):
        a32 = ""
        cmd = "%s %s %s/%s/%s"%(self.daq_ssh,self.daq_adler32_cmd,self.daq_path,self.data_dir,rawfile)
        for line in self.run_command(cmd):
            print line.rstrip()
            try:
                (a32,fdummy) = line.rstrip().split()
            except:
                a32 = ""
        return a32

    def get_checksum_kloe(self,rawfile):
        a32 = ""
        cmd = "%s %s %s/%s"%(self.kloe_ssh,self.kloe_adler32_cmd,self.kloe_tmpdir,rawfile)
        for line in self.run_command(cmd):
            print line.rstrip()
            try:
                (a32,fdummy) = line.rstrip().split()
            except:
                a32 = ""
        return a32

    def run_command(self,command):
        print "> %s"%command
        p = subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,shell=True)
        return iter(p.stdout.readline,b'')

    def check_stop_cdr(self):
        if (os.path.exists(self.stop_cdr_file)):
            if (os.path.isfile(self.stop_cdr_file)):
                print "- Stop request file %s found. Removing it and exiting..."%self.stop_cdr_file
                os.remove(self.stop_cdr_file)
            else:
                print "- WARNING - Stop request at path %s found but IT IS NOT A FILE."%self.stop_cdr_file
                print "- I will not try to remove it but I will exit anyway..."
            self.remove_lock_file()
            print ""
            print "### PadmeCDRServer ### Exiting ###"
            sys.exit(0)

    def copy_file_daq_srm(self,site,rawfile):

        self.renew_voms_proxy()

        copy_failed = False
        print "- File %s - Starting copy from DAQ to %s"%(rawfile,site)
        cmd = "gfal-copy -t 3600 -T 3600 -p -D\"SFTP PLUGIN:USER=%s\" -D\"SFTP PLUGIN:PRIVKEY=%s\" %s/%s/%s %s/%s/%s"%(self.daq_user,self.daq_keyfile,self.daq_sftp,self.data_dir,rawfile,self.site_srm[site],self.data_dir,rawfile)
        for line in self.run_command(cmd):
            print line.rstrip()
            if re.match("^gfal-copy error: ",line): copy_failed = True

        if copy_failed:
            print "- File %s - ***ERROR*** gfal-copy returned error status while copying from DAQ to %s"%(rawfile,site)
            with open(self.transfer_error_list_file,"a") as telf:
                telf.write("%s - %s copy\n"%(self.now_str(),rawfile))
            cmd = "gfal-rm %s/%s/%s"%(self.site_srm[site],self.data_dir,rawfile)
            for line in self.run_command(cmd): print line.rstrip()
            return "error"

        # Verify checksum at source and destination
        print "- File %s - Getting ADLER32 checksum at source"%rawfile
        a32_src = self.get_checksum_daq(rawfile)
        print "- File %s - Getting ADLER32 checksum at destination"%rawfile
        a32_dst = self.get_checksum_srm(site,rawfile)
        print "- File %s - ADLER32 CRC - Source: %s - Destination: %s"%(rawfile,a32_src,a32_dst)
        if ( a32_src == "" or a32_dst == "" or a32_src != a32_dst ):
            print "- File %s - ***ERROR*** unmatched checksum while copying from DAQ to %s"%(rawfile,site)
            with open(self.transfer_error_list_file,"a") as telf:
                telf.write("%s - %s checksum\n"%(self.now_str(),rawfile))
            cmd = "gfal-rm %s/%s/%s"%(self.site_srm[site],self.data_dir,rawfile)
            for line in self.run_command(cmd): print line.rstrip()
            return "error"

        return "ok"

    def copy_file_srm_srm(self,src_site,dst_site,rawfile):

        self.renew_voms_proxy()

        copy_failed = False
        print "- File %s - Starting copy from %s to %s"%(rawfile,src_site,dst_site)
        cmd = "gfal-copy -t 3600 -T 3600 -p --checksum ADLER32 %s/%s/%s %s/%s/%s"%(self.site_srm[src_site],self.data_dir,rawfile,self.site_srm[dst_site],self.data_dir,rawfile)
        for line in self.run_command(cmd):
            print line.rstrip()
            if ( re.match("^gfal-copy error: ",line) or re.match("^Command timed out",line) ):
                copy_failed = True

        if copy_failed:
            print "- File %s - ***ERROR*** gfal-copy returned error status while copying from %s to %s"%(rawfile,src_site,dst_site)
            with open(self.transfer_error_list_file,"a") as telf:
                telf.write("%s - %s copy\n"%(self.now_str(),rawfile))
            cmd = "gfal-rm %s/%s/%s"%(self.site_srm[dst_site],self.data_dir,rawfile)
            for line in self.run_command(cmd): print line.rstrip()
            return "error"

        return "ok"

    def copy_file_srm_kloe(self,site,rawfile):

        self.renew_voms_proxy()

        # Make sure destination directory exists on KLOE server
        rawdir = os.path.dirname(rawfile)
        if (rawdir == ""):
            print "- File %s - ***ERROR*** cannot extract directory from file name"%rawfile
            with open(self.transfer_error_list_file,"a") as telf:
                telf.write("%s - %s copy\n"%(self.now_str(),rawfile))
            for line in self.run_command("rm -f %s"%tmp_file): print line.rstrip()
            return "error"
        cmd = "%s \'( mkdir -p %s/%s/%s )\'"%(self.kloe_ssh,self.kloe_path,self.data_dir,rawdir)
        for line in self.run_command(cmd): print line.rstrip()

        # Name of temporary file to use during copy (will be erased after use)
        tmp_file = "/tmp/%s"%rawfile

        copy_failed = False
        print "- File %s - Starting copy from %s to KLOE"%(rawfile,site)

        # gfal-copy SFTP destination is not working: create a temporary local copy of the file
        cmd = "gfal-copy -t 3600 -T 3600 -p %s/%s/%s file://%s"%(self.site_srm[site],self.data_dir,rawfile,tmp_file)
        for line in self.run_command(cmd):
            print line.rstrip()
            if ( re.match("^gfal-copy error: ",line) or re.match("^Command timed out",line) ):
                copy_failed = True

        if copy_failed:
            print "- File %s - ***ERROR*** gfal-copy returned error status while copying from %s to local file"%(rawfile,site)
            with open(self.transfer_error_list_file,"a") as telf:
                telf.write("%s - %s copy\n"%(self.now_str(),rawfile))
            for line in self.run_command("rm -f %s"%tmp_file): print line.rstrip()
            return "error"

        # Now send local copy to KLOE temporary directory using good old scp
        cmd = "%s \'( mkdir -p %s/%s )\'"%(self.kloe_ssh,self.kloe_tmpdir,rawdir)
        for line in self.run_command(cmd): print line.rstrip()
        cmd = "scp -i %s %s %s@%s:%s/%s"%(self.kloe_keyfile,tmp_file,self.kloe_user,self.kloe_server,self.kloe_tmpdir,rawfile)
        for line in self.run_command(cmd): print line.rstrip()
        # Could add some scp error check but checksum should be enough

        # Clean up local temporary file
        cmd = "rm -f %s"%tmp_file
        for line in self.run_command(cmd): print line.rstrip()

        # Verify if the copy was correctly completed
        print "- File %s - Getting ADLER32 checksum at source"%rawfile
        a32_src = self.get_checksum_srm(site,rawfile)
        print "- File %s - Getting ADLER32 checksum at destination"%rawfile
        a32_dst = self.get_checksum_kloe(rawfile)
        print "- File %s - ADLER32 CRC - Source: %s - Destination: %s"%(rawfile,a32_src,a32_dst)
        if ( a32_src == "" or a32_dst == "" or a32_src != a32_dst ):
            print "- File %s - ***ERROR*** unmatched checksum while copying from %s to KLOE"%(rawfile,site)
            with open(self.transfer_error_list_file,"a") as telf:
                telf.write("%s - %s checksum\n"%(self.now_str(),rawfile))
            cmd = "%s \'( rm -f %s/%s )\'"%(self.kloe_ssh,self.kloe_tmpdir,rawfile)
            for line in self.run_command(cmd): print line.rstrip()
            return "error"

        # Finally move file from temporary directory to daq data directory
        cmd = "%s \'( mv %s/%s %s/%s/%s )\'"%(self.kloe_ssh,self.kloe_tmpdir,rawfile,self.kloe_path,self.data_dir,rawfile)
        for line in self.run_command(cmd): print line.rstrip()

        return "ok"

    def copy_file(self,src_site,dst_site,rawfile):

        # DAQ -> LNF or DAQ -> CNAF
        if (src_site == "DAQ"):
            if (dst_site == "LNF"):
                return self.copy_file_daq_srm("LNF",rawfile)
            elif (dst_site == "CNAF"):
                return self.copy_file_daq_srm("CNAF",rawfile)

        # LNF -> CNAF or LNF -> KLOE
        elif (src_site == "LNF"):
            if (dst_site == "CNAF"):
                return self.copy_file_srm_srm("LNF","CNAF",rawfile)
            elif (dst_site == "KLOE"):
                return self.copy_file_srm_kloe("LNF",rawfile)

        # CNAF -> LNF or CNAF -> KLOE
        elif (src_site == "CNAF"):
            if (dst_site == "LNF"):
                return self.copy_file_srm_srm("CNAF","LNF",rawfile)
            elif (dst_site == "KLOE"):
                return self.copy_file_srm_kloe("CNAF",rawfile)

        # Anything else is forbidden
        print "- WARNING - Copy from %s to %s is not supported"%(src_site,dst_site)
        return "error"

    def get_kloe_used_space(self):
        used = 100
        cmd = "%s \'( df | grep \/pdm | awk \"{print \$4}\" )\'"%self.kloe_ssh
        for line in self.run_command(cmd):
            try:
                used = int(line.rstrip()[:-1])
            except:
                print "- WARNING - Could not extract used disk space from KLOE server"
                used = 100
        return used

    def now_str(self):
        return time.strftime("%Y-%m-%d %H:%M:%S",time.gmtime())

    def main_loop(self):

        print ""
        print "### PadmeCDRServer ### Start copying ###"
        print ""

        # Loop forever waiting for new files to appear on data server
        while True:

            print ""
            print "=== PadmeCDRServer starting new iteration ==="
            print ""

            start_iteration_time = time.time()

            # Reset all file/dir lists
            self.ongoing_run = ""
            src_run_list = []
            src_file_list = []
            dst_file_list = []

            # Check on DAQ servers if a run is ongoing
            self.check_stop_cdr()
            if (self.src_site == "DAQ"): self.get_ongoing_run()

            # Get list of runs at source site
            self.check_stop_cdr()
            src_run_list = self.get_run_list(self.src_site)
            if (src_run_list and src_run_list[0] == "error"):
                print "WARNING - Source site %s has problems: suspending iteration"%self.src_site
            else:

                # Loop over all runs at source site and get list of files
                for run in src_run_list:

                    # Get list of files for this run at source site
                    self.check_stop_cdr()
                    src_file_list = self.get_file_list(self.src_site,run)
                    if (src_file_list and src_file_list[0] == "error"):
                        print "WARNING - Source site %s has problems: suspending iteration"%self.src_site
                        break

                    # Get list of files for this run at destination site
                    self.check_stop_cdr()
                    dst_file_list = self.get_file_list(self.dst_site,run)
                    if (dst_file_list and dst_file_list[0] == "error"):
                        print "WARNING - Destination site %s has problems: suspending iteration"%self.dst_site
                        break

                    suspend_iteration = False
                    for rawfile in src_file_list:
                        self.check_stop_cdr()
                        if ( not rawfile in dst_file_list ):

                            # Check disk space at KLOE front end before copying
                            if ( (self.dst_site == "KLOE") and (self.get_kloe_used_space() > 95) ):
                                print "- WARNING - KLOE disk space is more than 95% full - Suspending iteration"
                                suspend_iteration = True
                                break

                            # Copy file from source to destination
                            if self.copy_file(self.src_site,self.dst_site,rawfile) == "ok":
                                print "- File %s - Copy from %s to %s successful"%(rawfile,self.src_site,self.dst_site)
                            else:
                                print "- File %s - Copy from %s to %s failed"%(rawfile,self.src_site,self.dst_site)

                    if suspend_iteration: break

            end_iteration_time = time.time()

            print ""
            print "=== PadmeCDRServer iteration finished ==="
            print ""

            # Check if current iteration lasted a minimum time (currently 4 hours)
            # If not, pause the remaining time to avoid stress on grid data servers
            iteration_duration = end_iteration_time-start_iteration_time
            if (iteration_duration < self.iteration_minimum_duration):
                iteration_pause = self.iteration_minimum_duration-iteration_duration
                print "- Iteration lasted %ds < %ds - Sleeping %ds"%(iteration_duration,self.iteration_minimum_duration,iteration_pause)
                for _ in range(int(iteration_pause/10)):
                    self.check_stop_cdr()
                    time.sleep(10)
