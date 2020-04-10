move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move GHNConfig.json GHNConfig.json.sav
move Mapa.json Mapa.json.sav
move RelesConfig.json RelesConfig.json.sav
move temperaturas.json temperaturas.json.sav
move TermometrosConfig.json TermometrosConfig.json.sav

copy Config.json.produccion Config.json
copy WiFiConfig.json.produccion WiFiConfig.json
copy MQTTConfig.json.produccion MQTTConfig.json
copy GHNConfig.json.produccion GHNConfig.json
copy Mapa.json.produccion Mapa.json
copy RelesConfig.json.produccion RelesConfig.json
copy temperaturas.json.produccion temperaturas.json
copy TermometrosConfig.json.produccion TermometrosConfig.json


del Config.json.sav
del WiFiConfig.json.sav
del MQTTConfig.json.sav
del GHNConfig.json.sav
del Mapa.json.sav
del RelesConfig.json.sav
del temperaturas.json.sav
del TermometrosConfig.json.sav
 
date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
del AA_DESARROLLO 