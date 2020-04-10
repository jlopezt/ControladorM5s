move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move GHNConfig.json GHNConfig.json.sav
move Mapa.json Mapa.json.sav
move RelesConfig.json RelesConfig.json.sav
move temperaturas.json temperaturas.json.sav
move TermometrosConfig.json TermometrosConfig.json.sav

copy Config.json.desarrollo Config.json
copy WiFiConfig.json.desarrollo WiFiConfig.json
copy MQTTConfig.json.desarrollo MQTTConfig.json
copy GHNConfig.json.desarrollo GHNConfig.json
copy Mapa.json.desarrollo Mapa.json
copy RelesConfig.json.desarrollo RelesConfig.json
copy temperaturas.json.desarrollo temperaturas.json
copy TermometrosConfig.json.desarrollo TermometrosConfig.json


del Config.json.sav
del WiFiConfig.json.sav
del MQTTConfig.json.sav
del GHNConfig.json.sav
del Mapa.json.sav
del RelesConfig.json.sav
del temperaturas.json.sav
del TermometrosConfig.json.sav

date /T>AA_DESARROLLO
time /T>>AA_DESARROLLO 
del AA_PRODUCCION