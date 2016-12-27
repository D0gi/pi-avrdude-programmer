#!/bin/bash

# 'top' required
# You can use a configuration file for this.
# The easiest way is to open top, and configure it the way you want.
# In this case, that means using 1 to get the per-CPU listing.
# Then have it write a configuration file with Shift+W.
# After that, it will use that config file whenever that user runs top,
# and keep the same appearance, even in batch mode.






m=10

for ((i=1, pow=1, x=1; i<=m; i++)); do ((x+=((pow *= 2)))); done

# echo $pow
# echo $x
# printf '0x%04x\n' $x
# printf '0x%04x\n' $(($x >> 8))

l=$(printf '0x%02x\n' $(($x & 0x00ff)) )
h=$(printf '0x%02x\n' $(($x >> 8)))

echo $h $l