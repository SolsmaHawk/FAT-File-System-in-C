#include <stdio.h>
#include <time.h>



int formatDisk(char *diskName, __int16_t sectorSize, __int16_t clusterSize, __int16_t diskSize)
{
	FILE *fp = NULL;

	    __int16_t x[10] = {1,2,3,4,5,6,5000,6,-10,11};
	    __int16_t result[10];
	    int i;
	__int16_t sector[1]   = {sectorSize};      // size of a sector in bytes
	__int16_t cluster[1]   = {clusterSize};     // size of a cluster in sectors
	__int16_t disk[2]       = {diskSize,0};     // size of disk in clusters
	
	//__int16_t sector=sectorSize;
	// __int16_t = 4 bytes -> can store sector size + cluster size
	// __int16_t = first 2 bytes -> store disk size
	// __int16_t = 4 bytes -> store fat start + length
	// __int16_t = 4 bytes -> data_start, data_length (2 bytes each)
	// __int16_t = 4 bytes x 8 -> store disk name
	__int16_t z[1000000];
	
	for(int i =0;i<1000000;i++)
	{
		z[i]=3;
	}

	    fp=fopen(diskName, "w+");

	    if(fp != NULL)
	    {
		fwrite(sector, sizeof(__int16_t), 1 /*20/2*/, fp);
		fwrite(cluster, sizeof(__int16_t), 1 /*20/2*/, fp);
		fwrite(disk, sizeof(__int16_t), 2 /*20/2*/, fp); // write disk size then 0 to next byte
	        fwrite(x, sizeof(__int16_t), 10 /*20/2*/, fp); // write array to file in chunks the size of short (int16)
		 // fwrite(z, sizeof(__int16_t), 1000000 /*20/2*/, fp); // write array to file in chunks the size of short (int16)

		
	        rewind(fp);
	        fread(result, sizeof(__int16_t), 10 /*20/2*/, fp); // read from file 
	    }
	    else
	        return 1;

	    printf("Result\n");
	    for (i = 0; i < 10; i++)
	        printf("%d = %d\n", i, (int)result[i]);

	    fclose(fp);
	return 0;
}

int main(int argc, char *argv[]) {
	__int16_t test = 3;
	time_t t = time(NULL);
	  struct tm *tptr = localtime(&t);
	printf("%d",test<<5);
	formatDisk("/Volumes/USB20FD/OSHW4/test.bin",3000,7,10000);
	

	
}