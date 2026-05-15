#!/usr/bin/env python3
"""Fix soil raster data in MongoDB GridFS for Andrews Forest.

Reads soil_properties_lookup.csv, recomputes soil parameters per cell
based on 0_SOILTYPE (and 1-5_SOILTYPE), and overwrites the corrupted
GridFS files (0_FIELDCAP, 0_POROSITY, etc.).
"""
import struct
import numpy as np
from pymongo import MongoClient
from gridfs import GridFS

# Connect to MongoDB
client = MongoClient('127.0.0.1', 27017)
db = client['andrews_forest_model']
spatial_fs = GridFS(db, 'SPATIAL')

# ------------------------------------------------------------------
# 1. Parse soil_properties_lookup.csv
# ------------------------------------------------------------------
import csv

soil_params = {}  # seqn -> dict of params
max_layers = 9    # GridFS统一填充到9层

with open('data/AndrewsForest/data_prepare/lookup/soil_properties_lookup.csv') as f:
    reader = csv.reader(f)
    header = next(reader)
    while header[0].startswith('#'):
        header = next(reader)
    
    idx = {h: i for i, h in enumerate(header)}
    
    for row in reader:
        if not row or row[0].startswith('#'):
            continue
        seqn = int(row[idx['SEQN']])
        nlyr = int(row[idx['SOILLAYERS']])
        
        # Parse multi-layer values
        def parse_multivalue(s):
            return [float(x) for x in s.split('-')]
        
        z_vals = parse_multivalue(row[idx['SOL_Z']])
        fc_vals = parse_multivalue(row[idx['SOL_FC']])
        por_vals = parse_multivalue(row[idx['SOL_POROSITY']])
        clay_vals = parse_multivalue(row[idx['SOL_CLAY']])
        sand_vals = parse_multivalue(row[idx['SOL_SAND']])
        bd_vals = parse_multivalue(row[idx['SOL_BD']])
        k_vals = parse_multivalue(row[idx['SOL_K']])
        wp_vals = parse_multivalue(row[idx['SOL_WP']])
        
        # Add septic layer (0-10mm) if z0 >= 20mm
        if z_vals[0] >= 20.0:
            nlyr += 1
            z_vals.insert(0, 10.0)
            fc_vals.insert(0, fc_vals[0])
            por_vals.insert(0, por_vals[0])
            clay_vals.insert(0, clay_vals[0])
            sand_vals.insert(0, sand_vals[0])
            bd_vals.insert(0, bd_vals[0])
            k_vals.insert(0, k_vals[0])
            wp_vals.insert(0, wp_vals[0])
        
        # Compute thickness
        thick_vals = []
        for i in range(nlyr):
            if i == 0:
                thick_vals.append(z_vals[i])
            else:
                thick_vals.append(z_vals[i] - z_vals[i-1])
        
        # Pad to max_layers by repeating last layer
        def pad(arr):
            while len(arr) < max_layers:
                arr.append(arr[-1])
            return arr
        
        soil_params[seqn] = {
            'SOILLAYERS': nlyr,
            'SOILDEPTH': pad(z_vals),
            'SOILTHICK': pad(thick_vals),
            'FIELDCAP': pad(fc_vals),
            'POROSITY': pad(por_vals),
            'CLAY': pad(clay_vals),
            'SAND': pad(sand_vals),
            'DENSITY': pad(bd_vals),
            'CONDUCTIVITY': pad(k_vals),
            'WILTINGPOINT': pad(wp_vals),
        }

print(f'Loaded soil params for {len(soil_params)} SEQN values')
print(f'Max layers: {max_layers}')

# ------------------------------------------------------------------
# 2. Helper: read SOILTYPE from GridFS
# ------------------------------------------------------------------
def read_gridfs_int32(filename):
    f = db['SPATIAL.files'].find_one({'filename': filename})
    if f is None:
        return None, None
    count = f['metadata']['CELLSNUM']
    chunks = db['SPATIAL.chunks'].find({'files_id': f['_id']}, sort=[('n', 1)])
    data = b''.join(c['data'] for c in chunks)
    arr = np.array(struct.unpack(f'<{count}i', data[:count*4]), dtype=np.int32)
    return arr, f

