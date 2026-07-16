this branch is used for bitbanging the circuitboard to make sure everything works according to the predefined parameters  
(i.e. not too much signal bleeding (above 1V), Pin B receives Signal from Pin A succesfully without too much distortion, etc.)  

IMPORTANT: Change the platformio build-src-filter according to what bitbanging process you wanna use, otherwise the same main.cpp file will always compile
For more details check out the bitbanging documentation (pdf and excel sheet) which documents expected values, test procedures and more

