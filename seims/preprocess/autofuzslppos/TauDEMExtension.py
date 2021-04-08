"""Extensions based on TauDEM framework.

    @author   : Liangjun Zhu

    @changelog:
    - 17-08-01  - lj - initial implementation based on pygeoc.
    - 17-12-20  - lj - update code style
"""
from __future__ import absolute_import, unicode_literals

from pygeoc.TauDEM import TauDEM
from pygeoc.utils import StringClass, FileClass


class TauDEMExtension(TauDEM):
    """Extension functions based on TauDEM."""

    def __init__(self):
        """Initialize TauDEM."""
        TauDEM.__init__(self)

    @staticmethod
    def d8distuptoridge(np, p, fel, src, dist, distm,
                        workingdir=None, mpiexedir=None, exedir=None,
                        log_file=None, runtime_file=None, hostfile=None):
        """Run D8 distance to stream"""
        fname = TauDEM.func_name('d8distuptoridge')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-fel': fel, '-p': p, '-src': src},
                          wp=workingdir,
                          in_params={'-m': TauDEM.convertdistmethod(distm)},
                          out_files={'-du': dist},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def dinfdistuptoridge(np, ang, fel, slp, propthresh, dist, statsm, distm,
                          edgecontamination, rdg=None,
                          workingdir=None, mpiexedir=None, exedir=None,
                          log_file=None, runtime_file=None, hostfile=None):
        """Run Dinf distance to ridge."""
        fname = TauDEM.func_name('dinfdistuptoridge')
        in_params = {'-thresh': str(propthresh),
                     '-m': '%s %s' % (TauDEM.convertstatsmethod(statsm),
                                      TauDEM.convertdistmethod(distm))}
        if StringClass.string_match(edgecontamination, 'false') or edgecontamination is False:
            in_params['-nc'] = None
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-ang': ang, '-fel': fel, '-slp': slp, '-rdg': rdg},
                          wp=workingdir,
                          in_params=in_params,
                          out_files={'-du': dist},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def extractridge(np, angfile, elevfile, rdgsrc,
                     workingdir=None, mpiexedir=None, exedir=None,
                     log_file=None, runtime_file=None, hostfile=None):
        """Extract ridge source."""
        fname = TauDEM.func_name('ridgeextraction')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-dir': angfile, '-fel': elevfile},
                          wp=workingdir,
                          in_params=None,
                          out_files={'-src': rdgsrc},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def rpiskidmore(np, vlysrc, rdgsrc, rpi, vlytag=1, rdgtag=1, dist2vly=None, dist2rdg=None,
                    workingdir=None, mpiexedir=None, exedir=None,
                    log_file=None, runtime_file=None, hostfile=None):
        """Calculate RPI according to Skidmore (1990)."""
        fname = TauDEM.func_name('rpiskidmore')
        in_params = dict()
        if vlytag > 0:
            in_params['-vlytag'] = vlytag
        if rdgtag > 0:
            in_params['-rdgtag'] = rdgtag
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-vly': vlysrc, '-rdg': rdgsrc},
                          wp=workingdir,
                          in_params=in_params,
                          out_files={'-rpi': rpi, '-dist2vly': dist2vly, '-dist2rdg': dist2rdg},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def curvature(np, fel, profc,
                  horizc=None, planc=None, unspherc=None, avec=None, maxc=None, minc=None,
                  workingdir=None, mpiexedir=None, exedir=None,
                  log_file=None, runtime_file=None, hostfile=None):
        """Calculate various curvature."""
        fname = TauDEM.func_name('curvature')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-fel': fel},
                          wp=workingdir,
                          in_params=None,
                          out_files={'-prof': profc, '-plan': planc, '-horiz': horizc,
                                     '-unspher': unspherc, '-ave': avec, '-max': maxc,
                                     '-min': minc},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def simplecalculator(np, inputa, inputb, output, operator,
                         workingdir=None, mpiexedir=None, exedir=None,
                         log_file=None, runtime_file=None, hostfile=None):
        """Run simple calculator.

           operator = 0: add
                      1: minus
                      2: multiply
                      3: divide
                      4: a/(a+b)
                      5: mask
        """
        fname = TauDEM.func_name('simplecalculator')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-in': [inputa, inputb]},
                          wp=workingdir,
                          in_params={'-op': operator},
                          out_files={'-out': output},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def selecttyplocslppos(np, inputconf, outputconf=None, extlog=None,
                           workingdir=None, mpiexedir=None, exedir=None,
                           log_file=None, runtime_file=None, hostfile=None):
        """Select typical locations."""
        fname = TauDEM.func_name('selecttyplocslppos')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-in': inputconf},
                          wp=workingdir,
                          in_params=None,
                          out_files={'-out': outputconf, '-extlog': extlog},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def fuzzyslpposinference(np, config, workingdir=None, mpiexedir=None,
                             exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run fuzzy inference."""
        fname = TauDEM.func_name('fuzzyslpposinference')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-in': config},
                          wp=workingdir,
                          in_params=None,
                          out_files=None,
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def hardenslppos(np, simifiles, tags, hard, maxsimi,
                     sechard=None, secsimi=None, spsim=None, spsi=None,
                     workingdir=None, mpiexedir=None, exedir=None,
                     log_file=None, runtime_file=None, hostfile=None):
        """Select typical locations."""
        fname = TauDEM.func_name('hardenslppos')
        if len(simifiles) != len(tags):
            raise RuntimeError("hardenslppos: simifiles and tags must have the same size!")
        tag_path = ''
        for i, tag in enumerate(tags):
            tag_path += ' %d %s ' % (tag, simifiles[i])
        in_params = dict()
        if spsim is not None and spsi is not None:
            in_params['-m'] = '%d %s' % (spsim, spsi)
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          in_files={'-inf': '%d %s' % (len(simifiles), tag_path)},
                          wp=workingdir,
                          in_params=in_params,
                          out_files={'-maxS': [hard, maxsimi], '-secS': [sechard, secsimi]},
                          mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          log_params={'logfile': log_file, 'runtimefile': runtime_file})
