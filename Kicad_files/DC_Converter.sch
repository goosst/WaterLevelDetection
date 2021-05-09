EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Transistor_FET:IRLZ44N Q?
U 1 1 5F4A8BB1
P 4550 4000
AR Path="/5F4A8BB1" Ref="Q?"  Part="1" 
AR Path="/5F4A7200/5F4A8BB1" Ref="Q?"  Part="1" 
AR Path="/5F48B614/5F4A8BB1" Ref="Q_DCDC1"  Part="1" 
AR Path="/5F4A7E1B/5F4A8BB1" Ref="Q1"  Part="1" 
F 0 "Q1" H 4754 4046 50  0000 L CNN
F 1 "IRLZ44N" H 4754 3955 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-220-3_Vertical" H 4800 3925 50  0001 L CIN
F 3 "http://www.irf.com/product-info/datasheets/data/irlz44n.pdf" H 4550 4000 50  0001 L CNN
	1    4550 4000
	1    0    0    -1  
$EndComp
$Comp
L Device:L_Core_Ferrite L?
U 1 1 5F4A8BB7
P 4050 3650
AR Path="/5F4A8BB7" Ref="L?"  Part="1" 
AR Path="/5F4A7200/5F4A8BB7" Ref="L?"  Part="1" 
AR Path="/5F48B614/5F4A8BB7" Ref="L_DCDC1"  Part="1" 
AR Path="/5F4A7E1B/5F4A8BB7" Ref="L1"  Part="1" 
F 0 "L1" V 3869 3650 50  0000 C CNN
F 1 "L_Core_Ferrite" V 3960 3650 50  0001 C CNN
F 2 "Inductor_THT:L_Radial_D8.7mm_P5.00mm_Fastron_07HCP" H 4050 3650 50  0001 C CNN
F 3 "~" H 4050 3650 50  0001 C CNN
	1    4050 3650
	0    1    1    0   
$EndComp
$Comp
L Diode:1N5817 D?
U 1 1 5F4A8BBD
P 4650 3150
AR Path="/5F4A8BBD" Ref="D?"  Part="1" 
AR Path="/5F4A7200/5F4A8BBD" Ref="D?"  Part="1" 
AR Path="/5F48B614/5F4A8BBD" Ref="D_DCDC1"  Part="1" 
AR Path="/5F4A7E1B/5F4A8BBD" Ref="D1"  Part="1" 
F 0 "D1" V 4604 3229 50  0000 L CNN
F 1 "1N5817" V 4695 3229 50  0000 L CNN
F 2 "Diode_THT:D_DO-41_SOD81_P10.16mm_Horizontal" H 4650 2975 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88525/1n5817.pdf" H 4650 3150 50  0001 C CNN
	1    4650 3150
	0    1    1    0   
$EndComp
Wire Wire Line
	4650 3650 4650 3800
Wire Wire Line
	4200 3650 4650 3650
Wire Wire Line
	4650 3300 4650 3650
Connection ~ 4650 3650
Wire Wire Line
	3350 3650 3900 3650
$Comp
L Device:CP C?
U 1 1 5F4A8BEC
P 3950 2850
AR Path="/5F4A8BEC" Ref="C?"  Part="1" 
AR Path="/5F4A7200/5F4A8BEC" Ref="C?"  Part="1" 
AR Path="/5F48B614/5F4A8BEC" Ref="CDCDC1"  Part="1" 
AR Path="/5F4A7E1B/5F4A8BEC" Ref="C1"  Part="1" 
F 0 "C1" V 3695 2850 50  0000 C CNN
F 1 "CP" V 3786 2850 50  0001 C CNN
F 2 "Capacitor_THT:CP_Radial_D10.0mm_P5.00mm" H 3988 2700 50  0001 C CNN
F 3 "~" H 3950 2850 50  0001 C CNN
	1    3950 2850
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 2850 3350 2850
Wire Wire Line
	3350 2850 3350 3650
Connection ~ 3350 3650
Wire Wire Line
	4100 2850 4650 2850
Wire Wire Line
	4650 2850 4650 3000
Connection ~ 4650 2850
Text HLabel 4650 2450 2    50   Input ~ 0
Vout_plus
Wire Wire Line
	4650 2450 4650 2850
Text HLabel 3350 2600 2    50   Input ~ 0
Vout_min
Wire Wire Line
	3350 2600 3350 2850
Connection ~ 3350 2850
Text HLabel 4650 4950 2    50   Input ~ 0
Vin_min
Text HLabel 3350 4800 0    50   Input ~ 0
Vin_plus
Wire Wire Line
	4650 4200 4650 4950
Wire Wire Line
	3350 3650 3350 4500
Text HLabel 3700 4000 0    50   Input ~ 0
Gate
$Comp
L Device:R R_Reset?
U 1 1 5F6598BF
P 4000 4000
AR Path="/5F6598BF" Ref="R_Reset?"  Part="1" 
AR Path="/5F48B614/5F6598BF" Ref="R_gate1"  Part="1" 
AR Path="/5F4A7E1B/5F6598BF" Ref="R_gate1"  Part="1" 
F 0 "R_gate1" V 3793 4000 50  0000 C CNN
F 1 "200" V 3884 4000 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 3930 4000 50  0001 C CNN
F 3 "~" H 4000 4000 50  0001 C CNN
	1    4000 4000
	0    1    1    0   
$EndComp
Wire Wire Line
	3700 4000 3850 4000
Wire Wire Line
	4150 4000 4250 4000
$Comp
L Device:R R_gate?
U 1 1 5F65DA73
P 4250 4350
AR Path="/5F65DA73" Ref="R_gate?"  Part="1" 
AR Path="/5F48B614/5F65DA73" Ref="R_pulldown4"  Part="1" 
AR Path="/5F4A7E1B/5F65DA73" Ref="R_pulldown1"  Part="1" 
F 0 "R_pulldown1" H 4180 4304 50  0000 R CNN
F 1 "10k" H 4180 4395 50  0000 R CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 4180 4350 50  0001 C CNN
F 3 "~" H 4250 4350 50  0001 C CNN
	1    4250 4350
	-1   0    0    1   
$EndComp
Wire Wire Line
	4250 4500 3350 4500
Connection ~ 3350 4500
Wire Wire Line
	3350 4500 3350 4800
Wire Wire Line
	4250 4200 4250 4000
Connection ~ 4250 4000
Wire Wire Line
	4250 4000 4350 4000
$EndSCHEMATC
