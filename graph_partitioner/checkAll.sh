#!/bin/bash

HDV_DIR=.
DATASET_DIR=$HDV_DIR/dataset
FILE_QUALITY=$HDV_DIR/quality.csv
FILE_PERFORMANCE=$HDV_DIR/performance.csv

LIST_REAL="thermomech_dM.mtx patents_main.mtx delaunay_n18.mtx com-Amazon.mtx mario002.mtx largebasis.mtx delaunay_n20.mtx com-Youtube.mtx thermal2.mtx in-2004.mtx"
NUM="16 64 256 1024"

rm $FILE_QUALITY
touch $FILE_QUALITY
rm $FILE_PERFORMANCE
touch $FILE_PERFORMANCE

for i in $NUM
do
	echo "Number of subgraphs =" $i >> $FILE_QUALITY
	echo ",,ICN Ratio,Edge Replication,ICN Imbalance,Max ICN,Average ICN,Node Imbalance,Max Node,Average Node" >> $FILE_QUALITY
	echo "Number of subgraphs =" $i >> $FILE_PERFORMANCE
	echo ",METIS,BPart,Fennel,NE,TopoX,HEP,FSM(Split),FSM,HDV,HDV+A,HDV+A+B" >> $FILE_PERFORMANCE
	for j in $LIST_REAL
	do
		if [ ! -e "$DATASET_DIR/$j.csr" ]; then
			$HDV_DIR/convert/convert_CSR $DATASET_DIR/$j
		fi
		if [ ! -e "$DATASET_DIR/$j.metis" ]; then
			$HDV_DIR/convert/convert_Metis $DATASET_DIR/$j
		fi
		$HDV_DIR/bin/debug $DATASET_DIR/$j $i
		echo "" >> $FILE_PERFORMANCE
	done
	echo "" >> $FILE_QUALITY
	echo "" >> $FILE_PERFORMANCE
done
