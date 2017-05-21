

def merge_subbasins(self):
    """
    Merged selected subbasin with its parent.
    :param self:
    :return:
    """
    self.delineationFinishedOK = False
    li = self._iface.legendInterface()
    demLayer = QSWATUtils.getLayerByFilenameOrLegend(li.layers(), self._gv.demFile, FileTypes._DEM, '', self._dlg)
    if not demLayer:
        QSWATUtils.error('Cannot find DEM layer', self._gv.isBatch)
        return
    wshedLayer = QSWATUtils.getLayerByFilenameOrLegend(li.layers(), self._gv.wshedFile, FileTypes._SUBBASINS, '',
                                                       self._dlg)
    if not wshedLayer:
        QSWATUtils.error('Cannot find subbasins layer', self._gv.isBatch)
        return
    streamLayer = QSWATUtils.getLayerByFilenameOrLegend(li.layers(), self._gv.streamFile, FileTypes._STREAMS, '',
                                                        self._dlg)
    if not streamLayer:
        QSWATUtils.error('Cannot find stream reaches layer', self._gv.isBatch)
        wshedLayer.removeSelection()
        return
    selection = wshedLayer.selectedFeatures()
    if len(selection) == 0:
        QSWATUtils.information("Please select at least one subbasin to be merged", self._gv.isBatch)
        return
    outletLayer = QSWATUtils.getLayerByFilenameOrLegend(li.layers(), self._gv.outletFile, FileTypes._OUTLETS, '',
                                                        self._dlg)

    polygonidField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._POLYGONID)
    if polygonidField < 0:
        return
    areaField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._AREA, ignoreMissing=True)
    streamlinkField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._STREAMLINK, ignoreMissing=True)
    streamlenField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._STREAMLEN, ignoreMissing=True)
    dsnodeidwField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._DSNODEIDW, ignoreMissing=True)
    dswsidField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._DSWSID, ignoreMissing=True)
    us1wsidField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._US1WSID, ignoreMissing=True)
    us2wsidField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._US2WSID, ignoreMissing=True)
    subbasinField = self._gv.topo.getIndex(wshedLayer, QSWATTopology._SUBBASIN, ignoreMissing=True)
    linknoField = self._gv.topo.getIndex(streamLayer, QSWATTopology._LINKNO)
    if linknoField < 0:
        return
    dslinknoField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DSLINKNO)
    if dslinknoField < 0:
        return
    uslinkno1Field = self._gv.topo.getIndex(streamLayer, QSWATTopology._USLINKNO1, ignoreMissing=True)
    uslinkno2Field = self._gv.topo.getIndex(streamLayer, QSWATTopology._USLINKNO2, ignoreMissing=True)
    dsnodeidnField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DSNODEID, ignoreMissing=True)
    orderField = self._gv.topo.getIndex(streamLayer, QSWATTopology._ORDER, ignoreMissing=True)
    lengthField = self._gv.topo.getIndex(streamLayer, QSWATTopology._LENGTH, ignoreMissing=True)
    magnitudeField = self._gv.topo.getIndex(streamLayer, QSWATTopology._MAGNITUDE, ignoreMissing=True)
    ds_cont_arField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DS_CONT_AR, ignoreMissing=True)
    dropField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DROP, ignoreMissing=True)
    slopeField = self._gv.topo.getIndex(streamLayer, QSWATTopology._SLOPE, ignoreMissing=True)
    straight_lField = self._gv.topo.getIndex(streamLayer, QSWATTopology._STRAIGHT_L, ignoreMissing=True)
    us_cont_arField = self._gv.topo.getIndex(streamLayer, QSWATTopology._US_CONT_AR, ignoreMissing=True)
    wsnoField = self._gv.topo.getIndex(streamLayer, QSWATTopology._WSNO)
    if wsnoField < 0:
        return
    dout_endField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DOUT_END, ignoreMissing=True)
    dout_startField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DOUT_START, ignoreMissing=True)
    dout_midField = self._gv.topo.getIndex(streamLayer, QSWATTopology._DOUT_MID, ignoreMissing=True)
    if outletLayer:
        nodeidField = self._gv.topo.getIndex(outletLayer, QSWATTopology._ID, ignoreMissing=True)
        srcField = self._gv.topo.getIndex(outletLayer, QSWATTopology._PTSOURCE, ignoreMissing=True)
        resField = self._gv.topo.getIndex(outletLayer, QSWATTopology._RES, ignoreMissing=True)
        inletField = self._gv.topo.getIndex(outletLayer, QSWATTopology._INLET, ignoreMissing=True)
    # ids of the features will change as we delete them, so use polygonids, which we know will be unique
    pids = []
    for f in selection:
        pid = f.attributes()[polygonidField]
        pids.append(int(pid))
    # in the following
    # suffix A refers to the subbasin being merged
    # suffix UAs refers to the subbasin(s) upstream from A
    # suffix D refers to the subbasin downstream from A
    # suffix B refers to the othe subbasin(s) upstream from D
    # suffix M refers to the merged basin
    self._gv.writeMasterProgress(0, 0)
    for polygonidA in pids:
        wshedA = QSWATUtils.getFeatureByValue(wshedLayer, polygonidField, polygonidA)
        wshedAattrs = wshedA.attributes()
        reachA = QSWATUtils.getFeatureByValue(streamLayer, wsnoField, polygonidA)
        if not reachA:
            QSWATUtils.error('Cannot find reach with {0} value {1!s}'.format(QSWATTopology._WSNO, polygonidA),
                             self._gv.isBatch)
            continue
        reachAattrs = reachA.attributes()
        QSWATUtils.loginfo('A is reach {0!s} polygon {1!s}'.format(reachAattrs[linknoField], polygonidA))
        AHasOutlet = False
        AHasInlet = False
        AHasReservoir = False
        AHasSrc = False
        if dsnodeidnField >= 0:
            dsnodeidA = reachAattrs[dsnodeidnField]
            if outletLayer:
                pointFeature = QSWATUtils.getFeatureByValue(outletLayer, nodeidField, dsnodeidA)
                if pointFeature:
                    attrs = pointFeature.attributes()
                    if inletField >= 0 and attrs[inletField] == 1:
                        if srcField >= 0 and attrs[srcField] == 1:
                            AHasSrc = True
                        else:
                            AHasInlet = True
                    elif resField >= 0 and attrs[resField] == 1:
                        AHasReservoir = True
                    else:
                        AHasOutlet = True
        if AHasOutlet or AHasInlet or AHasReservoir or AHasSrc:
            QSWATUtils.information(
                'You cannot merge a subbasin which has an outlet, inlet, reservoir, or point source.  Not merging subbasin with {0} value {1!s}'.format(
                    QSWATTopology._POLYGONID, polygonidA), self._gv.isBatch)
            continue
        linknoA = reachAattrs[linknoField]
        reachUAs = [reach for reach in streamLayer.getFeatures() if reach.attributes()[dslinknoField] == linknoA]
        # check whether a reach immediately upstream from A has an inlet
        inletUpFromA = False
        if dsnodeidnField >= 0 and outletLayer:
            for reachUA in reachUAs:
                reachUAattrs = reachUA.attributes()
                dsnodeidUA = reachUAattrs[dsnodeidnField]
                pointFeature = QSWATUtils.getFeatureByValue(outletLayer, nodeidField, dsnodeidUA)
                if pointFeature:
                    attrs = pointFeature.attributes()
                    if inletField >= 0 and attrs[inletField] == 1 and (srcField < 0 or attrs[srcField] == 0):
                        inletUpFromA = True
                        break
        linknoD = reachAattrs[dslinknoField]
        reachD = QSWATUtils.getFeatureByValue(streamLayer, linknoField, linknoD)
        if not reachD:
            QSWATUtils.information('No downstream subbasin from subbasin with {0} value {1!s}: nothing to merge'.format(
                QSWATTopology._POLYGONID, polygonidA), self._gv.isBatch)
            continue
        reachDattrs = reachD.attributes()
        polygonidD = reachDattrs[wsnoField]
        QSWATUtils.loginfo('D is reach {0!s} polygon {1!s}'.format(linknoD, polygonidD))
        # reachD may be zero length, with no corresponding subbasin, so search downstream if necessary to find wshedD
        # at the same time collect zero-length reaches for later disposal
        wshedD = None
        nextReach = reachD
        zeroReaches = []
        while not wshedD:
            polygonidD = nextReach.attributes()[wsnoField]
            wshedD = QSWATUtils.getFeatureByValue(wshedLayer, polygonidField, polygonidD)
            if wshedD:
                break
            # nextReach has no subbasin (it is a zero length link); step downstream and try again
            # first make a check
            if lengthField >= 0 and nextReach.attributes()[lengthField] > 0:
                QSWATUtils.error(
                    'Internal error: stream reach wsno {0!s} has positive length but no subbasin.  Not merging subbasin with {1} value {2!s}'.format(
                        polygonidD, QSWATTopology._POLYGONID, polygonidA), self._gv.isBatch)
                continue
            if zeroReaches:
                zeroReaches.append(nextReach)
            else:
                zeroReaches = [nextReach]
            nextLink = nextReach.attributes()[dslinknoField]
            if nextLink < 0:
                # reached main outlet
                break
            nextReach = QSWATUtils.getFeatureByValue(streamLayer, linknoField, nextLink)
        if not wshedD:
            QSWATUtils.information('No downstream subbasin from subbasin with {0} value {1!s}: nothing to merge'.format(
                QSWATTopology._POLYGONID, polygonidA), self._gv.isBatch)
            continue
        wshedDattrs = wshedD.attributes()
        reachD = nextReach
        reachDattrs = reachD.attributes()
        linknoD = reachDattrs[linknoField]
        zeroLinks = [reach.attributes()[linknoField] for reach in zeroReaches]
        if inletUpFromA:
            DLinks = [linknoD].extend(zeroLinks) if zeroLinks else [linknoD]
            reachBs = [reach for reach in streamLayer.getFeatures() if
                       reach.attributes()[dslinknoField] in DLinks and reach.id() != reachA.id()]
            if reachBs != []:
                QSWATUtils.information(
                    'Subbasin with {0} value {1!s} has an upstream inlet and the downstream one has another upstream subbasin: cannot merge.'.format(
                        QSWATTopology._POLYGONID, polygonidA), self._gv.isBatch)
                continue
        # have reaches and watersheds A, UAs, D
        # we are ready to start editing the streamLayer
        OK = True
        try:
            OK = streamLayer.startEditing()
            if not OK:
                QSWATUtils.error('Cannot edit stream reaches shapefile', self._gv.isBatch)
                return
            if reachUAs == []:
                # A is a head reach (nothing upstream) and can be deleted
                # change any dslinks to zeroLinks to D as the zeroReaches will be deleted
                if zeroLinks:
                    for reach in streamLayer.getFeatures():
                        if reach.attributes()[dslinknoField] in zeroLinks:
                            streamLayer.changeAttributeValue(reach.id(), dslinknoField, linknoD)
                # change USLINK1 or USLINK2 references to A or zeroLinks to -1
                if uslinkno1Field >= 0:
                    Dup1 = reachDattrs[uslinkno1Field]
                    if Dup1 == linknoA or (zeroLinks and Dup1 in zeroLinks):
                        streamLayer.changeAttributeValue(reachD.id(), uslinkno1Field, -1)
                if uslinkno2Field >= 0:
                    Dup2 = reachDattrs[uslinkno2Field]
                    if Dup2 == linknoA or (zeroLinks and Dup2 in zeroLinks):
                        streamLayer.changeAttributeValue(reachD.id(), uslinkno2Field, -1)
                if magnitudeField >= 0:
                    # Magnitudes of D and below should be reduced by 1
                    nextReach = reachD
                    while nextReach:
                        mag = nextReach.attributes()[magnitudeField]
                        streamLayer.changeAttributeValue(nextReach.id(), magnitudeField, mag - 1)
                        nextReach = QSWATUtils.getFeatureByValue(streamLayer, linknoField,
                                                                 nextReach.attributes()[dslinknoField])
                # change Order field
                if orderField >= 0:
                    Delineation._ReassignStrahler(streamLayer, reachD, linknoField, dslinknoField, orderField)
                OK = streamLayer.deleteFeature(reachA.id())
                if not OK:
                    QSWATUtils.error('Cannot edit stream reaches shapefile', self._gv.isBatch)
                    streamLayer.rollBack()
                    return
                if zeroReaches:
                    for reach in zeroReaches:
                        streamLayer.deleteFeature(reach.id())
            else:
                # create new merged stream M from D and A and add it to streams
                # prepare reachM
                reachM = QgsFeature()
                streamFields = streamLayer.dataProvider().fields()
                reachM.setFields(streamFields)
                reachM.setGeometry(reachD.geometry().combine(reachA.geometry()))
                # check if we have single line
                if reachM.geometry().isMultipart():
                    QSWATUtils.loginfo('Multipart reach')
                OK = streamLayer.addFeature(reachM)
                if not OK:
                    QSWATUtils.error('Cannot add shape to stream reaches shapefile', self._gv.isBatch)
                    streamLayer.rollBack()
                    return
                idM = reachM.id()
                streamLayer.changeAttributeValue(idM, linknoField, linknoD)
                streamLayer.changeAttributeValue(idM, dslinknoField, reachDattrs[dslinknoField])
                # change dslinks in UAs to D (= M)
                for reach in reachUAs:
                    streamLayer.changeAttributeValue(reach.id(), dslinknoField, linknoD)
                # change any dslinks to zeroLinks to D as the zeroReaches will be deleted
                if zeroLinks:
                    for reach in streamLayer.getFeatures():
                        if reach.attributes()[dslinknoField] in zeroLinks:
                            streamLayer.changeAttributeValue(reach.id(), dslinknoField, linknoD)
                if uslinkno1Field >= 0:
                    Dup1 = reachDattrs[uslinkno1Field]
                    if Dup1 == linknoA or (zeroLinks and Dup1 in zeroLinks):
                        # in general these cannot be relied on, since as we remove zero length links
                        # there may be more than two upstream links from M
                        # At least don't leave it referring to a soon to be non-existent reach
                        streamLayer.changeAttributeValue(idM, uslinkno1Field, reachAattrs[uslinkno1Field])
                    else:
                        streamLayer.changeAttributeValue(idM, uslinkno1Field, Dup1)
                if uslinkno2Field >= 0:
                    Dup2 = reachDattrs[uslinkno2Field]
                    if Dup2 == linknoA or (zeroLinks and Dup2 in zeroLinks):
                        # in general these cannot be relied on, since as we remove zero length links
                        # there may be more than two upstream links from M
                        # At least don't leave it referring to a soon to be non-existent reach
                        streamLayer.changeAttributeValue(idM, uslinkno2Field, reachAattrs[uslinkno2Field])
                    else:
                        streamLayer.changeAttributeValue(idM, uslinkno2Field, Dup2)
                if dsnodeidnField >= 0:
                    streamLayer.changeAttributeValue(idM, dsnodeidnField, reachDattrs[dsnodeidnField])
                if orderField >= 0:
                    streamLayer.changeAttributeValue(idM, orderField, reachDattrs[orderField])
                if lengthField >= 0:
                    lengthA = reachAattrs[lengthField]
                    lengthD = reachDattrs[lengthField]
                    streamLayer.changeAttributeValue(idM, lengthField, lengthA + lengthD)
                elif slopeField >= 0 or straight_lField >= 0 or (dout_endField >= 0 and dout_midField >= 0):
                    # we will need these lengths
                    lengthA = reachA.geometry().length()
                    lengthD = reachD.geometry().length()
                if magnitudeField >= 0:
                    streamLayer.changeAttributeValue(idM, magnitudeField, reachDattrs[magnitudeField])
                if ds_cont_arField >= 0:
                    streamLayer.changeAttributeValue(idM, ds_cont_arField, reachDattrs[ds_cont_arField])
                if dropField >= 0:
                    dropA = reachAattrs[dropField]
                    dropD = reachDattrs[dropField]
                    streamLayer.changeAttributeValue(idM, dropField, dropA + dropD)
                elif slopeField >= 0:
                    dataA = self._gv.topo.getReachData(reachA, demLayer)
                    dropA = dataA.upperZ = dataA.lowerZ
                    dataD = self._gv.topo.getReachData(reachD, demLayer)
                    dropD = dataD.upperZ = dataD.lowerZ
                if slopeField >= 0:
                    streamLayer.changeAttributeValue(idM, slopeField, (dropA + dropD) / (lengthA + lengthD))
                if straight_lField >= 0:
                    dataA = self._gv.topo.getReachData(reachA, demLayer)
                    dataD = self._gv.topo.getReachData(reachD, demLayer)
                    dx = dataA.upperX - dataD.lowerX
                    dy = dataA.upperY - dataD.lowerY
                    streamLayer.changeAttributeValue(idM, straight_lField, math.sqrt(dx * dx + dy * dy))
                if us_cont_arField >= 0:
                    streamLayer.changeAttributeValue(idM, us_cont_arField, reachAattrs[us_cont_arField])
                streamLayer.changeAttributeValue(idM, wsnoField, polygonidD)
                if dout_endField >= 0:
                    streamLayer.changeAttributeValue(idM, dout_endField, reachDattrs[dout_endField])
                if dout_startField >= 0:
                    streamLayer.changeAttributeValue(idM, dout_startField, reachAattrs[dout_startField])
                if dout_endField >= 0 and dout_midField >= 0:
                    streamLayer.changeAttributeValue(idM, dout_midField,
                                                     reachDattrs[dout_endField] + (lengthA + lengthD) / 2.0)
                streamLayer.deleteFeature(reachA.id())
                streamLayer.deleteFeature(reachD.id())
                if zeroReaches:
                    for reach in zeroReaches:
                        streamLayer.deleteFeature(reach.id())
        except Exception as ex:
            QSWATUtils.error('Exception while updating stream reach shapefile: {0!s}'.format(str(ex)), self._gv.isBatch)
            OK = False
            streamLayer.rollBack()
            return
        else:
            if streamLayer.isEditable():
                streamLayer.commitChanges()
                streamLayer.triggerRepaint()
        if not OK:
            return

        # New watershed shapefile will be inconsistent with watershed grid, so remove grid to be recreated later.
        # Do not do it immediately because the user may remove several subbasins, so we wait until the
        # delineation form is closed.
        # clear name as flag that it needs to be recreated
        self._gv.basinFile = ''
        try:
            OK = wshedLayer.startEditing()
            if not OK:
                QSWATUtils.error('Cannot edit watershed shapefile', self._gv.isBatch)
                return
            # create new merged subbasin M from D and A and add it to wshed
            # prepare reachM
            wshedM = QgsFeature()
            wshedFields = wshedLayer.dataProvider().fields()
            wshedM.setFields(wshedFields)
            wshedM.setGeometry(wshedD.geometry().combine(wshedA.geometry()))
            OK = wshedLayer.addFeature(wshedM)
            if not OK:
                QSWATUtils.error('Cannot add shape to watershed shapefile', self._gv.isBatch)
                wshedLayer.rollBack()
                return
            idM = wshedM.id()
            wshedLayer.changeAttributeValue(idM, polygonidField, polygonidD)
            if areaField >= 0:
                areaA = wshedAattrs[areaField]
                areaD = wshedDattrs[areaField]
                wshedLayer.changeAttributeValue(idM, areaField, areaA + areaD)
            if streamlinkField >= 0:
                wshedLayer.changeAttributeValue(idM, streamlinkField, wshedDattrs[streamlinkField])
            if streamlenField >= 0:
                lenA = wshedAattrs[streamlenField]
                lenD = wshedDattrs[streamlenField]
                wshedLayer.changeAttributeValue(idM, streamlenField, lenA + lenD)
            if dsnodeidwField >= 0:
                wshedLayer.changeAttributeValue(idM, dsnodeidwField, wshedDattrs[dsnodeidwField])
            if dswsidField >= 0:
                wshedLayer.changeAttributeValue(idM, dswsidField, wshedDattrs[dswsidField])
                # change downlinks upstream of A from A to D (= M)
                wshedUAs = [wshed for wshed in wshedLayer.getFeatures() if
                            wshed.attributes()[dswsidField] == polygonidA]
                for wshedUA in wshedUAs:
                    wshedLayer.changeAttributeValue(wshedUA.id(), dswsidField, polygonidD)
            if us1wsidField >= 0:
                if wshedDattrs[us1wsidField] == polygonidA:
                    wshedLayer.changeAttributeValue(idM, us1wsidField, wshedAattrs[us1wsidField])
                else:
                    wshedLayer.changeAttributeValue(idM, us1wsidField, wshedDattrs[us1wsidField])
            if us2wsidField >= 0:
                if wshedDattrs[us2wsidField] == polygonidA:
                    wshedLayer.changeAttributeValue(idM, us2wsidField, wshedAattrs[us2wsidField])
                else:
                    wshedLayer.changeAttributeValue(idM, us2wsidField, wshedDattrs[us2wsidField])
            if subbasinField >= 0:
                wshedLayer.changeAttributeValue(idM, subbasinField, wshedDattrs[subbasinField])
            # remove A and D subbasins
            wshedLayer.deleteFeature(wshedA.id())
            wshedLayer.deleteFeature(wshedD.id())
        except Exception as ex:
            QSWATUtils.error('Exception while updating watershed shapefile: {0!s}'.format(str(ex)), self._gv.isBatch)
            OK = False
            wshedLayer.rollBack()
            return
        else:
            if wshedLayer.isEditable():
                wshedLayer.commitChanges()
                wshedLayer.triggerRepaint()