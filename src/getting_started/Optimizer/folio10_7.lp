\ Using Xpress-MP extensions
Maximize
 5.29 frac(treasury1) + 2.8 frac(treasury2) + 3.59 frac(treasury3) + 6.97 frac(treasury4) + 
3.3 frac(treasury5) + 3.02 frac(treasury6) + 2.98 frac(treasury7) + 3.41 frac(treasury8) + 
3.93 frac(treasury9) + 3.86 frac(treasury10) + 11.36 frac(hardware1) + 17.23 frac(hardware2) + 
16.31 frac(hardware3) + 21.1 frac(hardware4) + 13.76 frac(hardware5) + 9.31 frac(hardware6) + 
16.99 frac(hardware7) + 24.85 frac(hardware8) + 18.52 frac(hardware9) + 12.79 frac(hardware10) + 
36.1 frac(theater1) + 19.4 frac(theater2) + 23.5 frac(theater3) + 34.61 frac(theater4) + 
16.91 frac(theater5) + 27.04 frac(theater6) + 25.82 frac(theater7) + 24.99 frac(theater8) + 
36.89 frac(theater9) + 31.71 frac(theater10) + 10.89 frac(telecom1) + 17.98 frac(telecom2) + 
12.31 frac(telecom3) + 6.53 frac(telecom4) + 6.11 frac(telecom5) + 15.89 frac(telecom6) + 
12.46 frac(telecom7) + 11.11 frac(telecom8) + 16.6 frac(telecom9) + 13.93 frac(telecom10) + 
8.79 frac(brewery1) + 9.35 frac(brewery2) + 8.88 frac(brewery3) + 11.63 frac(brewery4) + 
6.21 frac(brewery5) + 9.26 frac(brewery6) + 4.77 frac(brewery7) + 5.98 frac(brewery8) + 
10.92 frac(brewery9) + 5.26 frac(brewery10) + 5.39 frac(highways1) + 4.85 frac(highways2) + 
8.34 frac(highways3) + 8.48 frac(highways4) + 6.42 frac(highways5) + 11.04 frac(highways6) + 
13.15 frac(highways7) + 10.01 frac(highways8) + 7.71 frac(highways9) + 11.07 frac(highways10) + 
5.74 frac(cars1) + 7.13 frac(cars2) + 8.75 frac(cars3) + 5.08 frac(cars4) + 
6.23 frac(cars5) + 8.19 frac(cars6) + 8.03 frac(cars7) + 5.96 frac(cars8) + 
4.17 frac(cars9) + 8.11 frac(cars10) + 6.79 frac(bank1) + 3.01 frac(bank2) + 
4.92 frac(bank3) + 4.14 frac(bank4) + 8.98 frac(bank5) + 8.81 frac(bank6) + 
4.7 frac(bank7) + 8.06 frac(bank8) + 6.22 frac(bank9) + 4.47 frac(bank10) + 
34.59 frac(software1) + 44.94 frac(software2) + 43.05 frac(software3) + 15.58 frac(software4) + 
42.05 frac(software5) + 20.66 frac(software6) + 20.76 frac(software7) + 19.85 frac(software8) + 
20.05 frac(software9) + 45.38 frac(software10) + 23.07 frac(electronics1) + 
19.1 frac(electronics2) + 23.83 frac(electronics3) + 16.54 frac(electronics4) + 
28.33 frac(electronics5) + 25.88 frac(electronics6) + 22.14 frac(electronics7) + 
22.65 frac(electronics8) + 12.59 frac(electronics9) + 28.1 frac(electronics10)

