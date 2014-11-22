// John Solsma 2014

#include <stdio.h>
#include <time.h>
#include <string.h>


struct  __attribute__ ((__packed__)) {
 	__int16_t a;
 	char b;
} packet_t;


int initDisk(FILE *fileToInit, char *diskName)
{
	fileToInit=fopen(diskName, "wb+");
	if(fileToInit != NULL)
	{
	printf("Disk successfully initialized at: %s",diskName);
	return 1;
	}
	else
		return 0;
}


int formatDisk(FILE *fileToFormat, char *diskName, __int16_t sectorSize, __int16_t clusterSize, __int16_t diskSize, __int16_t fatStart, __int16_t fatLength, __int16_t dataStart, __int16_t dataLength)
{
	FILE *fp = fileToFormat;


	__int16_t sector[1]    = {sectorSize};       // size of a sector in bytes
	__int16_t cluster[1]   = {clusterSize};      // size of a cluster in sectors
	__int16_t disk[1]      = {diskSize};         // size of disk in clusters
	__int16_t fatS[1]      = {fatStart};         // start of the FAT
	__int16_t fatL[1]      = {fatLength};        // length of the FAT
	__int16_t dataS[1]     = {dataStart};        // start of the data
	__int16_t dataL[1]     = {dataLength};       // length of the data



	    fp=fopen(diskName, "wb+");

	    if(fp != NULL)
	    	{
			fwrite(sector, sizeof(__int16_t), 1 /*20/2*/, fp);   //0
			fwrite(cluster, sizeof(__int16_t), 1 /*20/2*/, fp);  //1
			fwrite(disk, sizeof(__int16_t), 1 /*20/2*/, fp);     //2
			fwrite(fatS, sizeof(__int16_t), 1 /*20/2*/, fp);     //3
			fwrite(fatL, sizeof(__int16_t), 1 /*20/2*/, fp);     //4
			fwrite(dataS, sizeof(__int16_t), 1 /*20/2*/, fp);    //5
			fwrite(dataL, sizeof(__int16_t), 1 /*20/2*/, fp);    //6
			fwrite(diskName, sizeof(char[32]), 1 /*20/2*/, fp);  //7 - Disk Name
	    	// total MBR size = 46 bytes
			
			__int8_t buffer[1] = {0};
			
			fwrite(buffer, sizeof(__int8_t), 3 /*20/2*/, fp); // write in 3 buffer bytes to set MBR at 49 bytes
		
			__int16_t unallocatedCluster[1] = {0xFFFF};
			__int128_t unusedSector[1] = {0};
			
			for(int i = 0;i<diskSize;i++) // write FAT entry for each cluster
			{
				fwrite(unallocatedCluster, sizeof(__int16_t), 1 /*20/2*/, fp);
			}
			
			for(int i = 0;i<diskSize;i++) // write empty clusters to disk
			{
			fwrite(unusedSector, sizeof(__int128_t), clusterSize /*20/2*/, fp); // write (clusterSize) unused sectors per cluster
			}
			
			}
	   else
		{
	       	return 1;
		}
	   
	fclose(fp);
	return 0;
}


void readDisk(FILE *fileToRead, char *diskArea, char *diskName)
{	int i;
    __int16_t result[10];
	char result2[32];
	FILE *fp = fileToRead;
	if(strncmp("MBR",diskArea, 3)==0)
	{
		fp=fopen(diskName, "rb+");
			    if(fp != NULL)
				{
				rewind(fp);
				fread(result, sizeof(__int16_t), 7 /*20/2*/, fp); // read the first 6 slots - 12 bytes
				fread(result2, sizeof(char), 32 /*20/2*/, fp); // read file name - the next 32 bytes
				printf("\n");
				for (i = 0; i < 7; i++)
					{
						switch ( i ) {
						case 0:
						printf("Sector size in bytes          = %d\n", (int)result[i]);
						break;
						case 1:
						printf("Size of cluster in sectors    = %d\n", (int)result[i]);
						break;
						case 2:
						printf("Size of disk in clusters      = %d\n", (int)result[i]);
						break;
						case 3:
						printf("Start of FAT                  = byte %d - (MBR populates first 46 bytes (+3 buffer bytes) of disk) \n", (int)result[i]);
						break;
						case 4:
						printf("Length of FAT                 = %d bytes \n", (int)result[i]);
						break;
						case 5:
						printf("Start of data                 = byte %d - (byte after FAT) \n", (int)result[i]); 
						break;
						case 6:
						printf("Length of data                = %d\n", (int)result[i]);
						break;
						}
					}
					printf("%s",result2);
				fclose(fp);
			}
	}
}


int main(int argc, char *argv[]) {
	FILE *fileToInit;
	initDisk(fileToInit,"/Volumes/USB20FD/OSHW4/test.bin");
	time_t t = time(NULL);
	  struct tm *tptr = localtime(&t);
	//printf("%d",test<<5);
	formatDisk(fileToInit,"/Volumes/USB20FD/OSHW4/test.bin",128,8,10000,50,20000,20051,20);
	readDisk(fileToInit,"MBR","/Volumes/USB20FD/OSHW4/test.bin");

	
}