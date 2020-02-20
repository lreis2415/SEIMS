# Changelog
## 2018-10-17
疏林地的土地利用类型目前根据陈志彪等（2013, p124）书中的设置，设为16-Range-Brush，而该类型的IDC为CROP_IDC_PERENNIAL，实际上应为CROP_IDC_TREES。
游屋圳小流域林地主要为常绿阔叶林，根据SWAT的Crop查找表，有以下4种比较合适：
 + 6: FRST, Forest-Mixed
 + 8: FRSE, Forest-Evergreen
 + 133: FOEB, Evergreen-Broadleaf-forest
 + 135: FOMI, Mixed Forest
仔细查看参数可知，6和135、8和133基本一致.
另外，陈志彪书中将有林地设置为Forest-Decidudous落叶林是错误的。
因此有林地设置为8，疏林地设置为6
即将原来疏林地的编号16(RNGB)改为6(FRST)，将7(FRSD)改为8(FRSE)。

