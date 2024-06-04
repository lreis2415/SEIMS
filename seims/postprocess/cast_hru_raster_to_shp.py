import argparse
import os
import sys
from pathlib import Path

from osgeo import gdal
import geopandas as gpd
import numpy as np
import rasterio
from rasterio.features import shapes

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from preprocess.text import SpatialNamesUtils


def fill_hru_shp(
    hru: gpd.GeoDataFrame,
    hru_id: np.ndarray,
    simulation_result_file_path: Path,
):
    # with rasterio.open(simulation_result_file_path) as src:
    #     # Read the raster layer
    #     image = src.read(1)  # Assuming it's a single band raster
    #     image_polygons = ({'properties': {'value': v}, 'geometry': s}
    #                       for i, (s, v) in enumerate(shapes(image, mask=None, transform=src.transform)))
    #
    # # Create a GeoDataFrame from the shapes
    # value_shp = gpd.GeoDataFrame.from_features(list(image_polygons), crs=src.crs)
    # # join the value shp with the hru shp by position
    # res = gpd.sjoin(hru, value_shp, op='contains')
    # return res

    # find each position P of simulation result R(P) in hru_id HID(P)
    # then set HRU(HID(P)) = R(P)
    hru_copy = hru.copy()
    # add a new column to store the value, float32
    hru_copy['value'] = np.nan
    hru_copy['value'] = hru_copy['value'].astype(np.float32)
    nonempty_hru_ids = []
    with rasterio.open(simulation_result_file_path) as src:
        r = src.read(1)
        # find positions of R with nonempty values
        r_positions = np.where(r != src.nodata)
        for i, j in zip(r_positions[0], r_positions[1]):
            hid = hru_id[i, j]
            nonempty_hru_ids.append(hid)
            hru_id_val = hid
            val = r[i, j]
            print(f'writing value {val} to HRU {hru_id_val}')
            hru_copy.loc[hru_id_val, 'value'] = r[i, j]
    # drop empty HRUs
    hru_copy = hru_copy.loc[nonempty_hru_ids]
    return hru_copy


def main(
    simulation_result_dir: Path,
    simulation_result_file_suffix: str,
    hru_id_raster_path: Path,
    hru_dist_raster_path: Path,
):
    # Convert HRU distribution raster to shapefile
    # Read the raster layer
    with rasterio.open(hru_dist_raster_path) as src:
        # Extract shapes from the raster
        image = src.read(1)  # Assuming it's a single band raster
        results = ({'properties': {'hru_id': int(v)}, 'geometry': s}
                   for i, (s, v) in enumerate(shapes(image, mask=None, transform=src.transform)) if v != src.nodata)

    # Create a GeoDataFrame from the shapes
    gdf = gpd.GeoDataFrame.from_features(list(results), crs=src.crs)
    # Union polygons with the same value
    gdf = gdf.dissolve(by='hru_id')
    # Save GeoDataFrame as shapefile
    hru_shp_path = hru_dist_raster_path.parent / f'{SpatialNamesUtils._HRU_DIST}.shp'
    if hru_shp_path.exists():
        hru_shp_path.unlink()
    gdf.to_file(hru_shp_path.as_posix())

    hru_id_raster = rasterio.open(hru_id_raster_path)
    hru_id = hru_id_raster.read(1)
    # get all output files with suffix
    output_files = list(simulation_result_dir.glob(f'*{simulation_result_file_suffix}.tif'))
    for output_file in output_files:
        hru_shp = fill_hru_shp(gdf, hru_id, output_file)
        simulation_hru_shp_path = output_file.parent / f'{output_file.stem}.shp'
        if simulation_hru_shp_path.exists():
            simulation_hru_shp_path.unlink()
        hru_shp.to_file(simulation_hru_shp_path.as_posix())


if __name__ == '__main__':
    # Read config
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', type=str, help='SEIMS model result data directory')
    parser.add_argument('-w', type=str,
                        help=r'model worksapce directory, e.g.: D:\data\SEIMS\workspace\camels_11383500\spatial_raster.' + '\n' +
                             r'Will automatically search for HRU_ID.tif and HRU_DISTRIBUTION.tif')
    parser.add_argument('-s', type=str, default='con',
                        help='file name suffix for output on HRU_ID points, e.g., `con` for `0_SOLSW_SUM_con.tif`')
    args = parser.parse_args()
    result_dir = Path(args.d)
    workspace_dir = Path(args.w)
    suffix = args.s
    hru_id_raster = workspace_dir / f'{SpatialNamesUtils._HRU_ID}.tif'
    hru_dist_raster = workspace_dir / f'{SpatialNamesUtils._HRU_DIST}.tif'
    main(result_dir, suffix, hru_id_raster, hru_dist_raster)
