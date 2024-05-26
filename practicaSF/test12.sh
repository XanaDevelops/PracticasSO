make clean
make -j16
echo
echo CREAR DISCO
echo
./mi_mkfs disco 100000

echo
time ./simulacion disco
echo

./mi_ls -l disco /

