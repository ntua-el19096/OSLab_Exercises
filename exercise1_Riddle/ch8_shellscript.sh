#!/bin/bash
for i in {0..9}
do
    truncate -s 1G bf0$i
    echo "xxxxxxxxxxxxxxxx" >> bf0$i
done
./riddle
