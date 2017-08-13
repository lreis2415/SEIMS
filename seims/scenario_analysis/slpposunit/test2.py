# -*- coding: utf-8 -*-

from mpl_toolkits.mplot3d.axes3d import Axes3D
import matplotlib.pyplot as plt
# imports specific to the plots in this example
import numpy


# fig = plt.figure()
# ax = Axes3D(fig)
# X = np.arange(-3.0, 12.1, 0.08)
# Y = np.arange(-4.1, 5.8, 0.08)
# X, Y = np.meshgrid(X, Y)
# Z = -20*np.exp(-0.2*np.sqrt(np.sqrt((X**2+Y**2)/2)))+20+np.e-np.exp((np.cos(2*np.pi*X)+np.sin(2*np.pi*Y))/2)
# ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap='cool')
# plt.show()
def Count(arr, size, x, noDataValue):
    frequency = 0
    if (x == noDataValue):
        return 0
    for i in range(size):
        if (arr[i] == x):
            frequency = frequency + 1
    return frequency

def Mode(arr, size, noDataValue):
    maxv = arr[0]
    rangev = []
    for i in range(size):
        if (maxv < Count(arr, size, arr[i], noDataValue)):
            maxv = arr[i]
            rangev.append(arr[i])
    # print maxv
    if maxv != noDataValue:
        return maxv
    else:
        return numpy.sort(rangev)[-1]
