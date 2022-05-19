

rm -rf ./build

cmake . -B build -DCMAKE_TOOLCHAIN_FILE=ts7200.cmake &> /dev/null

cd ./build

for PATH in ../tests/integration/*.c; do 

FILE=${PATH##*/}
BASE=${FILE%.*}
TEST_NAME="${BASE}_test"

echo $TEST_NAME;

cmake --build . --target $TEST_NAME &> /dev/null

ts7200 $TEST_NAME &> ./tmp 

if grep -q "Return value: 1" ./tmp;then
  echo "Failed"
  else
  echo "Passed"
fi

rm ./tmp

done


