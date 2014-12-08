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
uint32_t date_format();
int fileExists(char * filename);
int file_exists (char * fileName);
int indexTranslation(int index);
void readMBR();
int allocateFAT();


/**
entry_type (1 byte) - indicates if this is a file/directory (0 - file, 1 - directory)
creation_time (2 bytes) - format described below
creation_date (2 bytes) - format described below
length of entry name (1 byte)  
entry name (16 bytes) - the file/directory name
size (4 bytes) - the size of the file in bytes. Should be zero for directories:
**/
typedef struct __attribute__ ((__packed__)) {
	uint8_t	entry_type;
	uint16_t	creation_time, creation_date;
	uint8_t	name_len;
	char		name[16];
	uint32_t	size;
	uint16_t  numChildren;
	uint32_t	entry_start;
} entry_t;


/**
pointer type (1 byte) - (0 = pointer to a file, 1 = pointer to a directory, 2 = pointer to another entry describing more children for this directory)
reserved (1 byte)
start_pointer (2 bytes) - points to the start of the entry describing the child 
**/
typedef struct __attribute__ ((__packed__)) {
	uint8_t	type;
	uint8_t reserved;
	uint16_t start;
} entry_ptr_t;


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
static int globalNumberOfClusters;

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
	globalNumberOfClusters = mbr->disk_size;
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


entry_t *fs_ls(int dh, int child_num)
{
	FILE *fp;
	fp=fopen(diskName, "rb+");
	fseek(fp, dh+sizeof(entry_t)+(sizeof(entry_ptr_t)*child_num-1)+1, SEEK_SET);
	
	entry_ptr_t *childDirPointer = (entry_ptr_t *)malloc(sizeof(entry_ptr_t));
	fread(childDirPointer, sizeof(entry_ptr_t), 1, fp);
	fseek(fp, childDirPointer->start, SEEK_SET);
	
	entry_t *childDirEntry = (entry_t *)malloc(sizeof(entry_t));
	fread(childDirEntry, sizeof(entry_t), 1, fp);
	
	//printf("Location of child <%s> %d: byte %d\n",childDirEntry->name,child_num,childDirPointer->start);
	return childDirEntry;
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
	globalNumberOfClusters=mbr->disk_size;
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
	for(int i=0;i<mbr->fat_length+2;i++) // +2 to account for MBR and root directory
	{
		fwrite(allocatedCluster, sizeof(__uint16_t), 1, fp); // write in FAT entries for cluster occupied by MBR + FAT + Root directory
	}
	
	// 3: Create root directory
	entry_t *root = (entry_t *)malloc(sizeof(entry_t));
	uint32_t time_stamp = date_format();
	root->entry_type = 1;
	root->creation_date = htons((time_stamp>>16) & 0xFFFF);
	root->creation_time = htons(time_stamp & 0xFFFF);
	root->name_len = 4;
	strcpy(root->name, "root");
	root->size = 0;     //directories are size -
	root->numChildren = 0;
	root->entry_start = globalStartOfData;
	printf("Disk %s formatting complete\n",mbr->disk_name);
	fseek(fp, globalStartOfData, SEEK_SET);
	fwrite(root, sizeof(entry_t), 1, fp);
	printf("root directory initialized starting at byte: %d\n",globalStartOfData);
	fclose(fp);
	
	free(root);
	free(mbr);
}



int fs_opendir(char *absolute_path)
{
	if(strncmp("/",absolute_path, 1)==0 && strlen(absolute_path)==1) // root directory
	{
		return globalStartOfData;
	}
	else
	{  
		FILE *fp;
		fp=fopen(diskName, "rb+");
		char dirPath[64];
		strcpy(dirPath, absolute_path);
		char *tok = strtok(dirPath, "/");
		int currentByteIndex=globalStartOfData;
		while (tok != NULL)
		{
		//printf("%s\n",tok);
		//read entry
		fseek(fp, currentByteIndex, SEEK_SET); // go to root directory
		int numChildren;
		entry_t *currentDir = (entry_t *)malloc(sizeof(entry_t));
		fread(currentDir, sizeof(entry_t), 1, fp);
		//check number of children
		numChildren=currentDir->numChildren;
		//search children for matching directory name
		//fseek(fp, globalStartOfData, SEEK_SET); // seek to first child pointer
		for(int i=1;i<numChildren+1;i++)
		{
			entry_t *new = fs_ls(currentByteIndex, i);
			//printf("%s\n",new->name);
			if(strcmp(new->name, tok)==0)
			{
				currentByteIndex=new->entry_start;
			}
		}
		//fseek to beginning of child entry
		
		
		tok = strtok(NULL, "/");
		}
		printf("\nByte location of %s at: %d\n",absolute_path,currentByteIndex);
		return currentByteIndex;
	}
	return 0;
}

