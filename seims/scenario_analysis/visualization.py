#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Visualization of Scenarios, which is relative independent with SA.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-09-12  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""
import os
import matplotlib
if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy


def plot_pareto_front(pop, ws, pop_size, gen_id):
    front = numpy.array([ind.fitness.values for ind in pop])
    # Plot
    plt.figure(gen_id)
    plt.title('Pareto frontier of Scenarios Optimization\n', color='#aa0903')
    # plt.xlabel('Economic calculate_economy(Million Yuan)')
    plt.xlabel('Economic effectiveness')
    plt.ylabel('Environmental effectiveness')
    # front[:, 0] /= 1000000.
    # front[:, 1] /= 1000.
    plt.scatter(front[:, 0], front[:, 1], c='r', alpha=0.8, s=12)
    plt.title('\nPopulation: %d, Generation: %d' % (pop_size, gen_id), color='green', fontsize=9,
              loc='right')
    img_path = ws + os.sep + 'Pareto_Gen_%d_Pop_%d.png' % (gen_id, pop_size)
    plt.savefig(img_path)
    # plt.show()
