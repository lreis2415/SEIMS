import pandas as pd
import numpy as np
import io

def interpolate_file(input_file, output_file, timestep_seconds_new=30):
    try:
        # Read files manually to handle header lines
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        header_lines = []
        data_lines = []
        header_row_index = -1
        
        # Parse lines to separate metadata headers (#) from CSV data
        for i, line in enumerate(lines):
            if line.strip().startswith('#'):
                if 'TIMESTEP' in line:
                    header_lines.append(f'#TIMESTEP {timestep_seconds_new}\n')
                else:
                    header_lines.append(line)
            else:
                # The first non-# line is usually the CSV header
                if header_row_index == -1:
                    header_row_index = i
                data_lines.append(line)
                
        # Create a string buffer from data lines for pandas
        data_str = "".join(data_lines)
        
        # Read the csv data
        df = pd.read_csv(io.StringIO(data_str))
        
        # Identify date column
        date_col = None
        for col in df.columns:
            if 'DATE' in col.upper() or 'TIME' in col.upper():
                date_col = col
                break
        
        if date_col is None:
            # Fallback for PCP file where header might be "DATETIME,StationID" format
            if df.index.name and ('DATE' in df.index.name.upper() or 'TIME' in df.index.name.upper()):
                 df.reset_index(inplace=True)
                 date_col = df.columns[0]
            else:
                 print(f"Error: Could not find DateTime column in {input_file}")
                 return

        # Parse Datetime
        df[date_col] = pd.to_datetime(df[date_col])
        
        # Remove potential duplicates in index
        df = df.drop_duplicates(subset=[date_col])
        
        df.set_index(date_col, inplace=True)
        
        if df.index.duplicated().any():
             print("Warning: Duplicate index found even after drop_duplicates, keeping first")
             df = df[~df.index.duplicated(keep='first')]

        # Create new index
        start_time = df.index.min()
        end_time = df.index.max()
        new_index = pd.date_range(start=start_time, end=end_time, freq=f'{timestep_seconds_new}s')
        
        # Interpolate
        if 'PCP' in input_file:
             # Ffill for precipitation intensity (step function)
             df_resampled = df.reindex(new_index).ffill()
        else:
             # Linear interpolation for meteo data
             # Need to handle non-numeric columns (like StationID)
             # usually StationID is constant, so we can establish it separately
             numeric_cols = df.select_dtypes(include=[np.number]).columns
             non_numeric_cols = df.select_dtypes(exclude=[np.number]).columns
             
             df_num = df[numeric_cols].reindex(new_index).interpolate(method='linear')
             df_non_num = df[non_numeric_cols].reindex(new_index).ffill() # Station ID stays constant
             
             df_resampled = pd.concat([df_non_num, df_num], axis=1)

             # Fix: Ensure StationID is integer (reindex/interpolate converts int to float)
             if 'StationID' in df_resampled.columns:
                 df_resampled['StationID'] = df_resampled['StationID'].round(0).astype('Int64')


        # Reset index
        df_resampled.reset_index(inplace=True)
        df_resampled.rename(columns={'index': date_col}, inplace=True)
        
        # Format Date output
        if 'PCP' in input_file: # YYYY-MM-DD HH:MM:SS
             df_resampled[date_col] = df_resampled[date_col].dt.strftime('%Y-%m-%d %H:%M:%S')
        else: # YYYY/MM/DD HH:MM
             df_resampled[date_col] = df_resampled[date_col].dt.strftime('%Y/%m/%d %H:%M')

        # Write output
        with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
            f.writelines(header_lines)
            df_resampled.to_csv(f, index=False, float_format='%.2f')
            
        print(f"Successfully created {output_file}")

    except Exception as e:
        import traceback
        traceback.print_exc()
        print(f"Failed to process {input_file}: {e}")

# Process files
interpolate_file(r'e:\code\SEIMS\data\youwuzhen\data_prepare\climate\2015_METEO_UTCTIME_5.csv', 
                 r'e:\code\SEIMS\data\youwuzhen\data_prepare\climate\2015_METEO_UTCTIME_30s.csv')

interpolate_file(r'e:\code\SEIMS\data\youwuzhen\data_prepare\climate\2015_PCP_Intensity.csv', 
                 r'e:\code\SEIMS\data\youwuzhen\data_prepare\climate\2015_PCP_Intensity_30s.csv')
