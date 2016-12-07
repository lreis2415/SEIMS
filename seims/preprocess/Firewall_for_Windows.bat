set exePath=G:\Compile\SEIMS_Preprocess\Release
netsh advfirewall firewall add rule name=AreaD8 dir=in action=allow program=%exePath%\aread8.exe ENABLE=yes
netsh advfirewall firewall add rule name=D8FlowDir dir=in action=allow program=%exePath%\d8flowdir.exe ENABLE=yes
netsh advfirewall firewall add rule name=D8DistDownToStream dir=in action=allow program=%exePath%\d8distdowntostream.exe ENABLE=yes
netsh advfirewall firewall add rule name=DinfFlowDir dir=in action=allow program=%exePath%\dinfflowdir.exe ENABLE=yes
netsh advfirewall firewall add rule name=DropAnalysis dir=in action=allow program=%exePath%\dropanalysis.exe ENABLE=yes
netsh advfirewall firewall add rule name=Grid_Layering dir=in action=allow program=%exePath%\grid_layering.exe ENABLE=yes
netsh advfirewall firewall add rule name=Import_Raster dir=in action=allow program=%exePath%\import_raster.exe ENABLE=yes
netsh advfirewall firewall add rule name=IUH dir=in action=allow program=%exePath%\iuh.exe ENABLE=yes
netsh advfirewall firewall add rule name=Mask_Raster dir=in action=allow program=%exePath%\mask_raster.exe ENABLE=yes
netsh advfirewall firewall add rule name=MoveOutletsToStreams dir=in action=allow program=%exePath%\moveoutletstostreams.exe ENABLE=yes
netsh advfirewall firewall add rule name=PeukerDouglas dir=in action=allow program=%exePath%\peukerdouglas.exe ENABLE=yes
netsh advfirewall firewall add rule name=PitRemove dir=in action=allow program=%exePath%\pitremove.exe ENABLE=yes
netsh advfirewall firewall add rule name=Reclassify dir=in action=allow program=%exePath%\reclassify.exe ENABLE=yes
netsh advfirewall firewall add rule name=StreamNet dir=in action=allow program=%exePath%\streamnet.exe ENABLE=yes
netsh advfirewall firewall add rule name=Threshold dir=in action=allow program=%exePath%\threshold.exe ENABLE=yes
pause