Subject To
_R1: frac(electronics10) - 0.1 buy(electronics10) >= 0
_R2: frac(electronics10) - 0.2 buy(electronics10) <= 0
_R3: frac(electronics9) - 0.1 buy(electronics9) >= 0
_R4: frac(electronics9) - 0.2 buy(electronics9) <= 0
_R5: frac(electronics8) - 0.1 buy(electronics8) >= 0
_R6: frac(electronics8) - 0.2 buy(electronics8) <= 0
_R7: frac(electronics7) - 0.1 buy(electronics7) >= 0
_R8: frac(electronics7) - 0.2 buy(electronics7) <= 0
_R9: frac(electronics6) - 0.1 buy(electronics6) >= 0
_R10: frac(electronics6) - 0.2 buy(electronics6) <= 0
_R11: frac(electronics5) - 0.1 buy(electronics5) >= 0
_R12: frac(electronics5) - 0.2 buy(electronics5) <= 0
_R13: frac(electronics4) - 0.1 buy(electronics4) >= 0
_R14: frac(electronics4) - 0.2 buy(electronics4) <= 0
_R15: frac(electronics3) - 0.1 buy(electronics3) >= 0
_R16: frac(electronics3) - 0.2 buy(electronics3) <= 0
_R17: frac(electronics2) - 0.1 buy(electronics2) >= 0
_R18: frac(electronics2) - 0.2 buy(electronics2) <= 0
_R19: frac(electronics1) - 0.1 buy(electronics1) >= 0
_R20: frac(electronics1) - 0.2 buy(electronics1) <= 0
_R21: frac(software10) - 0.1 buy(software10) >= 0
_R22: frac(software10) - 0.2 buy(software10) <= 0
_R23: frac(software9) - 0.1 buy(software9) >= 0
_R24: frac(software9) - 0.2 buy(software9) <= 0
_R25: frac(software8) - 0.1 buy(software8) >= 0
_R26: frac(software8) - 0.2 buy(software8) <= 0
_R27: frac(software7) - 0.1 buy(software7) >= 0
_R28: frac(software7) - 0.2 buy(software7) <= 0
_R29: frac(software6) - 0.1 buy(software6) >= 0
_R30: frac(software6) - 0.2 buy(software6) <= 0
_R31: frac(software5) - 0.1 buy(software5) >= 0
_R32: frac(software5) - 0.2 buy(software5) <= 0
_R33: frac(software4) - 0.1 buy(software4) >= 0
_R34: frac(software4) - 0.2 buy(software4) <= 0
_R35: frac(software3) - 0.1 buy(software3) >= 0
_R36: frac(software3) - 0.2 buy(software3) <= 0
_R37: frac(software2) - 0.1 buy(software2) >= 0
_R38: frac(software2) - 0.2 buy(software2) <= 0
_R39: frac(software1) - 0.1 buy(software1) >= 0
_R40: frac(software1) - 0.2 buy(software1) <= 0
_R41: frac(bank10) - 0.1 buy(bank10) >= 0
_R42: frac(bank10) - 0.2 buy(bank10) <= 0
_R43: frac(bank9) - 0.1 buy(bank9) >= 0
_R44: frac(bank9) - 0.2 buy(bank9) <= 0
_R45: frac(bank8) - 0.1 buy(bank8) >= 0
_R46: frac(bank8) - 0.2 buy(bank8) <= 0
_R47: frac(bank7) - 0.1 buy(bank7) >= 0
_R48: frac(bank7) - 0.2 buy(bank7) <= 0
_R49: frac(bank6) - 0.1 buy(bank6) >= 0
_R50: frac(bank6) - 0.2 buy(bank6) <= 0
_R51: frac(bank5) - 0.1 buy(bank5) >= 0
_R52: frac(bank5) - 0.2 buy(bank5) <= 0
_R53: frac(bank4) - 0.1 buy(bank4) >= 0
_R54: frac(bank4) - 0.2 buy(bank4) <= 0
_R55: frac(bank3) - 0.1 buy(bank3) >= 0
_R56: frac(bank3) - 0.2 buy(bank3) <= 0
_R57: frac(bank2) - 0.1 buy(bank2) >= 0
_R58: frac(bank2) - 0.2 buy(bank2) <= 0
_R59: frac(bank1) - 0.1 buy(bank1) >= 0
_R60: frac(bank1) - 0.2 buy(bank1) <= 0
_R61: frac(cars10) - 0.1 buy(cars10) >= 0
_R62: frac(cars10) - 0.2 buy(cars10) <= 0
_R63: frac(cars9) - 0.1 buy(cars9) >= 0
_R64: frac(cars9) - 0.2 buy(cars9) <= 0
_R65: frac(cars8) - 0.1 buy(cars8) >= 0
_R66: frac(cars8) - 0.2 buy(cars8) <= 0
_R67: frac(cars7) - 0.1 buy(cars7) >= 0
_R68: frac(cars7) - 0.2 buy(cars7) <= 0
_R69: frac(cars6) - 0.1 buy(cars6) >= 0
_R70: frac(cars6) - 0.2 buy(cars6) <= 0
_R71: frac(cars5) - 0.1 buy(cars5) >= 0
_R72: frac(cars5) - 0.2 buy(cars5) <= 0
_R73: frac(cars4) - 0.1 buy(cars4) >= 0
_R74: frac(cars4) - 0.2 buy(cars4) <= 0
_R75: frac(cars3) - 0.1 buy(cars3) >= 0
_R76: frac(cars3) - 0.2 buy(cars3) <= 0
_R77: frac(cars2) - 0.1 buy(cars2) >= 0
_R78: frac(cars2) - 0.2 buy(cars2) <= 0
_R79: frac(cars1) - 0.1 buy(cars1) >= 0
_R80: frac(cars1) - 0.2 buy(cars1) <= 0
_R81: frac(highways10) - 0.1 buy(highways10) >= 0
_R82: frac(highways10) - 0.2 buy(highways10) <= 0
_R83: frac(highways9) - 0.1 buy(highways9) >= 0
_R84: frac(highways9) - 0.2 buy(highways9) <= 0
_R85: frac(highways8) - 0.1 buy(highways8) >= 0
_R86: frac(highways8) - 0.2 buy(highways8) <= 0
_R87: frac(highways7) - 0.1 buy(highways7) >= 0
_R88: frac(highways7) - 0.2 buy(highways7) <= 0
_R89: frac(highways6) - 0.1 buy(highways6) >= 0
_R90: frac(highways6) - 0.2 buy(highways6) <= 0
_R91: frac(highways5) - 0.1 buy(highways5) >= 0
_R92: frac(highways5) - 0.2 buy(highways5) <= 0
_R93: frac(highways4) - 0.1 buy(highways4) >= 0
_R94: frac(highways4) - 0.2 buy(highways4) <= 0
_R95: frac(highways3) - 0.1 buy(highways3) >= 0
_R96: frac(highways3) - 0.2 buy(highways3) <= 0
_R97: frac(highways2) - 0.1 buy(highways2) >= 0
_R98: frac(highways2) - 0.2 buy(highways2) <= 0
_R99: frac(highways1) - 0.1 buy(highways1) >= 0
_R100: frac(highways1) - 0.2 buy(highways1) <= 0
_R101: frac(brewery10) - 0.1 buy(brewery10) >= 0
_R102: frac(brewery10) - 0.2 buy(brewery10) <= 0
_R103: frac(brewery9) - 0.1 buy(brewery9) >= 0
_R104: frac(brewery9) - 0.2 buy(brewery9) <= 0
_R105: frac(brewery8) - 0.1 buy(brewery8) >= 0
_R106: frac(brewery8) - 0.2 buy(brewery8) <= 0
_R107: frac(brewery7) - 0.1 buy(brewery7) >= 0
_R108: frac(brewery7) - 0.2 buy(brewery7) <= 0
_R109: frac(brewery6) - 0.1 buy(brewery6) >= 0
_R110: frac(brewery6) - 0.2 buy(brewery6) <= 0
_R111: frac(brewery5) - 0.1 buy(brewery5) >= 0
_R112: frac(brewery5) - 0.2 buy(brewery5) <= 0
_R113: frac(brewery4) - 0.1 buy(brewery4) >= 0
_R114: frac(brewery4) - 0.2 buy(brewery4) <= 0
_R115: frac(brewery3) - 0.1 buy(brewery3) >= 0
_R116: frac(brewery3) - 0.2 buy(brewery3) <= 0
_R117: frac(brewery2) - 0.1 buy(brewery2) >= 0
_R118: frac(brewery2) - 0.2 buy(brewery2) <= 0
_R119: frac(brewery1) - 0.1 buy(brewery1) >= 0
_R120: frac(brewery1) - 0.2 buy(brewery1) <= 0
_R121: frac(telecom10) - 0.1 buy(telecom10) >= 0
_R122: frac(telecom10) - 0.2 buy(telecom10) <= 0
_R123: frac(telecom9) - 0.1 buy(telecom9) >= 0
_R124: frac(telecom9) - 0.2 buy(telecom9) <= 0
_R125: frac(telecom8) - 0.1 buy(telecom8) >= 0
_R126: frac(telecom8) - 0.2 buy(telecom8) <= 0
_R127: frac(telecom7) - 0.1 buy(telecom7) >= 0
_R128: frac(telecom7) - 0.2 buy(telecom7) <= 0
_R129: frac(telecom6) - 0.1 buy(telecom6) >= 0
_R130: frac(telecom6) - 0.2 buy(telecom6) <= 0
_R131: frac(telecom5) - 0.1 buy(telecom5) >= 0
_R132: frac(telecom5) - 0.2 buy(telecom5) <= 0
_R133: frac(telecom4) - 0.1 buy(telecom4) >= 0
_R134: frac(telecom4) - 0.2 buy(telecom4) <= 0
_R135: frac(telecom3) - 0.1 buy(telecom3) >= 0
_R136: frac(telecom3) - 0.2 buy(telecom3) <= 0
_R137: frac(telecom2) - 0.1 buy(telecom2) >= 0
_R138: frac(telecom2) - 0.2 buy(telecom2) <= 0
_R139: frac(telecom1) - 0.1 buy(telecom1) >= 0
_R140: frac(telecom1) - 0.2 buy(telecom1) <= 0
_R141: frac(theater10) - 0.1 buy(theater10) >= 0
_R142: frac(theater10) - 0.2 buy(theater10) <= 0
_R143: frac(theater9) - 0.1 buy(theater9) >= 0
_R144: frac(theater9) - 0.2 buy(theater9) <= 0
_R145: frac(theater8) - 0.1 buy(theater8) >= 0
_R146: frac(theater8) - 0.2 buy(theater8) <= 0
_R147: frac(theater7) - 0.1 buy(theater7) >= 0
_R148: frac(theater7) - 0.2 buy(theater7) <= 0
_R149: frac(theater6) - 0.1 buy(theater6) >= 0
_R150: frac(theater6) - 0.2 buy(theater6) <= 0
_R151: frac(theater5) - 0.1 buy(theater5) >= 0
_R152: frac(theater5) - 0.2 buy(theater5) <= 0
_R153: frac(theater4) - 0.1 buy(theater4) >= 0
_R154: frac(theater4) - 0.2 buy(theater4) <= 0
_R155: frac(theater3) - 0.1 buy(theater3) >= 0
_R156: frac(theater3) - 0.2 buy(theater3) <= 0
_R157: frac(theater2) - 0.1 buy(theater2) >= 0
_R158: frac(theater2) - 0.2 buy(theater2) <= 0
_R159: frac(theater1) - 0.1 buy(theater1) >= 0
_R160: frac(theater1) - 0.2 buy(theater1) <= 0
_R161: frac(hardware10) - 0.1 buy(hardware10) >= 0
_R162: frac(hardware10) - 0.2 buy(hardware10) <= 0
_R163: frac(hardware9) - 0.1 buy(hardware9) >= 0
_R164: frac(hardware9) - 0.2 buy(hardware9) <= 0
_R165: frac(hardware8) - 0.1 buy(hardware8) >= 0
_R166: frac(hardware8) - 0.2 buy(hardware8) <= 0
_R167: frac(hardware7) - 0.1 buy(hardware7) >= 0
_R168: frac(hardware7) - 0.2 buy(hardware7) <= 0
_R169: frac(hardware6) - 0.1 buy(hardware6) >= 0
_R170: frac(hardware6) - 0.2 buy(hardware6) <= 0
_R171: frac(hardware5) - 0.1 buy(hardware5) >= 0
_R172: frac(hardware5) - 0.2 buy(hardware5) <= 0
_R173: frac(hardware4) - 0.1 buy(hardware4) >= 0