void fs_mkdir(int dh, char *child_name)
{
	FILE *fp;
	fp=fopen(diskName, "rb+");
	fseek(fp, dh, SEEK_SET);
	entry_t *rootDir = (entry_t *)malloc(sizeof(entry_t));
	
	fread(rootDir, sizeof(entry_t), 1, fp);
	//printf("%s\n",rootDir->name);
	rootDir->numChildren+=1; // increment entry children counter
	printf("Number of children in <%s> subdirectory: %d\n",rootDir->name,rootDir->numChildren);
	fseek(fp, dh, SEEK_SET); // return to beginning of directory
	fwrite(rootDir, sizeof(entry_t), 1, fp); // write back edited entry
	
	fseek(fp, dh+sizeof(entry_t)+(sizeof(entry_ptr_t)*rootDir->numChildren-1)+1, SEEK_SET);  // seek to byte after last pointer (if 0 children, byte after entry type)
	entry_ptr_t *newDirPointer = (entry_ptr_t *)malloc(sizeof(entry_ptr_t));
	int dirStart = allocateFAT();
	newDirPointer->type=1;
	newDirPointer->reserved=15;
	newDirPointer->start = indexTranslation(dirStart); // convert to byte location
	fwrite(newDirPointer, sizeof(entry_ptr_t), 1, fp);
	
	fseek(fp, indexTranslation(dirStart), SEEK_SET);
	entry_t *childDir = (entry_t *)malloc(sizeof(entry_t));
	uint32_t time_stamp = date_format();
	childDir->entry_type = 1;
	childDir->creation_date = htons((time_stamp>>16) & 0xFFFF);
	childDir->creation_time = htons(time_stamp & 0xFFFF);
	childDir->name_len = strlen(child_name);
	strcpy(childDir->name, child_name);
	childDir->size = 0;     //directories are size -
	childDir->numChildren = 0;
	childDir->entry_start = indexTranslation(dirStart);
	fwrite(childDir, sizeof(entry_t), 1, fp);
	printf("New directory <%s> successfully created at byte: %d\n\n",childDir->name,indexTranslation(dirStart));
	free(childDir);
	free(newDirPointer);
	fclose(fp);
	
}


/*
typedef struct __attribute__ ((__packed__)) {
	uint8_t	type;
	uint8_t reserved;
	uint16_t start;
} entry_ptr_t;

*/



//// Helper Functions

int allocateFAT()
{
	FILE *fp;
	fp=fopen(diskName, "rb+");
	int16_t result[globalNumberOfClusters];
	fseek(fp, globalStartOfFat, SEEK_SET);
	uint16_t allocatedCluster[1] = {0xFFFE};
	fread(result, sizeof(uint16_t), globalNumberOfClusters, fp);
	for(int i =0; i<globalNumberOfClusters;i++)
	{
		if((int)result[i]==-1)
		{
			fseek(fp, globalStartOfFat+i*2, SEEK_SET);
			fwrite(allocatedCluster, sizeof(uint16_t), 1, fp);
			printf("FAT block: %d allocated\n",i);
			fclose(fp);
			return(i);
		}
	}
	return -10;
	fclose(fp);
}


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

uint32_t date_format() {
	time_t t = time(NULL);
	struct tm *tptr = localtime(&t);
	uint32_t time_stamp;
	time_stamp = ((tptr->tm_year-80)<<25) + ((tptr->tm_mon+1)<<21) + ((tptr->tm_mday)<<16) + (tptr->tm_hour<<11) + (tptr->tm_min<<5) + ((tptr->tm_sec)%60)/2;
	return time_stamp;
}



int main(int argc, char *argv[]) {

	load_disk("test.bin");
	
	printf("\n");
	fs_mkdir(fs_opendir("/"), "null");
	fs_mkdir(fs_opendir("/"), "null2");
	fs_mkdir(fs_opendir("/null"), "cat");
	fs_mkdir(fs_opendir("/null/cat"), "hat");
	fs_mkdir(fs_opendir("/null/cat/hat"), "mat");
	
	/*
	fs_mkdir(fs_opendir("/null"), "new");
	fs_mkdir(fs_opendir("/null/new"), "dog");
	fs_mkdir(fs_opendir("/null/new/dog"), "cat");
	fs_mkdir(fs_opendir("/null/new/dog/cat"), "mouse");
	fs_mkdir(fs_opendir("/null/new/dog/cat/mouse"), "rat");
	
	fs_mkdir(fs_opendir("/"), "null2");
	fs_mkdir(fs_opendir("/"), "null3");
	
	for(int i = 0; i<100; i++)
	{
		int aInt = i;
		char str[15];
		sprintf(str, "dir%d", aInt);
		fs_mkdir(fs_opendir("/"), str);
	}
	*/
	
	//fs_mkdir(3072, "new");
	//fs_mkdir(3072, "dog");
	//fs_mkdir(3072, "new3");
	//fs_opendir("/null/new");
	//entry_t *new = fs_ls(3072, 1);
	//entry_t *new2 = fs_ls(3072, 2);
	//entry_t *new3 = fs_ls(3072, 3);
	//entry_t *new4 = fs_ls(3072, 4);
	//printf("%s\n",new->name);
	//printf("%d\n",fs_opendir("/"));
	//printf("%d\n",indexTranslation(allocateFAT()));
}
