move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav

copy WiFiConfig.json.produccion WiFiConfig.json
copy MQTTConfig.json.produccion MQTTConfig.json

del MQTTConfig.json.sav
del WiFiConfig.json.sav
 
date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
del AA_DESARROLLO 