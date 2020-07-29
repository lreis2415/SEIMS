#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Replace the relative links by global paths for Doxygen.

    @author   : Liang-Jun Zhu

    @changelog:
    - 18-08-08  - lj - Initial implementation.
    - 18-08-10  - lj - Add identification of images with png or jpg formats.
    - 20-07-29  - lj - Check the existence of the replaced file path, if not, then do not replace.
"""
import os
import re
from inspect import getsourcefile
from io import open


def current_path(local_function):
    """Get current path
    Reference: https://stackoverflow.com/questions/2632199/how-do-i-get-the-path-of-the-current-executed-file-in-python/18489147#18489147
    Examples:
        curpath = current_path(lambda: 0)
    """
    fpath = getsourcefile(local_function)
    if fpath is None:
        return None
    return os.path.dirname(os.path.abspath(fpath))


CPATH = current_path(lambda: 0)


def regrex_find_md_link(test_str, prefix):
    # regex = r"(?:(\[.+?\]\(.+?\.md\)))" # previous version
    regex = r"(?:(!?\[.+?\]\(.+?(.md|.png|.jpg|.tiff|.pdf)\)))"
    matches = re.finditer(regex, test_str, re.IGNORECASE)
    replaced_str = test_str[:]
    flag = False
    for matchNum, match in enumerate(matches):
        print('Match {matchNum} was found at {start}-{end}: {match}'.format(matchNum=matchNum + 1,
                                                                            start=match.start(),
                                                                            end=match.end(),
                                                                            match=match.group()))
        substr = test_str[match.start():match.end()]
        if 'https://' in substr or 'http://' in substr or 'ftp://' in substr:
            continue
        i = substr.find('](')
        j = substr.rfind(')')
        oldpath = substr[i + 2:j]
        newpath = os.path.normpath(prefix + oldpath)

        if os.path.exists('%s/../%s' % (CPATH, newpath)):
            k = replaced_str.find(substr)
            replaced_str = replaced_str[: i + k + 2] + newpath + replaced_str[j:]
            flag = True
    if flag:
        print('origin: %s, replaced: %s' % (test_str, replaced_str))
    return replaced_str, flag


def replace_relative_paths(filepath, relative_path):
    print('Replace relative paths of %s' % filepath)
    lines = list()
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            lines.append(line)
    rewrite = False
    new_lines = list()
    for line in lines:
        replaced_line, flag = regrex_find_md_link(line, relative_path)
        new_lines.append(replaced_line)
        if flag:
            rewrite = True
    if rewrite:
        with open(filepath, 'w', encoding='utf-8') as f:
            for line in new_lines:
                f.write(line)


def main(fpath, relative_path):
    entries = os.listdir(fpath)
    for entry in entries:
        tmppath = fpath + os.sep + entry
        if os.path.isdir(tmppath):
            main(tmppath, '%s%s/' % (relative_path, entry))
        else:
            name, ext = os.path.splitext(entry)
            if not ext.upper() == '.MD':
                continue
            if 'SUMMARY' == name.upper():
                continue
            replace_relative_paths(tmppath, relative_path)


if __name__ == '__main__':
    # cpath = current_path(lambda: 0)
    topname = os.path.split(CPATH)[-1] + '/'  # By default this should be 'doc/'
    langs = ['en', 'zh-cn']
    for lang in langs:
        nextdir = CPATH + os.sep + lang
        if not os.path.exists(nextdir):
            print('%s is not existed!' % nextdir)
            continue
        main(nextdir, '%s%s/' % (topname, lang))