_R174: frac(hardware4) - 0.2 buy(hardware4) <= 0
_R175: frac(hardware3) - 0.1 buy(hardware3) >= 0
_R176: frac(hardware3) - 0.2 buy(hardware3) <= 0
_R177: frac(hardware2) - 0.1 buy(hardware2) >= 0
_R178: frac(hardware2) - 0.2 buy(hardware2) <= 0
_R179: frac(hardware1) - 0.1 buy(hardware1) >= 0
_R180: frac(hardware1) - 0.2 buy(hardware1) <= 0
_R181: frac(treasury10) - 0.1 buy(treasury10) >= 0
_R182: frac(treasury10) - 0.2 buy(treasury10) <= 0
_R183: frac(treasury9) - 0.1 buy(treasury9) >= 0
_R184: frac(treasury9) - 0.2 buy(treasury9) <= 0
_R185: frac(treasury8) - 0.1 buy(treasury8) >= 0
_R186: frac(treasury8) - 0.2 buy(treasury8) <= 0
_R187: frac(treasury7) - 0.1 buy(treasury7) >= 0
_R188: frac(treasury7) - 0.2 buy(treasury7) <= 0
_R189: frac(treasury6) - 0.1 buy(treasury6) >= 0
_R190: frac(treasury6) - 0.2 buy(treasury6) <= 0
_R191: frac(treasury5) - 0.1 buy(treasury5) >= 0
_R192: frac(treasury5) - 0.2 buy(treasury5) <= 0
_R193: frac(treasury4) - 0.1 buy(treasury4) >= 0
_R194: frac(treasury4) - 0.2 buy(treasury4) <= 0
_R195: frac(treasury3) - 0.1 buy(treasury3) >= 0
_R196: frac(treasury3) - 0.2 buy(treasury3) <= 0
_R197: frac(treasury2) - 0.1 buy(treasury2) >= 0
_R198: frac(treasury2) - 0.2 buy(treasury2) <= 0
_R199: frac(treasury1) - 0.1 buy(treasury1) >= 0
_R200: frac(treasury1) - 0.2 buy(treasury1) <= 0
_R201: buy(treasury1) + buy(treasury2) + buy(treasury3) + buy(treasury4) + buy(treasury5) + 
buy(treasury6) + buy(treasury7) + buy(treasury8) + buy(treasury9) + buy(treasury10) + 
buy(hardware1) + buy(hardware2) + buy(hardware3) + buy(hardware4) + buy(hardware5) + 
buy(hardware6) + buy(hardware7) + buy(hardware8) + buy(hardware9) + buy(hardware10) + 
buy(theater1) + buy(theater2) + buy(theater3) + buy(theater4) + buy(theater5) + 
buy(theater6) + buy(theater7) + buy(theater8) + buy(theater9) + buy(theater10) + 
buy(telecom1) + buy(telecom2) + buy(telecom3) + buy(telecom4) + buy(telecom5) + 
buy(telecom6) + buy(telecom7) + buy(telecom8) + buy(telecom9) + buy(telecom10) + 
buy(brewery1) + buy(brewery2) + buy(brewery3) + buy(brewery4) + buy(brewery5) + 
buy(brewery6) + buy(brewery7) + buy(brewery8) + buy(brewery9) + buy(brewery10) + 
buy(highways1) + buy(highways2) + buy(highways3) + buy(highways4) + buy(highways5) + 
buy(highways6) + buy(highways7) + buy(highways8) + buy(highways9) + buy(highways10) + 
buy(cars1) + buy(cars2) + buy(cars3) + buy(cars4) + buy(cars5) + buy(cars6) + 
buy(cars7) + buy(cars8) + buy(cars9) + buy(cars10) + buy(bank1) + buy(bank2) + 
buy(bank3) + buy(bank4) + buy(bank5) + buy(bank6) + buy(bank7) + buy(bank8) + 
buy(bank9) + buy(bank10) + buy(software1) + buy(software2) + buy(software3) + 
buy(software4) + buy(software5) + buy(software6) + buy(software7) + buy(software8) + 
buy(software9) + buy(software10) + buy(electronics1) + buy(electronics2) + buy(electronics3) + 
buy(electronics4) + buy(electronics5) + buy(electronics6) + buy(electronics7) + 
buy(electronics8) + buy(electronics9) + buy(electronics10) <= 7
_R202: frac(treasury1) + frac(treasury2) + frac(treasury3) + frac(treasury4) + 
frac(treasury5) + frac(treasury6) + frac(treasury7) + frac(treasury8) + frac(treasury9) + 
frac(treasury10) + frac(hardware1) + frac(hardware2) + frac(hardware3) + frac(hardware4) + 
frac(hardware5) + frac(hardware6) + frac(hardware7) + frac(hardware8) + frac(hardware9) + 
frac(hardware10) + frac(theater1) + frac(theater2) + frac(theater3) + frac(theater4) + 
frac(theater5) + frac(theater6) + frac(theater7) + frac(theater8) + frac(theater9) + 
frac(theater10) + frac(telecom1) + frac(telecom2) + frac(telecom3) + frac(telecom4) + 
frac(telecom5) + frac(telecom6) + frac(telecom7) + frac(telecom8) + frac(telecom9) + 
frac(telecom10) + frac(brewery1) + frac(brewery2) + frac(brewery3) + frac(brewery4) + 
frac(brewery5) + frac(brewery6) + frac(brewery7) + frac(brewery8) + frac(brewery9) + 
frac(brewery10) + frac(highways1) + frac(highways2) + frac(highways3) + frac(highways4) + 
frac(highways5) + frac(highways6) + frac(highways7) + frac(highways8) + frac(highways9) + 
frac(highways10) + frac(cars1) + frac(cars2) + frac(cars3) + frac(cars4) + frac(cars5) + 
frac(cars6) + frac(cars7) + frac(cars8) + frac(cars9) + frac(cars10) + frac(bank1) + 
frac(bank2) + frac(bank3) + frac(bank4) + frac(bank5) + frac(bank6) + frac(bank7) + 
frac(bank8) + frac(bank9) + frac(bank10) + frac(software1) + frac(software2) + 
frac(software3) + frac(software4) + frac(software5) + frac(software6) + frac(software7) + 
frac(software8) + frac(software9) + frac(software10) + frac(electronics1) + 
frac(electronics2) + frac(electronics3) + frac(electronics4) + frac(electronics5) + 
frac(electronics6) + frac(electronics7) + frac(electronics8) + frac(electronics9) + 
frac(electronics10) = 1
_R203: frac(bank1) + frac(bank2) + frac(bank3) + frac(bank4) + frac(bank5) + 
frac(bank6) + frac(bank7) + frac(bank8) + frac(bank9) + frac(bank10) <= 0.25
_R204: frac(cars1) + frac(cars2) + frac(cars3) + frac(cars4) + frac(cars5) + 
frac(cars6) + frac(cars7) + frac(cars8) + frac(cars9) + frac(cars10) <= 0.25
_R205: frac(highways1) + frac(highways2) + frac(highways3) + frac(highways4) + 
frac(highways5) + frac(highways6) + frac(highways7) + frac(highways8) + frac(highways9) + 
frac(highways10) <= 0.25
_R206: frac(brewery1) + frac(brewery2) + frac(brewery3) + frac(brewery4) + frac(brewery5) + 
frac(brewery6) + frac(brewery7) + frac(brewery8) + frac(brewery9) + frac(brewery10) <= 0.25
_R207: frac(telecom1) + frac(telecom2) + frac(telecom3) + frac(telecom4) + frac(telecom5) + 
frac(telecom6) + frac(telecom7) + frac(telecom8) + frac(telecom9) + frac(telecom10) <= 0.25
_R208: frac(theater1) + frac(theater2) + frac(theater3) + frac(theater4) + frac(theater5) + 
frac(theater6) + frac(theater7) + frac(theater8) + frac(theater9) + frac(theater10) <= 0.25
_R209: frac(hardware1) + frac(hardware2) + frac(hardware3) + frac(hardware4) + 
frac(hardware5) + frac(hardware6) + frac(hardware7) + frac(hardware8) + frac(hardware9) + 
frac(hardware10) + frac(software1) + frac(software2) + frac(software3) + frac(software4) + 
frac(software5) + frac(software6) + frac(software7) + frac(software8) + frac(software9) + 
frac(software10) + frac(electronics1) + frac(electronics2) + frac(electronics3) + 
frac(electronics4) + frac(electronics5) + frac(electronics6) + frac(electronics7) + 
frac(electronics8) + frac(electronics9) + frac(electronics10) <= 0.25
_R210: frac(treasury1) + frac(treasury2) + frac(treasury3) + frac(treasury4) + 
frac(treasury5) + frac(treasury6) + frac(treasury7) + frac(treasury8) + frac(treasury9) + 
frac(treasury10) <= 0.25
_R211: frac(treasury4) + frac(treasury5) + frac(treasury9) + frac(hardware1) + 
frac(hardware2) + frac(hardware5) + frac(hardware8) + frac(hardware10) + frac(theater2) + 
frac(theater4) + frac(telecom1) + frac(telecom5) + frac(telecom6) + frac(telecom7) + 
frac(brewery9) + frac(brewery10) + frac(highways8) + frac(cars1) + frac(cars2) + 
frac(software7) + frac(software8) + frac(software10) + frac(electronics1) + 
frac(electronics4) + frac(electronics7) <= 0.5
_R212: frac(treasury4) + frac(treasury5) + frac(treasury9) + frac(hardware1) + 
frac(hardware2) + frac(hardware5) + frac(hardware8) + frac(hardware10) + frac(theater2) + 
frac(theater4) + frac(telecom1) + frac(telecom5) + frac(telecom6) + frac(telecom7) + 
frac(brewery9) + frac(brewery10) + frac(highways8) + frac(cars1) + frac(cars2) + 
frac(software7) + frac(software8) + frac(software10) + frac(electronics1) + 
frac(electronics4) + frac(electronics7) >= 0.2
_R213: frac(treasury2) + frac(treasury6) + frac(treasury7) + frac(treasury8) + 
frac(hardware4) + frac(hardware6) + frac(theater1) + frac(theater3) + frac(theater7) + 
frac(theater9) + frac(telecom2) + frac(telecom4) + frac(telecom8) + frac(highways3) + 
frac(highways4) + frac(cars5) + frac(cars6) + frac(bank2) + frac(bank4) + frac(bank8) + 
frac(bank9) + frac(software1) + frac(software2) + frac(software9) + frac(electronics3) + 
frac(electronics5) + frac(electronics6) + frac(electronics8) + frac(electronics9) + 
frac(electronics10) <= 0.5
_R214: frac(treasury2) + frac(treasury6) + frac(treasury7) + frac(treasury8) + 
frac(hardware4) + frac(hardware6) + frac(theater1) + frac(theater3) + frac(theater7) + 
frac(theater9) + frac(telecom2) + frac(telecom4) + frac(telecom8) + frac(highways3) + 
frac(highways4) + frac(cars5) + frac(cars6) + frac(bank2) + frac(bank4) + frac(bank8) + 
frac(bank9) + frac(software1) + frac(software2) + frac(software9) + frac(electronics3) + 
frac(electronics5) + frac(electronics6) + frac(electronics8) + frac(electronics9) + 
frac(electronics10) >= 0.2
_R215: frac(treasury1) + frac(treasury10) + frac(hardware7) + frac(hardware9) + 
frac(theater8) + frac(theater10) + frac(telecom3) + frac(telecom9) + frac(telecom10) + 
frac(brewery1) + frac(brewery4) + frac(brewery6) + frac(brewery7) + frac(brewery8) + 
frac(highways2) + frac(highways5) + frac(highways7) + frac(highways10) + frac(cars4) + 
frac(cars7) + frac(cars8) + frac(cars9) + frac(cars10) + frac(bank1) + frac(bank3) + 
frac(bank5) + frac(bank6) + frac(bank7) + frac(bank10) + frac(software3) + frac(software4) + 
frac(software5) + frac(software6) + frac(electronics2) <= 0.5
_R216: frac(treasury1) + frac(treasury10) + frac(hardware7) + frac(hardware9) + 
frac(theater8) + frac(theater10) + frac(telecom3) + frac(telecom9) + frac(telecom10) + 
frac(brewery1) + frac(brewery4) + frac(brewery6) + frac(brewery7) + frac(brewery8) + 
frac(highways2) + frac(highways5) + frac(highways7) + frac(highways10) + frac(cars4) + 
frac(cars7) + frac(cars8) + frac(cars9) + frac(cars10) + frac(bank1) + frac(bank3) + 
frac(bank5) + frac(bank6) + frac(bank7) + frac(bank10) + frac(software3) + frac(software4) + 
frac(software5) + frac(software6) + frac(electronics2) >= 0.2
_R217: frac(hardware1) + frac(hardware2) + frac(hardware3) + frac(hardware4) + 
frac(hardware5) + frac(hardware6) + frac(hardware7) + frac(hardware8) + frac(hardware9) + 
frac(hardware10) + frac(theater1) + frac(theater2) + frac(theater3) + frac(theater4) + 
frac(theater5) + frac(theater6) + frac(theater7) + frac(theater8) + frac(theater9) + 
frac(theater10) + frac(telecom1) + frac(telecom2) + frac(telecom3) + frac(telecom4) + 
frac(telecom5) + frac(telecom6) + frac(telecom7) + frac(telecom8) + frac(telecom9) + 
frac(telecom10) + frac(software1) + frac(software2) + frac(software3) + frac(software4) + 
frac(software5) + frac(software6) + frac(software7) + frac(software8) + frac(software9) + 
frac(software10) + frac(electronics1) + frac(electronics2) + frac(electronics3) + 
frac(electronics4) + frac(electronics5) + frac(electronics6) + frac(electronics7) + 
frac(electronics8) + frac(electronics9) + frac(electronics10) <= 0.333333

