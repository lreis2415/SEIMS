"""Extract ridge sources using flow direction, subbasin, and elevation.

    - 1. Identify original ridge sources (RdgOrgSrc), which are cells that have no flow-in cells
         or have very few flow-in proportion for Dinf (TODO).
    - 2. Read subbasin and identify the boundary grids as potential ridges (RdgPotSrc).
    - 3. Sort each subbasin's boundary cells by elevation, filter by the a given percent, e.g. 70%.
    - 4. Filter RdgOrgSrc by RdgPotSrc.

    Be caution, the derived ridge sources may need manually modification when further use.

    @author   : Liangjun Zhu

    @changelog:
    - 16-08-07  - lj - initial implementation.
    - 17-08-09  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from pygeoc.hydro import FlowModelConst
from pygeoc.postTauDEM import D8Util, DinfUtil
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import MathClass, FileClass, DEFAULT_NODATA

from autofuzslppos.Config import get_input_cfgs


class RidgeSourceExtraction(object):
    """Class for extracting ridge sources."""

    def __init__(self, flowdirf, subbsnf, elevf, rdgsrc, flow_model=1, prop=0., ws=None):
        """Initialize file names."""
        FileClass.check_file_exists(flowdirf)
        FileClass.check_file_exists(subbsnf)
        FileClass.check_file_exists(elevf)
        if ws is None:
            ws = os.path.basename(flowdirf)
        self.ws = ws
        if flow_model == 1:
            suffix = '_dinf.tif'
        else:
            suffix = '_d8.tif'
        self.rdgorg = self.ws + os.sep + 'RdgOrgSrc' + suffix
        self.boundsrc = self.ws + os.sep + 'RdgPotSrc' + suffix
        self.boundsrcfilter = self.ws + os.sep + 'RdgPotSrcFilter' + suffix

        if rdgsrc is None:
            rdgsrc = self.ws + os.sep + 'rdgsrc' + suffix
        self.rdgsrc = rdgsrc
        self.flowmodel = flow_model
        self.prop = prop
        # read raster data
        flowdir_r = RasterUtilClass.read_raster(flowdirf)
        self.flowdir_data = flowdir_r.data
        self.nrows = flowdir_r.nRows
        self.ncols = flowdir_r.nCols
        self.nodata_flow = flowdir_r.noDataValue
        self.geotrans = flowdir_r.geotrans
        self.srs = flowdir_r.srs
        subbsn_r = RasterUtilClass.read_raster(subbsnf)
        self.subbsn_data = subbsn_r.data
        self.nodata_subbsn = subbsn_r.noDataValue
        elev_r = RasterUtilClass.read_raster(elevf)
        self.elev_data = elev_r.data
        self.nodata_elev = elev_r.noDataValue

        # initialize output arrays
        self.rdgsrc_data = numpy.ones((self.nrows, self.ncols)) * 1
        self.rdgpot = numpy.ones((self.nrows, self.ncols)) * DEFAULT_NODATA

    def ridge_without_flowin_cell(self):
        """Find the original ridge sources that have no flow-in cells."""
        for row in range(self.nrows):
            for col in range(self.ncols):
                tempdir = self.flowdir_data[row][col]
                if MathClass.floatequal(tempdir, self.nodata_flow):
                    self.rdgsrc_data[row][col] = DEFAULT_NODATA
                    continue
                if self.flowmodel == 1:  # Dinf flow model
                    temp_coor = DinfUtil.downstream_index_dinf(tempdir, row, col)
                    for temprow, tempcol in temp_coor:
                        if 0 <= temprow < self.nrows and 0 <= tempcol < self.ncols:
                            self.rdgsrc_data[temprow][tempcol] = DEFAULT_NODATA
                        else:
                            self.rdgsrc_data[row][col] = DEFAULT_NODATA
                else:  # D8 flow model
                    temprow, tempcol = D8Util.downstream_index(tempdir, row, col)
                    if 0 <= temprow < self.nrows and 0 <= tempcol < self.ncols:
                        self.rdgsrc_data[temprow][tempcol] = DEFAULT_NODATA
                    else:
                        self.rdgsrc_data[row][col] = DEFAULT_NODATA
        RasterUtilClass.write_gtiff_file(self.rdgorg, self.nrows, self.ncols, self.rdgsrc_data,
                                         self.geotrans, self.srs, DEFAULT_NODATA, 6)

    def subbasin_boundary_cells(self, subbsn_perc):
        """Subbasin boundary cells that are potential ridge sources."""
        dir_deltas = FlowModelConst.d8delta_ag.values()
        subbsn_elevs = dict()

        def add_elev_to_subbsn_elevs(sid, elev):
            if sid not in subbsn_elevs:
                subbsn_elevs[sid] = [elev]
            else:
                subbsn_elevs[sid].append(elev)

        for row in range(self.nrows):
            for col in range(self.ncols):
                if MathClass.floatequal(self.subbsn_data[row][col], self.nodata_subbsn):
                    continue
                for r, c in dir_deltas:
                    new_row = row + r
                    new_col = col + c
                    if 0 <= new_row < self.nrows and 0 <= new_col < self.ncols:
                        if MathClass.floatequal(self.subbsn_data[new_row][new_col],
                                                self.nodata_subbsn):
                            subbsnid = self.subbsn_data[row][col]
                            self.rdgpot[row][col] = subbsnid
                            add_elev_to_subbsn_elevs(subbsnid, self.elev_data[row][col])
                        elif not MathClass.floatequal(self.subbsn_data[row][col],
                                                      self.subbsn_data[new_row][new_col]):
                            subbsnid = self.subbsn_data[row][col]
                            subbsnid2 = self.subbsn_data[new_row][new_col]
                            self.rdgpot[row][col] = subbsnid
                            self.rdgpot[new_row][new_col] = subbsnid2
                            add_elev_to_subbsn_elevs(subbsnid, self.elev_data[row][col])
                            add_elev_to_subbsn_elevs(subbsnid2, self.elev_data[new_row][new_col])

        RasterUtilClass.write_gtiff_file(self.boundsrc, self.nrows, self.ncols, self.rdgpot,
                                         self.geotrans, self.srs, DEFAULT_NODATA, 6)
        subbsn_elevs_thresh = dict()
        for sid, elevs in list(subbsn_elevs.items()):
            tmpelev = numpy.array(elevs)
            tmpelev.sort()
            subbsn_elevs_thresh[sid] = tmpelev[int(len(tmpelev) * subbsn_perc)]
        for row in range(self.nrows):
            for col in range(self.ncols):
                if MathClass.floatequal(self.rdgpot[row][col], DEFAULT_NODATA):
                    continue
                if self.elev_data[row][col] < subbsn_elevs_thresh[self.subbsn_data[row][col]]:
                    self.rdgpot[row][col] = DEFAULT_NODATA
        RasterUtilClass.write_gtiff_file(self.boundsrcfilter, self.nrows, self.ncols, self.rdgpot,
                                         self.geotrans, self.srs, DEFAULT_NODATA, 6)

    def filter_ridge_by_subbasin_boundary(self):
        for row in range(self.nrows):
            for col in range(self.ncols):
                if MathClass.floatequal(self.rdgsrc_data[row][col], DEFAULT_NODATA):
                    continue
                if MathClass.floatequal(self.rdgpot[row][col], DEFAULT_NODATA):
                    self.rdgsrc_data[row][col] = DEFAULT_NODATA
        RasterUtilClass.write_gtiff_file(self.rdgsrc, self.nrows, self.ncols, self.rdgsrc_data,
                                         self.geotrans, self.srs, DEFAULT_NODATA, 6)

    def run(self):
        """Entrance."""
        self.ridge_without_flowin_cell()
        self.subbasin_boundary_cells(0.6)
        self.filter_ridge_by_subbasin_boundary()

        # origin = RasterUtilClass.read_raster(self.rdgorg)
        # temp = self.subbsn_data == self.nodata_subbsn
        # masked = numpy.where(temp, origin.noDataValue, 0)
        # temp2 = (origin.data >= 0) & (self.subbsn_data != self.nodata_subbsn)
        # masked = numpy.where(temp2, origin.data, masked)
        # RasterUtilClass.write_gtiff_file(self.rdgsrc, origin.nRows, origin.nCols, masked,
        #                                  origin.geotrans, origin.srs,
        #                                  origin.noDataValue, origin.dataType)


def main():
    """Main workflow."""
    cfg = get_input_cfgs()
    flowmodel = cfg.flow_model
    if flowmodel == 1:
        flowdir = cfg.pretaudem.dinf
    else:
        flowdir = cfg.pretaudem.d8flow
    subbasin = cfg.pretaudem.subbsn
    elev = cfg.pretaudem.filldem
    prop = 0.
    ws = cfg.ws.pre_dir
    rdg = cfg.ridge

    rdgsrc_obj = RidgeSourceExtraction(flowdir, subbasin, elev, rdg, flowmodel, prop, ws)
    rdgsrc_obj.run()


if __name__ == '__main__':
    main()
