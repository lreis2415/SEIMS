# -*- coding: utf-8 -*-

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from config import *
from hydroPlot import *

if __name__ == "__main__":
    LoadConfiguration(GetINIfile())

    try:
        conn = MongoClient(host=HOSTNAME, port=PORT)
    except ConnectionFailure:
        sys.stderr.write("Could not connect to MongoDB: %s" % ConnectionFailure.message)
        sys.exit(1)

    ClimateDB = conn[ClimateDBName]
    SpatialDB = conn[SpatialDBName]
    # Date Array
    '''
    TODO:
    This way is not expansible, since the time interval is not considered.
    The recommendation is:
        1. read the date of each item from simulated file.
        2. match the observed values if stated with the date.
    Plz review and update. By LJ.
    '''
    dateArr = GetDateArr(TIME_Start, TIME_End)
    # Precipatation
    '''
    TODO:
    The precipitation should be read according to the subbasin ID.
        Especially when plot a specific subbasin (such as ID 3).
        For the whole basin, the subbasin ID is 0.
    Plz review and update. By LJ.
    '''
    preci = GetPreciObs(TIME_Start, TIME_End, ClimateDB, SpatialDB)

    # simulation
    dataSimList = []
    for i in range(len(PLOT_VARS)):
        txtData = ReadSimfromTxt(TIME_Start, TIME_End, MODEL_DIR, PLOT_VARS[i], PLOT_SUBBASINID)
        dataSimList.append(txtData)

    # # Create multiple plot
    # sim_flow = ReadSimfromTxt(TIME_Start, TIME_End, MODEL_DIR, "Q")
    # SearchObs(TIME_Start, TIME_End, 'Q', ClimateDB)
    # CreatePlot(dateArr, sim_flow, preci, dataSimList, PLOT_VARS, MODEL_DIR, ClimateDB)

    # Create multi hydrographs, updated by LJ
    CreatePlot2(dateArr, preci, dataSimList, PLOT_VARS, MODEL_DIR, ClimateDB)

