// John Solsma 2014


#include <stdio.h>
#include <stdlib.h>
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
int indexTranslation(int index);
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
static int globalStartOfFat;
static int globalStartOfData;


void load_disk(char *disk_file)
{
	if(fileExists(disk_file)==TRUE)
	{
		strcpy(diskName, disk_file);
		printf("Disk already created.  Loading disk: %s\n",diskName);
		readMBR();
	}
	else
	{
		strcpy(diskName, disk_file);
		printf("No disk file named: %s\n",diskName);
		format(128, 8, 1000);
	}
}


void readMBR() // reads MBR and sets global variables
{
	FILE *fp;
	fp=fopen(diskName, "rb+");
	if(fp != NULL)
	{
		printf("MBR loaded from file:  %s\n",diskName);
		
	}
	else
	{
		printf("There was an error reading from disk.\n");
		return;
	}
	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));
	//rewind(fp);
	fseek(fp, 0, SEEK_SET);
	fread(mbr, sizeof(mbr_t), 1, fp);
	
	globalClusterSize = mbr->sector_size*mbr->cluster_size;
	globalStartOfFat= indexTranslation(mbr->fat_start);
	globalStartOfData= indexTranslation(mbr->data_start);
	
	printf("Disk sector size:      %d  bytes\n",mbr->sector_size);
	printf("Disk cluster size:     %d  sectors, %d bytes\n",mbr->cluster_size,globalClusterSize);
	printf("Disk size:             %d  clusters\n",mbr->disk_size);
	printf("Start of FAT:  cluster %d\n",mbr->fat_start);
	printf("FAT length:            %d clusters, %d entries\n",mbr->fat_length,mbr->disk_size);
	printf("Start of data: cluster %d, byte: %d\n",mbr->data_start,indexTranslation(mbr->data_start));
	printf("Data length:           %d  clusters\n",mbr->data_length);
	printf("Disk name:             %s\n",mbr->disk_name);
	free(mbr);
}


void format(uint16_t sector_size, uint16_t cluster_size, uint16_t disk_size)
{ 
	FILE *fp;
	fp=fopen(diskName, "wb");
	if(fp != NULL)
	{
		printf("New disk successfully initialized at: %s\n",diskName);
		
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
	rewind(fp);                     // return to beginning cluster 0 - byte 0
	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));
	mbr->sector_size = sector_size;
	mbr->cluster_size = cluster_size;
	mbr->disk_size = disk_size;      // number of clusters
	mbr->fat_start = 1;            // cluster 1
	globalStartOfFat = indexTranslation(mbr->fat_start);
	mbr->fat_length = (int)ceil((float)(disk_size*2)/(float)globalClusterSize); // number of clusters FAT occupies
	mbr->data_start = mbr->fat_length+1;                                        // Cluster after fat_length
	globalStartOfData = indexTranslation(mbr->data_start);
	mbr->data_length = disk_size-mbr->data_start;
	strcpy(mbr->disk_name,"C\0");
	printf("Number of clusters required to store FAT: %d\n",mbr->fat_length);
	printf("Data starts at cluster: %d Total data length: %d clusters\n",mbr->data_start,mbr->data_length);
	fwrite(mbr, sizeof(mbr_t), 1, fp);
	
	// 3: Write FAT entries
	fseek(fp, indexTranslation(mbr->fat_start), SEEK_SET);
	uint16_t unallocatedCluster[1] = {0xFFFF};
	uint16_t allocatedCluster[1]   = {0xFFFE};
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
	free(mbr);
	fclose(fp);
}

int fs_opendir(char *absolute_path);
void fs_mkdir(int dh, char *child_name);
//entry_t *fs_ls(int dh, int child_num);


//// Helper Functions


int indexTranslation(int index)
{
	return index*globalClusterSize;
}

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

uint32_t dateInt() {
	time_t t = time(NULL);
	struct tm *tptr = localtime(&t);
	uint32_t time_stamp;
	time_stamp = ((tptr->tm_year-80)<<25) + ((tptr->tm_mon+1)<<21) + ((tptr->tm_mday)<<16) + (tptr->tm_hour<<11) + (tptr->tm_min<<5) + ((tptr->tm_sec)%60)/2;
	return time_stamp;
}



int main(int argc, char *argv[]) {

	load_disk("test.bin");

}