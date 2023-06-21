import osgeo  # Note! This is a workaround for a 'ImportError: DLL load failed' of `import rasterio`!
import rasterio
import geopandas as gpd
import numpy as np
from rasterio import features  # Cannot call rasterio.features, can only be imported.
from pygeoc.utils import DEFAULT_NODATA

from preprocess.config import PreprocessConfig
from preprocess.db_build_mongodb import ImportMongodbClass
from preprocess.text import SpatialNamesUtils, ParamAbstractionTypes
from utility import mask_rasterio


class HruConstructor(object):

    def __init__(self):
        """
        Delineate HRU based on subbasin, soil, and landuse rasters.
        property_names: property names, will be the header of shapefile attribute table.
            e.g.: 'subbasin', 'soil', 'landuse'
        property_raster_paths: property raster paths
        """
        self._property_names = []
        self._property_raster_paths = []

    def add_property(self, property_name: str, property_raster_path: str):
        """Add property to delineate HRU.
        Args:
            property_name: property name, will be the header of shapefile attribute table.
                           e.g.: 'subbasin', 'soil', 'landuse'
            property_raster_path: property raster path
        """
        self._property_names.append(property_name)
        self._property_raster_paths.append(property_raster_path)

    def _create_hru_map(self, subbasin: np.ndarray) -> (np.ndarray, dict, np.ndarray, dict, dict):
        """Delineate HRU based on subbasin, soil, and landuse rasters.
        Returns:
            HRU id map
            HRU id map metadata for rasterio
            HRU id dict, key is the combination of property values
        """
        if subbasin is None:
            raise ValueError('ErrorInput: subbasin or basin is required when delineating HRU!')
        property_raster_src_list = []
        property_raster_list = []
        for i in range(len(self._property_names)):
            property_raster_path = self._property_raster_paths[i]
            src = rasterio.open(property_raster_path)
            property_raster_src_list.append(src)
            property_raster = src.read(1)
            property_raster_list.append(property_raster)
        property_raster_list.append(subbasin)

        hru_dist_map = property_raster_list[0].copy()
        hru_dist_map = hru_dist_map.astype(np.int32)
        hru_id_map = property_raster_list[0].copy()
        hru_id_map = hru_id_map.astype(np.int32)
        hru_area_map = property_raster_list[0].copy()
        hru_area_map = hru_area_map.astype(np.float32)

        # hru_id_dict: HRU id dict, key is the combination of property values, value is a hru_id
        # e.g.: {(1, 2, 3): 1, (1, 2, 4): 2}
        hru_id_dict = dict()
        # hru_position_dict: HRU first occurrence position dict, key is hru_id, value is a list of (row, col) tuples
        # e.g.: {1: (1, 2), 2: (5, 6)}
        hru_position_dict = dict()

        hru_id_seq = 1
        nodata_list = [src.nodata for src in property_raster_src_list]
        concat = np.dstack(property_raster_list)

        cell_area = property_raster_src_list[0].res[0] * property_raster_src_list[0].res[1]
        for i in range(concat.shape[0]):
            for j in range(concat.shape[1]):
                comb = concat[i, j]  # combination of property values
                # if any nodata, set hru_id to nodata
                if [comb[_] == nodata_list[_] for _ in range(len(nodata_list))].count(True) > 0:
                    hru_id_map[i, j] = DEFAULT_NODATA
                    hru_area_map[i, j] = DEFAULT_NODATA
                    hru_dist_map[i, j] = DEFAULT_NODATA
                    continue
                # make comb (array) to be dict key
                comb = tuple(comb)
                # if comb not in hru_id_dict, add it to hru_id_dict
                if comb not in hru_id_dict:
                    hru_id_dict[comb] = hru_id_seq
                    hru_id_map[i, j] = hru_id_seq
                    hru_area_map[i, j] = cell_area
                    hru_position_dict[hru_id_seq] = (i, j)
                    hru_id_seq += 1
                else:
                    # only record the first occurrence of a new combination
                    hru_id_map[i, j] = DEFAULT_NODATA
                    hru_area_map[i, j] = DEFAULT_NODATA
                    hru_area_map[hru_position_dict[hru_id_dict[comb]]] += cell_area

                hru_dist_map[i, j] = hru_id_dict[comb]

        meta_int = property_raster_src_list[0].meta.copy()
        meta_int.update(dtype=rasterio.int32, nodata=DEFAULT_NODATA)
        meta_float = property_raster_src_list[0].meta.copy()
        meta_float.update(dtype=rasterio.float32, nodata=DEFAULT_NODATA)
        return hru_dist_map, meta_int, hru_id_map, meta_int, hru_area_map, meta_float, hru_id_dict

    @staticmethod
    def _write_raster(raster_path, raster, meta):
        with rasterio.open(raster_path, 'w', **meta) as dst:
            dst.write(raster, 1)

    def generate_cell_area_file(self, cfg: PreprocessConfig, mongoargs):
        """Generate cell area raster.
        Args:
            cfg: PreprocessConfig
        """
        with rasterio.open(cfg.spatials.mask) as src:
            cell_area = src.read(1)
            cell_area[cell_area != src.nodata] = src.res[0] * src.res[1]
            meta = src.meta.copy()
            meta.update(dtype=rasterio.float32, nodata=DEFAULT_NODATA)
            self._write_raster(cfg.spatials.cell_area, cell_area, meta)
        cell_area_cfg = [cfg.spatials.cell_area, SpatialNamesUtils._CELLAREA,
                        DEFAULT_NODATA, DEFAULT_NODATA, 'FLOAT32']
        mask_rasterio(cfg.seims_bin, [cell_area_cfg], mongoargs=mongoargs,
                      # maskfile=cfg.spatials.subbsn, include_nodata=False, mode='MASKDEC',
                      maskfile=cfg.spatials.subbsn, include_nodata=False, mode='MASKDEC',
                      abstraction_type=ParamAbstractionTypes.PHYSICAL)

    def delineate(self, cfg: PreprocessConfig):
        mongoargs = [cfg.hostname, cfg.port, cfg.spatial_db, 'SPATIAL']
        self.generate_cell_area_file(cfg, mongoargs=mongoargs)
        if not cfg.has_conceptual_subbasins():
            return
        if 0 in cfg.conceptual_subbasins:
            # if lumped, use MASK.tif, set it to 1
            subbasin_mask_path = cfg.spatials.mask
            subbasin_mask_src = rasterio.open(subbasin_mask_path)
            subbasin_mask = subbasin_mask_src.read(1)
            subbasin_mask[subbasin_mask != subbasin_mask_src.nodata] = 1
        else:
            # if not lumped, use SUBBASIN.tif
            subbasin_mask_path = cfg.spatials.subbsn
            subbasin_mask_src = rasterio.open(subbasin_mask_path)
            subbasin_mask = subbasin_mask_src.read(1)

        hru_dist_map, meta_int, hru_id_map, meta_int, hru_area_map, meta_float, hru_id_dict = \
            self._create_hru_map(subbasin_mask)

        self._write_raster(cfg.spatials.hru_dist, hru_dist_map, meta_int)
        self._write_raster(cfg.spatials.hru_id, hru_id_map, meta_int)
        self._write_raster(cfg.spatials.hru_area, hru_area_map, meta_float)

        # mask subbasin_mask by hru_id_map, only keep its hru_id area
        subbasin_mask[hru_id_map == DEFAULT_NODATA] = DEFAULT_NODATA
        self._write_raster(cfg.spatials.hru_subbasin_id, subbasin_mask, meta_int)

        hru_area_cfg = [cfg.spatials.hru_area, SpatialNamesUtils._CELLAREA,
                        DEFAULT_NODATA, DEFAULT_NODATA, 'FLOAT32']
        subbasin_cfg = [cfg.spatials.hru_subbasin_id, SpatialNamesUtils._SUBBASINOUT,
                        DEFAULT_NODATA, DEFAULT_NODATA, 'INT32']
        mask_raster_cfg = [hru_area_cfg, subbasin_cfg]

        mask_rasterio(cfg.seims_bin, mask_raster_cfg, mongoargs=mongoargs,
                      # maskfile=cfg.spatials.subbsn, include_nodata=False, mode='MASKDEC',
                      maskfile=cfg.spatials.hru_subbasin_id, include_nodata=False, mode='MASKDEC',
                      abstraction_type=ParamAbstractionTypes.CONCEPTUAL)

        # again write the SUBBASIN (e.g., 0_SUBBASIN) to DB, include nodata.
        # this is for mask_rasterio to use it as mask
        mask_rasterio(cfg.seims_bin, [subbasin_cfg], mongoargs=mongoargs,
                      # maskfile=cfg.spatials.subbsn, include_nodata=False, mode='MASKDEC',
                      maskfile=cfg.spatials.hru_subbasin_id, include_nodata=True, mode='MASKDEC',
                      abstraction_type=ParamAbstractionTypes.CONCEPTUAL)

        ImportMongodbClass.spatial_rasters(cfg,
                                           mask_rasterio_maskfile=cfg.spatials.hru_subbasin_id,
                                           abstraction_type=ParamAbstractionTypes.CONCEPTUAL)


def main():
    from preprocess.config import parse_ini_configuration
    cfg = parse_ini_configuration()
    hru = HruConstructor()
    hru.add_property(SpatialNamesUtils._SOILTYPEMFILE, cfg.spatials.soil_type)
    hru.add_property(SpatialNamesUtils._LANDUSEMFILE, cfg.spatials.landuse)
    hru.delineate(cfg)


if __name__ == '__main__':
    main()
