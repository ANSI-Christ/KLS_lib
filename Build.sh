clear
make -f ./makefile clean
clear
echo '>> compile <<'
echo ' '

make -f ./makefile
make -f ./makefile rmo > /dev/null

sleep 1

