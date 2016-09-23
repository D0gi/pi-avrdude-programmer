#!/bin/bash

# 'top' required
# You can use a configuration file for this.
# The easiest way is to open top, and configure it the way you want.
# In this case, that means using 1 to get the per-CPU listing.
# Then have it write a configuration file with Shift+W.
# After that, it will use that config file whenever that user runs top,
# and keep the same appearance, even in batch mode.

fill=2046



while true;
 do
	time=$(date +%H%M%S)
	for ((j=0, r=0x16; j<6; j++, r-=2));
	 do
		row1=$(printf '0x%02x\n' $r)
		r=$(($r-2))
		row2=$(printf '0x%02x\n' $r)
		n=${time:$j:1}

		#for ((i=0, pow=1, x=0; i<n; ++i)); do ((x+=((pow *= 2)))); done
		x=$(( $fill >> 11-$n ))
		
		l=$(printf '0x%02x\n' $(($x & 0x00ff)) )
		h=$(printf '0x%02x\n' $(($x >> 8)))

		echo i2cset -y 1 0x03 $row1 $l $h i
		echo i2cset -y 1 0x03 $row2 $l $h i
		echo
		i2cset -y 1 0x03 $row1 $l $h i
		i2cset -y 1 0x03 $row2 $l $h i
	 done
 done
