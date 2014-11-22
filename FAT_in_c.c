#include <stdio.h>
#include <time.h>

int initDisk(char *diskName)
{
	return 0;
}

int formatDisk(char *diskName, __int16_t sectorSize, __int16_t clusterSize, __int16_t diskSize, __int16_t fatStart, __int16_t fatLength, __int16_t dataStart, __int16_t dataLength)
{
	FILE *fp = NULL;

	    __int16_t x[10] = {1,2,3,4,5,6,5000,6,-10,11};
	    __int16_t result[10];
	  char result2[32];
	    int i;
	__int16_t sector[1]   = {sectorSize};      // size of a sector in bytes
	__int16_t cluster[1]   = {clusterSize};     // size of a cluster in sectors
	__int16_t disk[1]       = {diskSize};     // size of disk in clusters
	__int16_t fatS[1]       = {fatStart};
	__int16_t fatL[1]       = {fatLength};
	__int16_t dataS[1]    = {dataStart};
	__int16_t dataL[1]    = {dataLength};
	char diskN[32];
	diskN[31]='\0';
	diskN[0]=*diskName;
	//char binary_string[ sizeof(__int16_t) ];

	
	
	
	//__int16_t sector=sectorSize;
	// __int16_t = 2 bytes -> can store sector size + cluster size
	// __int16_t = first 2 bytes -> store disk size
	// __int16_t = 2 bytes -> store fat start + length
	// __int16_t = 2 bytes -> data_start, data_length (2 bytes each)
	// __int16_t = 2 bytes x 8 -> store disk name
	__int16_t z[1000000];
	
	for(int i =0;i<1000000;i++)
	{
		z[i]=3;
	}

	    fp=fopen(diskName, "wb+");

	    if(fp != NULL)
	    {
		fwrite(sector, sizeof(__int16_t), 1 /*20/2*/, fp); //0
		fwrite(cluster, sizeof(__int16_t), 1 /*20/2*/, fp); //1
		fwrite(disk, sizeof(__int16_t), 1 /*20/2*/, fp); //2
		fwrite(fatS, sizeof(__int16_t), 1 /*20/2*/, fp);  //3
		fwrite(fatL, sizeof(__int16_t), 1 /*20/2*/, fp); //4
		fwrite(dataS, sizeof(__int16_t), 1 /*20/2*/, fp);  //5
		fwrite(dataL, sizeof(__int16_t), 1 /*20/2*/, fp); //6
		fwrite(diskName, sizeof(char[32]), 1 /*20/2*/, fp); //7 - Disk Name
		
	     // fwrite(x, sizeof(__int16_t), 10 /*20/2*/, fp); // write array to file in chunks the size of short (int16)
		
		 // fwrite(z, sizeof(__int16_t), 1000000 /*20/2*/, fp); // write array to file in chunks the size of short (int16)

		
	        rewind(fp);
	        fread(result, sizeof(__int16_t), 7 /*20/2*/, fp); // read the first 6 slots - 12 bytes
		  fread(result2, sizeof(char), 32 /*20/2*/, fp); // read file name - the next 32 bytes
	    }
	    else
	        return 1;

	    printf("Result\n");
	    for (i = 0; i < 7; i++)
	        printf("%d = %d\n", i, (int)result[i]);
	printf("%s",result2);
	    fclose(fp);
	return 0;
}

int main(int argc, char *argv[]) {
	__int16_t test = 3;
	time_t t = time(NULL);
	  struct tm *tptr = localtime(&t);
	printf("%d",test<<5);
	formatDisk("/Volumes/USB20FD/OSHW4/test.bin",3000,7,10000,10,10,20,20);
	

	
}