# ------------------------------------------------------------------
# 3. Helper: write double raster to GridFS
# ------------------------------------------------------------------
def write_gridfs_double(filename, arr_layers, metadata_template):
    """arr_layers: shape (max_layers, n_cells) layer-first"""
    layers, n_cells = arr_layers.shape
    assert layers == max_layers
    
    # Flatten in layer-first order
    buf = arr_layers.astype(np.float64).tobytes()
    
    # Build metadata from template, updating LAYERS and DATATYPE
    meta = dict(metadata_template['metadata'])
    meta['LAYERS'] = layers
    meta['DATATYPE'] = 'DOUBLE'
    meta['DATATYPE_OUT'] = 'DOUBLE'
    
    # Remove old file if exists
    if spatial_fs.exists(filename=filename):
        old = spatial_fs.get_version(filename=filename)
        spatial_fs.delete(old._id)
    
    spatial_fs.put(buf, filename=filename, metadata=meta, content_type='NumericStream')
    print(f'  Written {filename}: {n_cells} cells x {layers} layers')

# ------------------------------------------------------------------
# 4. Process each subbasin
# ------------------------------------------------------------------
# Files to fix per subbasin
file_map = {
    'FIELDCAP': 'FIELDCAP',
    'POROSITY': 'POROSITY',
    'SOILDEPTH': 'SOILDEPTH',
    'SOILTHICK': 'SOILTHICK',
    'CLAY': 'CLAY',
    'SAND': 'SAND',
    'DENSITY': 'DENSITY',
    'CONDUCTIVITY': 'CONDUCTIVITY',
}

for subid in range(6):
    soiltype_file = f'{subid}_SOILTYPE'
    arr_type, f_meta = read_gridfs_int32(soiltype_file)
    if arr_type is None:
        print(f'Subbasin {subid}: {soiltype_file} not found, skipping')
        continue
    
    n_cells = len(arr_type)
    print(f'\nSubbasin {subid}: {n_cells} cells')
    
    # Prepare output arrays (layer-first: layers x cells)
    out_arrays = {k: np.zeros((max_layers, n_cells), dtype=np.float64) for k in file_map}
    out_arrays['SOILLAYERS'] = np.zeros(n_cells, dtype=np.float64)
    
    for i in range(n_cells):
        seqn = int(arr_type[i])
        if seqn not in soil_params:
            print(f'  WARNING: SEQN {seqn} not in lookup table (cell {i})')
            continue
        sp = soil_params[seqn]
        out_arrays['SOILLAYERS'][i] = sp['SOILLAYERS']
        for lyr in range(max_layers):
            out_arrays['FIELDCAP'][lyr, i] = sp['FIELDCAP'][lyr]
            out_arrays['POROSITY'][lyr, i] = sp['POROSITY'][lyr]
            out_arrays['SOILDEPTH'][lyr, i] = sp['SOILDEPTH'][lyr]
            out_arrays['SOILTHICK'][lyr, i] = sp['SOILTHICK'][lyr]
            out_arrays['CLAY'][lyr, i] = sp['CLAY'][lyr]
            out_arrays['SAND'][lyr, i] = sp['SAND'][lyr]
            out_arrays['DENSITY'][lyr, i] = sp['DENSITY'][lyr]
            out_arrays['CONDUCTIVITY'][lyr, i] = sp['CONDUCTIVITY'][lyr]
    
    # Read existing metadata template from one of the files (e.g., FIELDCAP)
    template_file = f'{subid}_FIELDCAP'
    template = db['SPATIAL.files'].find_one({'filename': template_file})
    if template is None:
        print(f'  WARNING: {template_file} not found, cannot get metadata template')
        continue
    
    # Write each file
    for key, gridfs_name in file_map.items():
        fname = f'{subid}_{gridfs_name}'
        if key == 'SOILLAYERS':
            # SOILLAYERS is 1-layer
            arr = out_arrays['SOILLAYERS'].reshape((1, n_cells))
            # Update metadata for 1 layer
            meta = dict(template['metadata'])
            meta['LAYERS'] = 1
            buf = arr.astype(np.float64).tobytes()
            if spatial_fs.exists(filename=fname):
                spatial_fs.delete(spatial_fs.get_version(filename=fname)._id)
            spatial_fs.put(buf, filename=fname, metadata=meta, content_type='NumericStream')
            print(f'  Written {fname}: {n_cells} cells x 1 layer')
        else:
            write_gridfs_double(fname, out_arrays[key], template)

print('\nDone! Soil GridFS data fixed.')
