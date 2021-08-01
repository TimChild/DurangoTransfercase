# DurangoTransfercase
Arduino code to control the transfer case of a 2005 Dodge Durango


Specifically control of an NV244 transfer case shift motor. 


Overall project was to upgrade from the 1 speed (NV144) transfer case that the 2005 Dodge Durango 4.7L SLT came with, to a 2 speed (NV244) transfer case which came in the 5.7L models. Annoyingly it seems that there is no way to reprogram the car for the combination of a 4.7L engine with the 2 speed transfer case as it was not an original option, and the PCM determines the behavior of the FCU which actually interacts with the T-case (i.e. replacing the FCU does not work as it's behaviour is determined by the PCM). 

So, Arduino controller it is...
