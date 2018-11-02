#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Utility functions of DEAP package.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29 - lj - Extract from other packages.
"""
import os
import scoop


def scoop_log(msg):
    if os.name != 'nt':
        scoop.logger.warn(msg)
    else:
        print(msg)
