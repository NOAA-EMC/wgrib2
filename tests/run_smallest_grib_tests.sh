#!/bin/sh
# This script runs a series of tests of the smallest_grib program.
#
# Alyson Stahl 7/2026

set -e

echo ""
echo "*** Running smallest_grib tests"

ref_file=data/ref_smallest_grib.grb2
output=smallest_grib.grb2
file1=data/png_4bits.png
file2=data/large_png.grb2
file3=data/ref_c3_overflow.grib2

../aux_progs/smallest_grib2 && exit 1
if [ $? -ne 8 ]; then
    echo "Error: Test failed for invalid number of arguments."
    exit 1
fi

../aux_progs/smallest_grib2 /bad_directory/"$output" "$file1" "$file2" "$file3" && exit 1
if [ $? -ne 8 ]; then
    echo "Error: Test failed for bad output file."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" badfile.grb2 "$file2" "$file3" && exit 1
if [ $? -ne 8 ]; then
    echo "Error: Test failed for bad input file 1."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" badfile.grb2 "$file3" && exit 1
if [ $? -ne 8 ]; then
    echo "Error: Test failed for bad input file 2."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" "$file2" badfile.grb2 && exit 1
if [ $? -ne 8 ]; then
    echo "Error: Test failed for bad input file 3."
    exit 1
fi

# Message 1 Errors 

# First 4 bytes does not start with GRIB
../aux_progs/smallest_grib2 "$output" data/bad_sec0_1.grb2 "$file2" "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 1 in file 1."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" data/bad_sec0_2.grb2 "$file2" "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 2 in file 1."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" data/bad_sec0_3.grb2 "$file2" "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 3 in file 1."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" data/bad_sec0_4.grb2 "$file2" "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 4 in file 1."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" data/bad_sec0_size.grb2 "$file2" "$file3" && exit 1
if [ $? -ne 4 ]; then
    echo "Error: Test failed for read error in file 1."
    exit 1
fi

# Message 2 Errors 
# First 4 bytes does not start with GRIB
../aux_progs/smallest_grib2 "$output" "$file1" data/bad_sec0_1.grb2 "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 1 in file 2."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" data/bad_sec0_2.grb2 "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 2 in file 2."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" data/bad_sec0_3.grb2 "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 3 in file 2."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" data/bad_sec0_4.grb2 "$file3" && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 4 in file 2."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" data/bad_sec0_size.grb2 "$file3" && exit 1
if [ $? -ne 4 ]; then
    echo "Error: Test failed for read error in file 2."
    exit 1
fi

# Message 3 Errors 

# First 4 bytes does not start with GRIB
../aux_progs/smallest_grib2 "$output" "$file1" "$file2" data/bad_sec0_1.grb2 && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 1 in file 3."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" "$file2" data/bad_sec0_2.grb2 && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 2 in file 3."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" "$file2" data/bad_sec0_3.grb2 && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 3 in file 3."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" "$file2" data/bad_sec0_4.grb2 && exit 1
if [ $? -ne 1 ]; then
    echo "Error: Test failed for bad sec0 message at byte 4 in file 3."
    exit 1
fi

../aux_progs/smallest_grib2 "$output" "$file1" "$file2" data/bad_sec0_size.grb2 && exit 1
if [ $? -ne 4 ]; then
    echo "Error: Test failed for read error in file 3."
    exit 1
fi

# Smallest = 1
../aux_progs/smallest_grib2 "$output" "$file1" "$file2" "$file3"
ret=$(../src/wgrib2 "$output")
expected=$(../src/wgrib2 "$ref_file")
if [ "$ret" != "$expected" ]; then
    echo "Error: Smallest grib test failed where smallest is file 1."
    exit 1
fi

# Smallest = 2
../aux_progs/smallest_grib2 "$output" "$file2" "$file1" "$file3"
ret=$(../src/wgrib2 "$output")
expected=$(../src/wgrib2 "$ref_file")
if [ "$ret" != "$expected" ]; then
    echo "Error: Smallest grib test failed where smallest is file 2."
    exit 1
fi
# Smallest = 3

../aux_progs/smallest_grib2 "$output" "$file2" "$file3" "$file1"
ret=$(../src/wgrib2 "$output")
expected=$(../src/wgrib2 "$ref_file")
if [ "$ret" != "$expected" ]; then
    echo "Error: Smallest grib test failed where smallest is file 3."
    exit 1
fi

echo "*** SUCCESS!"
exit 0