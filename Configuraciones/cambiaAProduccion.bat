del D:\arduino\desarrollos\Sketchs\Termostato\Codigo\ControladorM5s\data\*.* /q /s

xcopy .\produccion\*.* D:\arduino\desarrollos\Sketchs\Termostato\Codigo\ControladorM5s\data /S /Y

date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
xcopy AA_PRODUCCION D:\arduino\desarrollos\Sketchs\Termostato\Codigo\ControladorM5s\data /Y
del AA_DESARROLLO 