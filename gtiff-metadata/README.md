# Test for metadata transfer between geotiffs

Test program `store_md.c` to reproduce the problems in the metadata transfer between Geo-Tiffs unsin GDAL.

## Installation 

You need gcc, make GDAL > 1.11 and exiftool. On linux (Debian) all `make`.


## Tested under operating system

Linux: 3.16.0-6-amd64 #1 SMP Debian 3.16.57-2 (2018-07-14) x86_64 GNU/Linux
Debian 8 - Jessie

## GDAL version and provenance

GDAL Version 2.0.2 version compiled from source 

## Usage:
```
Usage: store-md infile outfile N|Y N|Y
1. infile   the geotiff input file
2. outfile the copy of the file 
3. N|Y    parameter to remove the EXIT_ prefix from the tag N - don do it, Y - yes remov it
4. N|Y    parameter to choose the metatdata domain for storag
             N - use  GDALSetMetadataItem(h_dataset, key, value, NULL) 
             Y - use  GDALSetMetadataItem(h_dataset, key, value,"EXIF") 

1.  make a copy of it with the help of  `GDALCreateCopy(...)` 
2.  list the content of metadata domains of the source image
3.  try to copy the tags of the exif domain with 4 different settings 
4.  list the content of metadata domains of the source image
```
## Tests. 

There are is a test dataset `source.tif` you can use to reprocue the behaviour.

```bash
# Storage of EXIF metadata in the default domain
./store-md source.tif dest-nn.tif N N
gdalinfo -mdd EXIF dest-nn.tif
exiftools dest-nn.tif

# FAILED TEST of metadata storage in the EXIF domain
# without EXI_ prefix removement
./store-md source.tif dest-ny.tif N Y
gdalinfo -mdd EXIF dest-ny.tif
exiftools dest-ny.tif

# FAILED TEST of metadata storage in the EXIF domain
# with EXI_ prefix removement
./store-md source.tif dest-yy.tif Y Y
gdalinfo -mdd EXIF dest-nn.tif
exiftools dest-yy.tif
```





