#!/bin/bash
# This script will read the opcode matrix CSV file and create a cpp file that contains the matrix as a variable
# the file gets written as asm/opcode.cpp. This script also generates a list of mnemonics from the opcode matrix
# this matrix generation could, in theory, be used with opcode matricies from different CPU architectures

declare -A MNEMONIC_LIST

split_line() {
	OIFS=$IFS
	IFS=","
	[[ "$1" != "" ]] && {
        printf "\t{" >> "$OUTPUT_FILE"
        COMMA=""
		for field in $1
		do
			mnemonic=$(echo "$field" | awk '{ split($0,a," "); print a[1] }')
			addrmode=$(echo "$field" | awk '{ split($0,a," "); print a[2] }')

			printf "%s{ .mnemonic = \"%s\", .addrmode=\"%s\" }" "$COMMA" "$mnemonic" "$addrmode" >> "$OUTPUT_FILE"
			MNEMONIC_LIST["$mnemonic"]=1
            COMMA=","
		done
        printf "},\n" >> "$OUTPUT_FILE"
	}
	IFS=$OIFS
}

OUTPUT_FILE=asm/opcode.cpp
MATRIX_FILE=opmatrix.csv

[[ -f "$OUTPUT_FILE" ]] && { rm -f "$OUTPUT_FILE"; }

[[ ! -f "$MATRIX_FILE" ]] && { echo "matrix file $MATRIX_FILE could not be found"; exit 1; }

printf "#include \"opcode.h\"\n\nextern const Opcode opcodeMatrix[16][16] = {" >> "$OUTPUT_FILE"

while IFS= read -r line
do
	split_line "$line"
done < "$MATRIX_FILE"

printf "};\n\n" >> "$OUTPUT_FILE"

printf "extern const std::string mnemonics[] = {" >> "$OUTPUT_FILE"

COMMA=""
mapfile -d '' sorted_mnemonics < <(printf '%s\0' "${!MNEMONIC_LIST[@]}" | sort -z)
for key in "${sorted_mnemonics[@]}"; do
	printf "%s\"%s\"" "$COMMA" "$key" >> "$OUTPUT_FILE"
	COMMA=","
done

# write enpty string to the last mnemonic entry so we know where to end the array
printf ",\"\"};" >> "$OUTPUT_FILE"