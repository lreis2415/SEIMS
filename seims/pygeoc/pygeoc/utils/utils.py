#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Utility Classes and Functions

    author: Liangjun Zhu
    changlog: 12-04-12 jz - origin version
              16-07-01 lj - reorganized for pygeoc
              17-06-25 lj - check by pylint and reformat by Google style
"""

import argparse
import glob
import os
import platform
import socket
import subprocess
import sys
import time
from math import sqrt
from shutil import copy, rmtree

sysstr = platform.system()

# Global constants
SQ2 = 1.4142135623730951
PI = 3.141592653589793
ZERO = 1e-12
DELTA = 1e-6
DEFAULT_NODATA = -9999.


class MathClass(object):
    """Basic math related."""

    def __init__(self):
        pass

    @staticmethod
    def isnumerical(x):
        """Check the input x is numerical or not."""
        try:
            xx = float(x)
        except TypeError:
            return False
        except ValueError:
            return False
        except Exception:
            return False
        else:
            return True

    @staticmethod
    def floatequal(a, b):
        """If float a is equal to float b."""
        return abs(a - b) < DELTA

    @staticmethod
    def nashcoef(obsvalues, simvalues):
        """Calculate Nash coefficient.
        Args:
            obsvalues: observe values array
            simvalues: simulate values array

        Returns:
            NSE, or raise exception
        """
        if len(obsvalues) != len(simvalues):
            raise ValueError("The size of observed and simulated values must be"
                             " the same for NSE calculation!")
        n = len(obsvalues)
        ave = sum(obsvalues) / n
        a1 = 0.
        a2 = 0.
        for i in range(n):
            a1 += pow(float(obsvalues[i]) - float(simvalues[i]), 2.)
            a2 += pow(float(obsvalues[i]) - ave, 2.)
        if a2 == 0.:
            return 1.
        return 1. - a1 / a2

    @staticmethod
    def rsquare(obsvalues, simvalues):
        """Calculate R-square.
        Args:
            obsvalues: observe values array
            simvalues: simulate values array

        Returns:
            R-square value, or raise exception
        """
        if len(obsvalues) != len(simvalues):
            raise ValueError("The size of observed and simulated values must be "
                             "the same for R-square calculation!")
        n = len(obsvalues)
        obs_avg = sum(obsvalues) / n
        pred_avg = sum(simvalues) / n
        obs_minus_avg_sq = 0.
        pred_minus_avg_sq = 0.
        obs_pred_minus_avgs = 0.
        for i in range(n):
            obs_minus_avg_sq += pow((obsvalues[i] - obs_avg), 2.)
            pred_minus_avg_sq += pow((simvalues[i] - pred_avg), 2.)
            obs_pred_minus_avgs += (obsvalues[i] - obs_avg) * (simvalues[i] - pred_avg)
        # Calculate R-square
        yy = (pow(obs_minus_avg_sq, 0.5) * pow(pred_minus_avg_sq, 0.5))
        if yy == 0.:
            return 1.
        return pow((obs_pred_minus_avgs / yy), 2.)

    @staticmethod
    def rmse(list1, list2):
        """Calculate RMSE.
        Args:
            list1: values list 1
            list2: values list 2

        Returns:
            RMSE value
        """
        n = len(list1)
        s = 0.
        for i in range(n):
            s += pow(list1[i] - list2[i], 2.)
        return sqrt(s / n)


class StringClass(object):
    """String handling class
    """

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def string_match(str1, str2):
        """Compare two string regardless capital or not"""
        return str1.lower() == str2.lower()

    @staticmethod
    def strip_string(str_src):
        """Remove space(' ') and indent('\t') at the begin and end of the string."""
        old_str = ''
        new_str = str_src
        while old_str != new_str:
            old_str = new_str
            new_str = old_str.strip('\t')
            new_str = new_str.strip(' ')
        return new_str

    @staticmethod
    def split_string(str_src, spliters=None):
        """Split string by split character space(' ') and indent('\t') as default
        Args:
            str_src: source string
            spliters: e.g. [' ', '\t'], []

        Returns:
            split sub-strings as list
        """
        if spliters is None or not spliters:
            spliters = [' ', '\t']
        dest_strs = []
        src_strs = [str_src]
        while True:
            old_dest_strs = src_strs[:]
            for s in spliters:
                for src_s in src_strs:
                    temp_strs = src_s.split(s)
                    for temp_s in temp_strs:
                        temp_s = StringClass.strip_string(temp_s)
                        if temp_s != '':
                            dest_strs.append(temp_s)
                src_strs = dest_strs[:]
                dest_strs = []
            if old_dest_strs == src_strs:
                dest_strs = src_strs[:]
                break
        return dest_strs

    @staticmethod
    def is_substring(substr, str_src):
        """Is substr part of str_src, case insensitive."""
        return substr.lower() in str_src.lower()

    @staticmethod
    def string_in_list(tmp_str, strlist):
        """Is tmp_str in strlist, case insensitive."""
        new_str_list = strlist[:]
        for i, str_in_list in enumerate(new_str_list):
            new_str_list[i] = str_in_list.lower()
        return tmp_str.lower() in new_str_list

    @staticmethod
    def is_valid_ip_addr(address):
        """Check the validation of IP address"""
        try:
            socket.inet_aton(address)
            return True
        except Exception:
            return False


class FileClass(object):
    """File IO related"""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def is_file_exists(filename):
        """Check the existence of file or folder path"""
        if filename is None or not os.path.exists(filename):
            return False
        else:
            return True

    @staticmethod
    def check_file_exists(filename):
        """Throw exception if the file not existed"""
        if not FileClass.is_file_exists(filename):
            UtilClass.error("Input files path %s is None or not existed!\n" % filename)

    @staticmethod
    def copy_files(filename, dstfilename):
        """Copy files with the same name and different suffixes, such as ESRI Shapefile."""
        FileClass.remove_files(dstfilename)
        dst_prefix = os.path.splitext(dstfilename)[0]
        pattern = os.path.splitext(filename)[0] + '.*'
        for f in glob.iglob(pattern):
            ext = os.path.splitext(f)[1]
            dst = dst_prefix + ext
            copy(f, dst)

    @staticmethod
    def remove_files(filename):
        """
        Delete all files with same root as fileName,
        i.e. regardless of suffix, such as ESRI shapefile
        """
        pattern = os.path.splitext(filename)[0] + '.*'
        for f in glob.iglob(pattern):
            os.remove(f)

    @staticmethod
    def is_up_to_date(outfile, basedatetime):
        """Return true if outfile exists and is no older than base datetime."""
        if os.path.exists(outfile):
            if os.path.getmtime(outfile) >= basedatetime:
                return True
        return False

    @staticmethod
    def get_executable_fullpath(name):
        """get the full path of a given executable name"""
        if sysstr == 'Windows':  # not test yet
            findout = UtilClass.run_command('where %s' % name)
        else:
            findout = UtilClass.run_command('which %s' % name)
        if findout == [] or len(findout) == 0:
            print ("%s is not included in the env path" % name)
            exit(-1)
        return findout[0].split('\n')[0]

    @staticmethod
    def get_filename_by_suffixes(dir_src, suffixes):
        """get file names with the given suffixes in the given directory
        Args:
            dir_src: directory path
            suffixes: wanted suffixes

        Returns:
            file names with the given suffixes as list
        """
        list_files = os.listdir(dir_src)
        re_files = []
        for f in list_files:
            name, ext = os.path.splitext(f)
            if StringClass.string_in_list(ext, suffixes):
                re_files.append(f)
        return re_files

    @staticmethod
    def get_full_filename_by_suffixes(dir_src, suffixes):
        """get full file names with the given suffixes in the given directory
        Args:
            dir_src: directory path
            suffixes: wanted suffixes

        Returns:
            full file names with the given suffixes as list
        """
        full_paths = []
        for name in FileClass.get_filename_by_suffixes(dir_src, suffixes):
            full_paths.append(dir_src + os.sep + name)
        return full_paths

    @staticmethod
    def get_core_name_without_suffix(file_path):
        """Return core file name without suffix."""
        file_name = os.path.basename(file_path)
        core_name = file_name.split('.')[0]
        return core_name


class DateClass(object):
    """Utility function to handle datetime."""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def is_leapyear(year):
        """Is leap year?"""
        return (year % 4 == 0 and year % 100 != 0) or (year % 400 == 0)

    @staticmethod
    def day_of_month(year, month):
        """Day number of month"""
        if month in [1, 3, 5, 7, 8, 10, 12]:
            return 31
        elif month in [4, 6, 9, 11]:
            return 30
        elif DateClass.is_leapyear(year):
            return 29
        else:
            return 28

    @staticmethod
    def day_of_year(dt):
        """Day index of year from 1 to 365 or 366"""
        sec = time.mktime(dt.timetuple())
        t = time.localtime(sec)
        return t.tm_yday


class UtilClass(object):
    """Other common used utility functions"""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def run_command(commands):
        """Execute external command, and return the output lines list
        17-07-04 lj - Handling subprocess crash in Windows, refers to
            https://stackoverflow.com/questions/5069224/handling-subprocess-crash-in-windows
        Args:
            commands: string or list

        Returns:
            output lines
        """
        print (commands)
        use_shell = True
        if isinstance(commands, list) or isinstance(commands, tuple):
            use_shell = False
        if sysstr == 'Windows':
            import ctypes
            SEM_NOGPFAULTERRORBOX = 0x0002  # From MSDN
            ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
            subprocess_flags = 0x8000000  # win32con.CREATE_NO_WINDOW?
        else:
            subprocess_flags = 0
        process = subprocess.Popen(commands, shell=use_shell, stdout=subprocess.PIPE,
                                   stdin=open(os.devnull),
                                   stderr=subprocess.STDOUT, universal_newlines=True,
                                   creationflags=subprocess_flags)

        if process.stdout is None:
            return ['']
        out_lines = process.stdout.readlines()
        err = False
        for line in out_lines:
            if 'ERROR' in line.upper():
                err = True
                break
        if err or (process.returncode is not None and process.returncode != 0):
            raise subprocess.CalledProcessError(-1, commands,
                                                "ERROR occurred when running subprocess!")
        return out_lines

    @staticmethod
    def current_path():
        """Get current path"""
        path = sys.path[0]
        if os.path.isdir(path):
            return path
        elif os.path.isfile(path):
            return os.path.dirname(path)

    @staticmethod
    def mkdir(dir_path):
        """Make directory if not existed"""
        if not os.path.isdir(dir_path) or not os.path.exists(dir_path):
            os.mkdir(dir_path)

    @staticmethod
    def rmmkdir(dir_path):
        """If directory existed, then remove and make; else make it."""
        if not os.path.isdir(dir_path):
            os.mkdir(dir_path)
        else:
            rmtree(dir_path, True)
            os.mkdir(dir_path)

    @staticmethod
    def print_msg(contentlist):
        """concatenate message list as single string"""
        if isinstance(contentlist, list) or isinstance(contentlist, tuple):
            contentstr = ''
            for content in contentlist:
                contentstr += "%s\n" % content
            return contentstr
        else:
            return contentlist

    @staticmethod
    def error(msg):
        """throw RuntimeError exception"""
        raise RuntimeError(msg)

    @staticmethod
    def writelog(logfile, contentlist, mode='replace'):
        """write log"""
        if os.path.exists(logfile):
            if mode == 'replace':
                os.remove(logfile)
                log_status = open(logfile, 'w')
            else:
                log_status = open(logfile, 'a')
        else:
            log_status = open(logfile, 'w')
        log_status.write(UtilClass.print_msg(contentlist))
        log_status.flush()
        log_status.close()


class C(object):
    """Empty"""
    pass


def get_config_file():
    """Get model configuration file name from argv"""
    c = C()
    parser = argparse.ArgumentParser(description="Read configuration file.")
    parser.add_argument('-ini', help="Full path of configuration file")
    args = parser.parse_args(namespace=c)
    ini_file = args.ini
    if not FileClass.is_file_exists(ini_file):
        print ("Usage: -ini <full path to the configuration file.>")
        exit(0)
    return ini_file
