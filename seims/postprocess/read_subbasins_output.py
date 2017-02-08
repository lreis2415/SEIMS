# -*- coding: utf-8 -*-
import os


def read_subbasins_output(files, years):
    outputs = {}
    for vname, fnames in files.items():
        if vname not in outputs.keys():
            outputs[vname] = {}
        for fname in fnames:
            f = open(fname)
            lines = f.readlines()
            f.close()

            cursubbasinID = -1
            for line in lines:
                items = line.split()
                # print (items)
                if len(items) == 2 and items[0] == 'Subbasin:':  # indicate a new subbasin
                    cursubbasinID = int(items[1])
                    # print (cursubbasinID)
                    if cursubbasinID not in outputs[vname].keys():
                        outputs.get(vname)[cursubbasinID] = {}
                    continue
                if len(items) < 3:
                    continue
                if cursubbasinID < 0:
                    continue
                for year in years:
                    if items[0][:4] == str(year):
                        if year not in outputs.get(vname)[cursubbasinID].keys():
                            outputs.get(vname)[cursubbasinID][year] = 0.
                        outputs.get(vname)[cursubbasinID][year] += float(items[-1])
    return outputs


if __name__ == "__main__":
    modeloutputdir = r'C:\z_code\Hydro\SEIMS\model_data\dingguang\model_dingguang_30m_longterm\OUTPUT0'
    inputfiles = {'TNout': ['0_CH_TN'], 'TPout': ['0_CH_TP'], 'CODout': ['0_CH_COD'],
                  'TNin': ['0_SUR_NH4_TOCH', '0_SUR_NO3_TOCH', '0_SEDORGNTOCH', '0_NO3GWTOCH',
                           '0_LATNO3TOCH', '0_ptTNToCh'],
                  'TPin': ['0_SUR_SOLP_TOCH', '0_SEDMINPATOCH', '0_SEDMINPSTOCH', '0_SEDORGPTOCH',
                           '0_MINPGWTOCH', '0_ptTPToCh'],
                  'CODin': ['0_SUR_COD_TOCH', '0_ptCODTOCH']}
    years = [2014]
    strs = "subbasin\\variable-year "
    for vname in inputfiles.keys():
        for year in years:
            strs += " %s-%d" % (vname, year)
        for i, fname in enumerate(inputfiles.get(vname)):
            inputfiles.get(vname)[i] = modeloutputdir + os.sep + fname + ".txt"
    outputs = read_subbasins_output(inputfiles, years)

    print (strs)  # header line
    printdict = {}
    for vname, vdict in outputs.items():
        for subid, subdict in vdict.items():
            if subid not in printdict.keys():
                printdict[subid] = "%d" % subid
            for year in years:
                printdict[subid] += " %f" % subdict.get(year)
    for subid, subvalues in printdict.items():
        print ("%s" % (subvalues))

