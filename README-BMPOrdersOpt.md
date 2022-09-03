
## How to optimize BMP implementation orders

### 1. Getting familiar with SEIMS operations

Make sure you have read through SEIMS-UserManual.pdf and followed the steps described in it and the execution commands for the major steps.

### 2. Recompile

Since the C++ code of this branch has been modified, SEIMS needs to be recompiled to use this branch. You can follow the commands on Linux:

```
cd ~/Programs/SEIMS
mkdir build
cd build
cmake ..
make -j4
make install
```

Or on Windows:

```
cd D:\demo\SEIMS
d:
mkdir build
cd build
cmake -G "Visual Studio 12 2013 Win64" ..
msbuild.exe ALL_BUILD.vcxproj /p:Configuration=Release
msbuild.exe INSTALL.vcxproj /p:Configuration=Release
```

### 3. Data preprocessing

Run the preprocessing script to re-import the updated meteorological and precipitation data, sample spatial data at 30m resolution, BMP stepwise economic benefit and environmental effectiveness, and model output configurations into the database.
Use the commands below to run the preprocessing script:
```
cd ~/Programs/SEIMS/seims/preprocess
python main.py -ini ~/Programs/SEIMS/data/youwuzhen/workspace/preprocess.ini
```

### 4. Run the model

After the data import is complete, you can run the model to check whether the calculation results are correct. Calibrated parameters (param.cali) for 10m resolution data are provided in the model folder for demonstration purposes only. You can calibrate the model according to your needs. Please refer to the relative chapters in SEIMS-UserManual.pdf.

Use the commands below to run the model:
```
cd ~/Programs/SEIMS/seims
python run_seims.py -ini ~/Programs/SEIMS/data/youwuzhen/workspace/runmodel.ini
```

### 5. Scenario analysis

You can review whether you need to optimize spatial configurations of the BMP scenarios first according to your own situation. The scenario in the Pareto fronts of this step can be used as the starting scenario for the next step: optimization of the implementation orders of BMPs. Alternatively, the spatial configuration of the BMP scenario developed by the expert's experience can be selected.

Use the commands below to run scenario analysis:
```
cd ~/Programs/SEIMS/seims/scenario_analysis/spatialunits
python main_nsga2.py -ini ~/Programs/SEIMS/data/youwuzhen/workspace/scenario_analysis.ini
```
### 6. optimization of the implementation orders of BMPs

#### 6.1 Starting scenario

Starting scenario information must be provided as text in the model folder containing the Scenario ID, Gene number, and Gene values. For demo data, please refer to the file 'Scenario_196508708.txt' in the model folder.

> Scenario ID: 196508708
> Gene number: 105
> Gene values: 0.0, 2.0, 2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 2.0, 2.0, 0.0, 2.0, 0.0, 0.0,0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 0.0, 2.0, 0.0, 1.0, 1.0, 0.0, 2.0, 2.0, 0.0, 2.0, 2.0, 0.0, 0.0, 2.0, 0.0, 2.0, 2.0, 0.0, 2.0, 0.0, 0.0, 1.0, 3.0, 2.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 2.0, 0.0, 2.0, 2.0, 1.0, 0.0, 0.0, 2.0, 2.0, 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 1.0, 3.0, 4.0, 1.0, 3.0, 0.0, 1.0, 3.0, 0.0, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0

#### 6.2 Configuration file

The configuration file scenario_analysis_bmps_order.ini adds some configuration items for the BMPs implementation orders, which are explained as follows.


> # implementation periods (in year)
> implementation_period = 5
> # whether BMPs effectiveness variable and, if so, what is the change frequency (in year)
> effectiveness_changeable = True
> change_frequency = 1
> # whether to consider stepwise investment and, if so, what are the respective investment limits.
> enable_investment_quota = False
> investment_each_period = [90, 70, 30, 20, 20]
> # the discount rate during the stepwise investment periods
> discount_rate = 0.1
> # starting scenario to optimize BMP implementation oders 
> selected_scenario_file = Scenario_196508708.txt
> # average annual sediment yield and annual sediment yield of the baseline scenario
> Eval_info = {"OUTPUTID": "SED_OL", "ENVEVAL": "SED_OL_SUM.tif", "BASE_ENV": 1688493,"BASE_SED_PERIODS":[1347426, 818153, 1009664, 3634481, 1632739]}

#### 6.3 Run the optimization of implementation orders of BMPs

Use the commands below to run the optimization of implementation orders of BMPs:
```
cd ~/Programs/SEIMS/seims/scenario_analysis/spatialunits
python bmps_order_nsga2.py -ini ~/Programs/SEIMS/data/youwuzhen/workspace/scenario_analysis_bmps_order.ini
```
