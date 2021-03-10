#!/bin/bash
# This script will read the opcode matrix CSV file and create a cpp file that contains the matrix as a variable
# the file gets written as asm/opcode.cpp

split_line() {
	OIFS=$IFS
	IFS=","
	[[ "$1" != "" ]] && {
		for field in "$1"
		do
			echo "	// csv field $field"
			mnemonic=`echo $field | awk '{ split($0,a," "); print a[1] }'`
			addrmode=`echo $field | awk '{ split($0,a," "); print a[2] }'`

			echo "	{ mnemonic = \"$mnemonic\", addrmode=\"$addrmode\" }"
		done
	}
	IFS=$OIFS
}

MATRIX_FILE=opmatrix.csv
[[ ! -f "$MATRIX_FILE" ]] && { echo "matrix file $MATRIX_FILE could not be found"; exit 1; }

echo "#include \"opcode.h\"

Opcode opcodeMatrix[16][16] = {"

while IFS= read -r line
do
	split_line "$line"
done < "$MATRIX_FILE"
