2.4 Running a SEIMS-based waterhsed model {#run_seims_model}
============================================================

[TOC]

# Simple usage
For simple usage, open a terminal (e.g., CMD on Window), enter the following commands to execute the predefined SEIMS-based watershed model of the Youwuzhen watershed (hereafter referred to as the Youwuzhen watershed model):
```python
cd D:\demo\SEIMS\seims\test
D:
python demo_runmodel.py –name youwuzhen
```

The simulation will be finished in a ~28 seconds (Figure 1). The predefined output information can be found in the model folder (i.e., `MODEL_DIR\OUTPUT0`), such as Figure 2.

![ywzrunmodelsimple](../../img/screenshot-runmodel-simple-usage.jpg) <img src="screenshot-runmodel-simple-usage.jpg" alt="ywzrunmodelsimple" width="400"/>

Figure 1 Simple usage of the OpenMP version of the Youwuzhen watershed model

 ![ywzrunmodeloutput0](../../img/screenshot-runmodel-output0-simple-usage.jpg) <img src="screenshot-runmodel-output0-simple-usage.jpg" alt="ywzrunmodeloutput0" width="400"/>

Figure 2 Predefined outputs of the Youwuzhen watershed model

