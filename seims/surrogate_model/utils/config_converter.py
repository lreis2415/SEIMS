"""Convert JSON config to INI format for SEIMS."""
import json
import os
from configparser import ConfigParser


def json_to_ini(json_config: dict, base_ini: str, output_ini: str):
    """
    Convert surrogate model JSON config to SEIMS INI format.

    Args:
        json_config: Dictionary from JSON config
        base_ini: Base scenario_analysis.ini to copy BMPs settings from
        output_ini: Output INI file path
    """
    # Read base INI
    cf = ConfigParser()
    if os.path.exists(base_ini):
        cf.read(base_ini, encoding='utf-8')

    # Update [SEIMS_Model] section
    if not cf.has_section('SEIMS_Model'):
        cf.add_section('SEIMS_Model')

    seims = json_config['seims']
    cf.set('SEIMS_Model', 'MODEL_DIR', seims['model_dir'])
    cf.set('SEIMS_Model', 'BIN_DIR', seims['bin_dir'])
    cf.set('SEIMS_Model', 'MODEL_NAME', seims['model_name'])
    cf.set('SEIMS_Model', 'HOST', seims['host'])
    cf.set('SEIMS_Model', 'PORT', str(seims['port']))
    cf.set('SEIMS_Model', 'SCENARIO_DB', seims['scenario_db'])
    cf.set('SEIMS_Model', 'VERSION', 'OMP')
    cf.set('SEIMS_Model', 'NTHREAD', str(seims.get('nthread', 4)))
    cf.set('SEIMS_Model', 'LYRMTD', str(seims.get('lyrmtd', 0)))
    cf.set('SEIMS_Model', 'SCENARIO_ID', '0')

    # Simulation period
    eval_cfg = json_config['evaluation']
    cf.set('SEIMS_Model', 'SIMU_STIME', eval_cfg['eval_stime'])
    cf.set('SEIMS_Model', 'SIMU_ETIME', eval_cfg['eval_etime'])

    # Update [Scenario_Common] section
    if not cf.has_section('Scenario_Common'):
        cf.add_section('Scenario_Common')

    cf.set('Scenario_Common', 'Eval_Time_start', eval_cfg['eval_stime'])
    cf.set('Scenario_Common', 'Eval_Time_end', eval_cfg['eval_etime'])

    # Add missing optional fields with defaults
    if not cf.has_option('Scenario_Common', 'prioritize_key_bmps'):
        cf.set('Scenario_Common', 'prioritize_key_bmps', 'False')

    # Update spatial unit configuration if provided
    if 'spatial' in json_config:
        spatial = json_config['spatial']
        unit = spatial.get('unit', 'CONNFIELD')
        method = spatial.get('config_method', 'UPDOWN')

        # Update BMPs_cfg_units based on unit type
        if unit == 'SLPPOS':
            bmps_cfg = '{"SLPPOS": {"DISTRIBUTION": "RASTER|SLPPOS_UNITS", "UNITJSON": "slppos_3cls_units_updown.json", "SLPPOS_TAG_NAME": {"1": "summit", "4": "backslope", "16": "valley"}}}'
        elif unit == 'CONNFIELD':
            bmps_cfg = '{"CONNFIELD": {"DISTRIBUTION": "RASTER|FIELDS_15", "UNITJSON": "connected_field_units_updown_15.json"}}'
        else:
            bmps_cfg = None

        if bmps_cfg and cf.has_section('BMPs'):
            cf.set('BMPs', 'BMPs_cfg_units', bmps_cfg)
            cf.set('BMPs', 'BMPs_cfg_method', method)

    # Write to file
    with open(output_ini, 'w', encoding='utf-8') as f:
        cf.write(f)

    return output_ini
