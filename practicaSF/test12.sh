clear
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

result=$(./mi_ls disco /)
trimmed_result=$(echo "$result" | sed 's/^[ \t]*//;s/[ \t]*$//')
echo /$trimmed_result/
echo ./verificacion disco /$(echo "$trimmed_result")/
## ejecutarlo desde aqui error??