Bounds
frac(treasury1) <= 0.2
frac(treasury2) <= 0.2
frac(treasury3) <= 0.2
frac(treasury4) <= 0.2
frac(treasury5) <= 0.2
frac(treasury6) <= 0.2
frac(treasury7) <= 0.2
frac(treasury8) <= 0.2
frac(treasury9) <= 0.2
frac(treasury10) <= 0.2
frac(hardware1) <= 0.2
frac(hardware2) <= 0.2
frac(hardware3) <= 0.2
frac(hardware4) <= 0.2
frac(hardware5) <= 0.2
frac(hardware6) <= 0.2
frac(hardware7) <= 0.2
frac(hardware8) <= 0.2
frac(hardware9) <= 0.2
frac(hardware10) <= 0.2
frac(theater1) <= 0.2
frac(theater2) <= 0.2
frac(theater3) <= 0.2
frac(theater4) <= 0.2
frac(theater5) <= 0.2
frac(theater6) <= 0.2
frac(theater7) <= 0.2
frac(theater8) <= 0.2
frac(theater9) <= 0.2
frac(theater10) <= 0.2
frac(telecom1) <= 0.2
frac(telecom2) <= 0.2
frac(telecom3) <= 0.2
frac(telecom4) <= 0.2
frac(telecom5) <= 0.2
frac(telecom6) <= 0.2
frac(telecom7) <= 0.2
frac(telecom8) <= 0.2
frac(telecom9) <= 0.2
frac(telecom10) <= 0.2
frac(brewery1) <= 0.2
frac(brewery2) <= 0.2
frac(brewery3) <= 0.2
frac(brewery4) <= 0.2
frac(brewery5) <= 0.2
frac(brewery6) <= 0.2
frac(brewery7) <= 0.2
frac(brewery8) <= 0.2
frac(brewery9) <= 0.2
frac(brewery10) <= 0.2
frac(highways1) <= 0.2
frac(highways2) <= 0.2
frac(highways3) <= 0.2
frac(highways4) <= 0.2
frac(highways5) <= 0.2
frac(highways6) <= 0.2
frac(highways7) <= 0.2
frac(highways8) <= 0.2
frac(highways9) <= 0.2
frac(highways10) <= 0.2
frac(cars1) <= 0.2
frac(cars2) <= 0.2
frac(cars3) <= 0.2
frac(cars4) <= 0.2
frac(cars5) <= 0.2
frac(cars6) <= 0.2
frac(cars7) <= 0.2
frac(cars8) <= 0.2
frac(cars9) <= 0.2
frac(cars10) <= 0.2
frac(bank1) <= 0.2
frac(bank2) <= 0.2
frac(bank3) <= 0.2
frac(bank4) <= 0.2
frac(bank5) <= 0.2
frac(bank6) <= 0.2
frac(bank7) <= 0.2
frac(bank8) <= 0.2
frac(bank9) <= 0.2
frac(bank10) <= 0.2
frac(software1) <= 0.2
frac(software2) <= 0.2
frac(software3) <= 0.2
frac(software4) <= 0.2
frac(software5) <= 0.2
frac(software6) <= 0.2
frac(software7) <= 0.2
frac(software8) <= 0.2
frac(software9) <= 0.2
frac(software10) <= 0.2
frac(electronics1) <= 0.2
frac(electronics2) <= 0.2
frac(electronics3) <= 0.2
frac(electronics4) <= 0.2
frac(electronics5) <= 0.2
frac(electronics6) <= 0.2
frac(electronics7) <= 0.2
frac(electronics8) <= 0.2
frac(electronics9) <= 0.2
frac(electronics10) <= 0.2

