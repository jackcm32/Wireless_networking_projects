#!/bin/bash

trials=2

# Change this to a new path if running on a different machine
ns3_path=Documents/UNI/2016/Wireless\ networking/ns-allinone-3.24.1/ns-3.24.1

# Move to the working directory
cd
cd "${ns3_path}"
./waf

# # Wall simulation 
index=0
for i in `seq 0 10`;
do
	for j in `seq 1 $trials`;
	do
		wall_count[$index]=$i
		output="$(./waf --run "scratch/Jmack_NS3proj 
			--x_min=-2.0 
			--x_max=32.0 
			--y_min=-4.0 
			--y_max=4.0 
			--z_min=0.0 
			--z_max=10.0 
			--nfloors=1.0 
			--x_rooms=$((i+1))
			--y_rooms=1.0
			--x_ap=32.0" )"
		# Grab out lost packets value and put it in the lost packets array
		lost_packets[$index]=${output#*Lost Packets: *}
		index=$((index+1))
    done
done 

cd
cd Wireless_networking_projects/NS3

python data_plotter.py 'wall' "${index}" "${wall_count[@]}" "${lost_packets[@]}"


# Move to the working directory
cd
cd "${ns3_path}"

# different size room simulations
index=0
for i in `seq 2 8`;
do
	for j in `seq 1 $trials`;
	do
		room_size[$index]=$i
		output="$(./waf --run "scratch/Jmack_NS3proj 
			--x_min=$(($i*(-1))) 
			--x_max=$((8+($i*1))) 
			--y_min=$(($i*(-1))) 
			--y_max=$(($i*1))
			--z_min=0.0 
			--z_max=10.0 
			--nfloors=1.0 
			--x_rooms=1.0
			--y_rooms=1.0
			--x_ap=10.0" )"
		# Grab out lost packets value and put it in the lost packets array
		lost_packets[$index]=${output#*Lost Packets: *}
		index=$((index+1))

    done
done 

cd
cd Wireless_networking_projects/NS3

python data_plotter.py 'room' "${index}" "${room_size[@]}" "${lost_packets[@]}"


