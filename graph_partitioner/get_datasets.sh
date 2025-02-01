#!/bin/bash

mkdir dataset

wget https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/delaunay_n18.tar.gz
tar -zxvf delaunay_n18.tar.gz
mv delaunay_n18/delaunay_n18.mtx ./dataset
rm -r delaunay_n18 delaunay_n18.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/delaunay_n20.tar.gz
tar -zxvf delaunay_n20.tar.gz
mv delaunay_n20/delaunay_n20.mtx ./dataset
rm -r delaunay_n20 delaunay_n20.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/QLi/largebasis.tar.gz
tar -zxvf largebasis.tar.gz
mv largebasis/largebasis.mtx ./dataset
rm -r largebasis largebasis.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/GHS_indef/mario002.tar.gz
tar -zxvf mario002.tar.gz
mv mario002/mario002.mtx ./dataset
rm -r mario002 mario002.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/Botonakis/thermomech_dM.tar.gz
tar -zxvf thermomech_dM.tar.gz
mv thermomech_dM/thermomech_dM.mtx ./dataset
rm -r thermomech_dM thermomech_dM.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/Pajek/patents_main.tar.gz
tar -zxvf patents_main.tar.gz
mv patents_main/patents_main.mtx ./dataset
rm -r patents_main patents_main.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/SNAP/com-Amazon.tar.gz
tar -zxvf com-Amazon.tar.gz
mv com-Amazon/com-Amazon.mtx ./dataset
rm -r com-Amazon com-Amazon.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/SNAP/com-Youtube.tar.gz
tar -zxvf com-Youtube.tar.gz
mv com-Youtube/com-Youtube.mtx ./dataset
rm -r com-Youtube com-Youtube.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/Schmid/thermal2.tar.gz
tar -zxvf thermal2.tar.gz
mv thermal2/thermal2.mtx ./dataset
rm -r thermal2 thermal2.tar.gz

wget https://suitesparse-collection-website.herokuapp.com/MM/LAW/in-2004.tar.gz
tar -zxvf in-2004.tar.gz
mv in-2004/in-2004.mtx ./dataset
rm -r in-2004 in-2004.tar.gz

