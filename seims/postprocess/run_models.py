# -*- coding: utf-8 -*-

def SumByFile(filename, year):
    f = open(filename)
    lines = f.readlines()
    f.close()

    s = 0
    for line in lines:
        items = line.split()
        if len(items) < 3:
            continue
        if items[0][:4] == str(year):
            s += float(items[-1])
    return s


dirlist = []
# parlist = ['COD', 'TN', 'TP']
# solidReduction = [611.76, 118.98, 60.5]
parDict = {'COD': 687., 'TN': 127., 'TP': 62.}
baseFolder = r"C:\z_code\Hydro\SEIMS\model_data\dianbu"
watershedList = ["", "2", "3"]
outputList = ["2", "3"]

year = 2014
for par in parDict.keys():
    varTotalPre = 0.
    varTotalAfter = 0.
    for wsID in watershedList:
        filenamePre = r'%s\model_dianbu%s_30m_longterm\OUTPUT%s\0_CH_%s.txt' % (baseFolder, wsID, 3, par)
        filenameAfter = r'%s\model_dianbu%s_30m_longterm\OUTPUT%s\0_CH_%s.txt' % (baseFolder, wsID, 2, par)
        varTotalPre += SumByFile(filenamePre, year)
        varTotalAfter += SumByFile(filenameAfter, year)
    liquid = (varTotalPre - varTotalAfter) / 1000.
    total = parDict.get(par) + liquid
    print "%s: Base: %.2f, DemoExtended: %.2f, L: %.2f, Total: %.2f, ReduceRate: %.2f" % \
          (par, varTotalPre, varTotalAfter, liquid, total, liquid * 100. / varTotalPre * 1000.)
