transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/ShiftReg.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/defines.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/BRAM.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/NTTN.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/NTT2.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/ModRed_sub.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/ModRed.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/ModMult.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/intMult.v}
vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/AddressGenerator.v}

vlog -vlog01compat -work work +incdir+C:/users/crossover/Documents/NTT-Parametric {C:/users/crossover/Documents/NTT-Parametric/NTT_PolyMul_test_v2.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cycloneiv_hssi_ver -L cycloneiv_pcie_hip_ver -L cycloneiv_ver -L rtl_work -L work -voptargs="+acc"  NTT_PolyMul_test_v2

add wave *
view structure
view signals
run -all
