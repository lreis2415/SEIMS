#! /usr/bin/env python
# coding=utf-8
# @Get soil texture property, based on WetSpa
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#


def GetTexture(clay, silt, sand):
    # silt = 100 - sand - clay
    if (clay >= 40 and silt <= 40 and sand <= 45):
        return [12, 1, 0.22]  # clay / nian tu
    elif (clay >= 40 and silt >= 40):
        return [11, 1, 0.26]  # silt caly / fen nian tu
    elif (clay >= 35) and (sand >= 45):
        return [10, 1, 0.28]  # sandy clay / sha nian tu
    elif (clay >= 25 and sand <= 45 and sand >= 20):
        return [9, 2, 0.3]  # clay loam / nian rang tu
    elif (clay >= 25 and sand <= 20):
        return [8, 2, 0.32]  # silt clay loam / fen zhi nian rang tu
    elif (clay >= 20 and silt <= 30 and sand >= 45):
        return [7, 2, 0.2]  # sandy clay loam / sha zhi nian rang tu
    elif (clay >= 10) and (silt >= 30) and (silt <= 50) and (sand <= 50):
        return [6, 3, 0.3]  # loam / rang tu
    elif (silt >= 50 and silt <= 80) or (clay >= 15 and silt >= 80):
        return [4, 3, 0.38]  # silt loam / fen zhi rang tu
    elif (silt >= 80 and clay <= 15):
        return [5, 3, 0.34]  # silt / fen tu
    elif (clay <= 10 and sand <= 50) or (sand >= 50 and sand <= 80):
        return [3, 4, 0.13]  # sandy loam / sha zhi rang tu
    elif (sand <= 90):
        return [2, 4, 0.04]  # loamy sand / rang zhi sha tu
    else:
        return [1, 4, 0.02]  # sand / sha tu


if __name__ == "__main__":
    print GetTexture(60, 10, 30)