Binaries
buy(treasury1) buy(treasury2) buy(treasury3) buy(treasury4) buy(treasury5) buy(treasury6) 
buy(treasury7) buy(treasury8) buy(treasury9) buy(treasury10) buy(hardware1) 
buy(hardware2) buy(hardware3) buy(hardware4) buy(hardware5) buy(hardware6) buy(hardware7) 
buy(hardware8) buy(hardware9) buy(hardware10) buy(theater1) buy(theater2) buy(theater3) 
buy(theater4) buy(theater5) buy(theater6) buy(theater7) buy(theater8) buy(theater9) 
buy(theater10) buy(telecom1) buy(telecom2) buy(telecom3) buy(telecom4) buy(telecom5) 
buy(telecom6) buy(telecom7) buy(telecom8) buy(telecom9) buy(telecom10) buy(brewery1) 
buy(brewery2) buy(brewery3) buy(brewery4) buy(brewery5) buy(brewery6) buy(brewery7) 
buy(brewery8) buy(brewery9) buy(brewery10) buy(highways1) buy(highways2) buy(highways3) 
buy(highways4) buy(highways5) buy(highways6) buy(highways7) buy(highways8) buy(highways9) 
buy(highways10) buy(cars1) buy(cars2) buy(cars3) buy(cars4) buy(cars5) buy(cars6) 
buy(cars7) buy(cars8) buy(cars9) buy(cars10) buy(bank1) buy(bank2) buy(bank3) 
buy(bank4) buy(bank5) buy(bank6) buy(bank7) buy(bank8) buy(bank9) buy(bank10) 
buy(software1) buy(software2) buy(software3) buy(software4) buy(software5) buy(software6) 
buy(software7) buy(software8) buy(software9) buy(software10) buy(electronics1) 
buy(electronics2) buy(electronics3) buy(electronics4) buy(electronics5) buy(electronics6) 
buy(electronics7) buy(electronics8) buy(electronics9) buy(electronics10) 

End
