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
baseFolder = r"C:\z_code\Hydro\SEIMS\model_data\dingguang\model_dingguang_30m_longterm"
baseSceneID = "0"
ScenarioIDs = ["1", "2", "3", "4"]
ScenarioNames = ["DemoExtend_ALL", "DemoExtend_PLANT", "DemoExtend_ANIMAL", "DemoExtend_SEWAGE"]
year = 2014
subbasinID = 0
# statistics base scenario outputs
baseParDict = {'COD': 0., 'TN': 0., 'TP': 0.}
for par in baseParDict.keys():
    filename = r'%s\OUTPUT%s\%d_CH_%s.txt' % (baseFolder, baseSceneID, subbasinID, par)
    baseParDict[par] = round(SumByFile(filename, year) / 1000., 2)
print "base Scene, ", baseParDict

for idx, sceneID in enumerate(ScenarioIDs):
    parDict = {'COD': 0., 'TN': 0., 'TP': 0.}
    reduceDict = {'COD': 0., 'TN': 0., 'TP': 0.}
    reduceRateDict = {'COD': 0., 'TN': 0., 'TP': 0.}
    for par in parDict.keys():
        filename = r'%s\OUTPUT%s\%d_CH_%s.txt' % (baseFolder, sceneID, subbasinID, par)
        parDict[par] = round(SumByFile(filename, year) / 1000., 2)
        reduceDict[par] = round(baseParDict[par] - parDict[par], 2)
        reduceRateDict[par] = round(reduceDict[par]/baseParDict[par], 2)
    print ScenarioNames[idx], parDict
    print "    reduce, ", reduceDict
    print "    reduceRate, ", reduceRateDict
