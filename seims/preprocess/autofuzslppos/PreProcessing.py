"""Calculate terrain attributes from DEM and other optional inputs for deriving slope position.

    Slope, Curvature, RPI, HAND, Hillslope, etc.

    @author   : Liangjun Zhu

    @changelog:
    - 15-09-08  - lj - initial implementation.
    - 17-07-31  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals

from io import open
import time
from shutil import copy2
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.TauDEM import TauDEMWorkflow
from pygeoc.utils import FileClass
from pygeoc.raster import RasterUtilClass

from autofuzslppos.Config import get_input_cfgs
from autofuzslppos.TauDEMExtension import TauDEMExtension
from autofuzslppos.Util import slope_rad_to_deg


def check_watershed_delineation_results(cfg):
    """Check if watershed_delineation is need to run."""
    if not FileClass.is_file_exists(cfg.pretaudem.filldem):
        return False
    if not FileClass.is_file_exists(cfg.pretaudem.outlet_m):
        return False
    if cfg.d8_stream_thresh <= 0 and not FileClass.is_file_exists(cfg.pretaudem.drptxt):
        return False
    if not FileClass.is_file_exists(cfg.pretaudem.d8flow):
        return False
    if cfg.flow_model == 1:
        if not FileClass.is_file_exists(cfg.pretaudem.dinf):
            return False
        if not FileClass.is_file_exists(cfg.pretaudem.dinf_slp):
            return False
        if not FileClass.is_file_exists(cfg.pretaudem.stream_pd):
            return False
    return True


def pre_processing(cfg):
    start_t = time.time()
    if not cfg.flag_preprocess:
        return 0
    single_basin = False
    if cfg.outlet is not None:
        single_basin = True
    pretaudem_done = check_watershed_delineation_results(cfg)
    if cfg.valley is None or not FileClass.is_file_exists(cfg.valley) or not pretaudem_done:
        cfg.valley = cfg.pretaudem.stream_raster
        # Watershed delineation based on D8 flow model.
        TauDEMWorkflow.watershed_delineation(cfg.proc, cfg.dem, cfg.outlet, cfg.d8_stream_thresh,
                                             single_basin,
                                             cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                             logfile=cfg.log.preproc, runtime_file=cfg.log.runtime,
                                             hostfile=cfg.hostfile)
    # use outlet_m or not
    outlet_use = None
    if single_basin:
        outlet_use = cfg.pretaudem.outlet_m
    log_status = open(cfg.log.preproc, 'a', encoding='utf-8')
    log_status.write('Calculating RPI(Relative Position Index)...\n')
    log_status.flush()
    if cfg.flow_model == 1:  # Dinf model, extract stream using the D8 threshold
        if cfg.valley is None or not FileClass.is_file_exists(cfg.valley):
            if cfg.d8_stream_thresh <= 0:
                with open(cfg.pretaudem.drptxt, 'r', encoding='utf-8') as drpf:
                    temp_contents = drpf.read()
                    (beg, cfg.d8_stream_thresh) = temp_contents.rsplit(' ', 1)
            print(cfg.d8_stream_thresh)
            TauDEMExtension.areadinf(cfg.proc, cfg.pretaudem.dinf,
                                     cfg.pretaudem.dinfacc_weight, outlet_use,
                                     cfg.pretaudem.stream_pd, 'false',
                                     cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                     cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
            TauDEMExtension.threshold(cfg.proc, cfg.pretaudem.dinfacc_weight,
                                      cfg.pretaudem.stream_dinf, float(cfg.d8_stream_thresh),
                                      cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                      cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
            cfg.valley = cfg.pretaudem.stream_dinf
        # calculate Height Above the Nearest Drainage (HAND)
        TauDEMExtension.dinfdistdown(cfg.proc, cfg.pretaudem.dinf, cfg.pretaudem.filldem,
                                     cfg.pretaudem.dinf_slp, cfg.valley,
                                     cfg.dinf_down_stat, 'v', 'false',
                                     cfg.dinf_dist_down_wg, cfg.pretaudem.dist2stream_v,
                                     cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                     cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
    else:
        # calculate Height Above the Nearest Drainage (HAND)
        TauDEMExtension.d8distdowntostream(cfg.proc, cfg.pretaudem.d8flow,
                                           cfg.pretaudem.filldem, cfg.valley,
                                           cfg.pretaudem.dist2stream_v, 'v', 1,
                                           cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                           cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
    if cfg.rpi_method == 1:  # calculate RPI based on hydrological proximity measures (Default).
        if cfg.flow_model == 0:  # D8 model
            TauDEMExtension.d8distdowntostream(cfg.proc, cfg.pretaudem.d8flow,
                                               cfg.pretaudem.filldem, cfg.valley,
                                               cfg.pretaudem.dist2stream, cfg.d8_down_method, 1,
                                               cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                               cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
            TauDEMExtension.d8distuptoridge(cfg.proc, cfg.pretaudem.d8flow,
                                            cfg.pretaudem.filldem, cfg.ridge,
                                            cfg.pretaudem.distup2rdg, cfg.d8_up_method,
                                            cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                            cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
        elif cfg.flow_model == 1:  # Dinf model
            # Dinf distance down
            TauDEMExtension.dinfdistdown(cfg.proc, cfg.pretaudem.dinf, cfg.pretaudem.filldem,
                                         cfg.pretaudem.dinf_slp, cfg.valley,
                                         cfg.dinf_down_stat, cfg.dinf_down_method, 'false',
                                         cfg.dinf_dist_down_wg, cfg.pretaudem.dist2stream,
                                         cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                         cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
            TauDEMExtension.dinfdistuptoridge(cfg.proc, cfg.pretaudem.dinf,
                                              cfg.pretaudem.filldem, cfg.pretaudem.dinf_slp,
                                              cfg.propthresh, cfg.pretaudem.distup2rdg,
                                              cfg.dinf_up_stat, cfg.dinf_up_method, 'false',
                                              cfg.ridge,
                                              cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                              cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
        TauDEMExtension.simplecalculator(cfg.proc, cfg.pretaudem.dist2stream,
                                         cfg.pretaudem.distup2rdg, cfg.pretaudem.rpi_hydro, 4,
                                         cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                         cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
    if cfg.rpi_method == 0:  # calculate RPI based on Skidmore's method
        if cfg.ridge is None or not FileClass.is_file_exists(cfg.ridge):
            cfg.ridge = cfg.pretaudem.rdgsrc
            angfile = cfg.pretaudem.d8flow
            elevfile = cfg.pretaudem.dist2stream_v
            if cfg.flow_model == 1:  # D-inf model
                angfile = cfg.pretaudem.dinf
                elevfile = cfg.pretaudem.dist2stream_v
            TauDEMExtension.extractridge(cfg.proc, angfile, elevfile, cfg.ridge,
                                         cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                         cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
        TauDEMExtension.rpiskidmore(cfg.proc, cfg.valley, cfg.ridge,
                                    cfg.pretaudem.rpi_skidmore, 1, 1,
                                    cfg.pretaudem.dist2stream_ed, cfg.pretaudem.dist2rdg_ed,
                                    cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                                    cfg.log.preproc, cfg.log.runtime, cfg.hostfile)
    log_status.write('Calculating Horizontal Curvature and Profile Curvature...\n')
    TauDEMExtension.curvature(cfg.proc, cfg.pretaudem.filldem,
                              cfg.topoparam.profc, cfg.topoparam.horizc,
                              None, None, None, None, None,
                              cfg.ws.pre_dir, cfg.mpi_dir, cfg.bin_dir,
                              cfg.log.preproc, cfg.log.runtime, cfg.hostfile)

    if cfg.flow_model == 0:
        slope_rad_to_deg(cfg.pretaudem.slp, cfg.topoparam.slope)
    elif cfg.flow_model == 1:
        slope_rad_to_deg(cfg.pretaudem.dinf_slp, cfg.topoparam.slope)
    if cfg.rpi_method == 1:
        copy2(cfg.pretaudem.rpi_hydro, cfg.topoparam.rpi)
    else:
        copy2(cfg.pretaudem.rpi_skidmore, cfg.topoparam.rpi)
    copy2(cfg.pretaudem.dist2stream_v, cfg.topoparam.hand)
    copy2(cfg.pretaudem.filldem, cfg.topoparam.elev)

    if single_basin:  # clip RPI
        RasterUtilClass.mask_raster(cfg.topoparam.rpi, cfg.pretaudem.subbsn, cfg.topoparam.rpi)

    log_status.write('Preprocessing succeed!\n')
    end_t = time.time()
    cost = (end_t - start_t) / 60.
    log_status.write('Time consuming: %.2f min.\n' % cost)
    log_status.close()
    with open(cfg.log.runtime, 'a', encoding='utf-8') as logf:
        logf.write('Preprocessing Time-consuming: %s\n' % repr(cost))
    return cost


def main():
    """TEST CODE"""
    fuzslppos_cfg = get_input_cfgs()
    pre_processing(fuzslppos_cfg)


if __name__ == '__main__':
    main()
