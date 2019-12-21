move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav

copy WiFiConfig.json.desarrollo WiFiConfig.json
copy MQTTConfig.json.desarrollo MQTTConfig.json

del MQTTConfig.json.sav
del WiFiConfig.json.sav

date /T>AA_DESARROLLO
time /T>>AA_DESARROLLO 
del AA_PRODUCCION