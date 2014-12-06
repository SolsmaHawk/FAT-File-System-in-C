// John Solsma 2014


#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <math.h>

#define TRUE  1
#define FALSE 0

void format(uint16_t sector_size, uint16_t cluster_size, uint16_t disk_size);
int fileExists(char * filename);
int file_exists (char * fileName);
void readMBR();

typedef struct __attribute__ ((__packed__)) {
	uint16_t sector_size; // bytes ( >= 64 bytes)
	uint16_t cluster_size; // number of sectors (at least 1 sector/cluster) 
	uint16_t disk_size; // size of disk in clusters
	uint16_t fat_start;
	uint16_t fat_length; // number of clusters
	uint16_t data_start; 
	uint16_t data_length; // clusters
	char     disk_name[16];
} mbr_t;


static char diskName[32];
static int globalClusterSize;



void load_disk(char disk_file[32])
{
	if(file_exists(disk_file)==TRUE)
	{
		strcpy(diskName, disk_file);
		readMBR();
	}
	else
	{
		strcpy(diskName, disk_file);
		format(128, 8, 1000);
	}
}

void readMBR() // reads MBR and sets global variables
{
	printf("Disk already created. Loading disk: %s",diskName);
}

int indexTranslation(int index)
{
	return index*globalClusterSize;
}


void format(uint16_t sector_size, uint16_t cluster_size, uint16_t disk_size)
{ printf("We got this disk name: %s\n",diskName);
	
	uint16_t sector[1]    = {sector_size};       // size of a sector in bytes
	uint16_t cluster[1]   = {cluster_size};      // size of a cluster in sectors
		
	FILE *fp;
	fp=fopen(diskName, "wb");
	if(fp != NULL)
	{
		printf("Disk successfully initialized at: %s\n",diskName);
		
	}
	else
	{
		printf("There was an error creating the disk.");
		return;
	}

	// 1: Write clusters
	char* buf = calloc(cluster_size, sector_size);
	globalClusterSize=cluster_size*sector_size;  // set cluster size in bytes
	printf("Cluster size in bytes set at: %d\n",cluster_size*sector_size);
	for(int i=0;i<disk_size;i++)
	{
		fwrite(buf, sizeof(uint8_t), cluster_size*sector_size, fp); // write each cluster
		
	}
	printf("Finished writing: %d clusters\n",disk_size);
	
	// 2: Write MBR
	rewind(fp);                    // return to beginning cluster 0 - byte 1
	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));
	mbr->sector_size = sector_size;
	mbr->cluster_size = cluster_size;
	mbr->disk_size = disk_size;    // number of clusters
	mbr->fat_start = 1;            // cluster 1
	mbr->fat_length = (int)ceil((float)(disk_size*2)/(float)globalClusterSize); // number of clusters FAT occupies
	mbr->data_start = mbr->fat_length+1;                                        // Cluster after fat_length
	mbr->data_length = disk_size-mbr->data_start;
	strcpy(mbr->disk_name,"C\0");
	printf("Number of clusters required to store FAT: %d\n",mbr->fat_length);
	printf("Data starts at cluster: %d Total data length: %d clusters\n",mbr->data_start,mbr->data_length);
	fwrite(mbr, sizeof(mbr_t), 1, fp);
	
	// 3: Write FAT entries
	fseek(fp, indexTranslation(mbr->fat_start), SEEK_SET);
	__uint16_t unallocatedCluster[1] = {0xFFFF};
	__uint16_t allocatedCluster[1]   = {0xFFFE};
	for(int i=0;i<disk_size;i++)
	{
		fwrite(unallocatedCluster, sizeof(__uint16_t), 1, fp);
	}
	
	fseek(fp, indexTranslation(mbr->fat_start), SEEK_SET);
	for(int i=0;i<mbr->fat_length+1;i++)
	{
		fwrite(allocatedCluster, sizeof(__uint16_t), 1, fp); // write in FAT entries for cluster occupied by MBR + FAT
	}
	printf("Disk %s formatting complete\n",mbr->disk_name);
	
	fclose(fp);
}

/*
uint16_t sector_size; // bytes ( >= 64 bytes)
uint16_t cluster_size; // number of sectors (at least 1 sector/cluster) 
uint16_t disk_size; // size of disk in clusters
uint16_t fat_start;
uint16_t fat_length; // number of clusters
uint16_t data_start; 
uint16_t data_length; // clusters
char     disk_name[16];
*/



uint32_t dateInt() {
	time_t t = time(NULL);
	struct tm *tptr = localtime(&t);
	uint32_t time_stamp;
	time_stamp = ((tptr->tm_year-80)<<25) + ((tptr->tm_mon+1)<<21) + ((tptr->tm_mday)<<16) + (tptr->tm_hour<<11) + (tptr->tm_min<<5) + ((tptr->tm_sec)%60)/2;
	return time_stamp;
}




//// Helper Functions

int fileExists(char *filename){
    /* try to open file to read */
    FILE *file;
    if ((file = fopen(filename, "r"))){
        fclose(file);
        return 1;
    }
	else
	{
	return 0;	
	}
    
}

int file_exists (char * fileName)
{
   struct stat buf;
   int i = stat ( fileName, &buf );
     /* File found */
     if ( i == 0 )
     {
       return 1;
     }
	else
	{
	return 0;	
	}
     
       
}



int main(int argc, char *argv[]) {
	//FILE *fp;
	//printf("How is this possible?");
		//char * names = diskName;
		//fp=fopen("test.bin", "wb+");
	printf("MBR size: %lu\n",sizeof(mbr_t));

	strcpy(diskName, "test.bin");
	//diskName ="test.bin";
	printf("%s\n",diskName);
	//format(128, 8, 1000);
	format(128, 8, 1000);
	//diskName="Hello";
	

	
	
}