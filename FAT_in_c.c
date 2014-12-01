// John Solsma 2014

#include <stdio.h>
#include <time.h>
#include <string.h>

#define START_OF_FAT 50
#define FILE_INFO_SIZE_WITH_COUNTER 44 // 40 bytes + 4 bytes to hold pointer counter
#define FILE_INFO_SIZE 40 // 40 bytes

/**
entry_type (1 byte) - indicates if this is a file/directory (0 - file, 1 - directory)
creation_time (2 bytes) - format described below
creation_date (2 bytes) - format described below
length of entry name (1 byte)  
entry name (32 bytes) - the file/directory name
size (4 bytes) - the size of the file in bytes. Should be zero for directories:
**/

typedef struct __attribute__ ((__packed__)) {
	__uint8_t		entry_type;
	__uint16_t	creation_time, creation_data;
	__uint8_t		name_len;
	char		name[32];
	__uint32_t	size;
} entry_t;


/**
pointer type (1 byte) - (0 = pointer to a file, 1 = pointer to a directory, 2 = pointer to another entry describing more children for this directory)
reserved (1 byte)
start_pointer (2 bytes) - points to the start of the entry describing the child 
**/

typedef struct __attribute__ ((__packed__)) { // size = 38 bytes
	__uint8_t	type;
	__uint8_t reserved;
	__uint32_t start;
	char		childName[32];
} entry_ptr_t;


/**
pointer counter = (4 bytes) - tracks the number of children in a directory 
**/
typedef struct __attribute__ ((__packed__)) {
	__uint32_t counter;
} entry_ptr_counter_t;



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

static __int16_t clusters;
//static __int16_t sectorSize;
//static __int16_t numberOfSectorsInCluster;
static __int16_t globalClusterSize;
static __int16_t startOfData;

int formatDisk(FILE *fileToFormat, char *diskName, __int16_t sectorSize, __int16_t clusterSize, __int16_t diskSize, __int16_t fatStart, __int16_t fatLength, __int16_t dataStart, __int16_t dataLength)
{
	FILE *fp = fileToFormat;
	startOfData = diskSize*2+START_OF_FAT+1;
	//printf("Here it is: %d",startOfData);
	clusters = diskSize; sectorSize = sectorSize; // set global # of clusters to calculate data start + other globals
	globalClusterSize = sectorSize*clusterSize;
	
	__int16_t sector[1]    = {sectorSize};       // size of a sector in bytes
	__int16_t cluster[1]   = {clusterSize};      // size of a cluster in sectors
	__int16_t disk[1]      = {diskSize};         // size of disk in clusters
	__int16_t fatS[1]      = {fatStart};         // start of the FAT
	__int16_t fatL[1]      = {fatLength};        // length of the FAT
	__int16_t dataS[1]     = {dataStart};        // start of the data
	__int16_t dataL[1]     = {dataLength};       // length of the data
	entry_ptr_counter_t pointerCounter;
	pointerCounter.counter = 0;

	    fp=fopen(diskName, "wb+");

	    if(fp != NULL)
	    	{
			fwrite(sector, sizeof(__int16_t), 1, fp);   // 0
			fwrite(cluster, sizeof(__int16_t), 1, fp);  // 1
			fwrite(disk, sizeof(__int16_t), 1, fp);     // 2
			fwrite(fatS, sizeof(__int16_t), 1, fp);     // 3
			fwrite(fatL, sizeof(__int16_t), 1, fp);     // 4
			fwrite(dataS, sizeof(__int16_t), 1, fp);    // 5
			fwrite(dataL, sizeof(__int16_t), 1, fp);    // 6
			fwrite(diskName, sizeof(char[32]), 1, fp);  // 7 - Disk Name
	    	// total MBR size = 46 bytes
			
			__int8_t buffer[1] = {0};
			
			fwrite(buffer, sizeof(__int8_t), 3, fp);                   // write in 3 buffer bytes to set MBR at 49 bytes
		
			__int16_t unallocatedCluster[1] = {0xFFFF};
			__int128_t unusedSector[1] = {0xFFFF};
			
			for(int i = 0;i<diskSize;i++)                              // write FAT entry for each cluster
			{
				fwrite(unallocatedCluster, sizeof(__int16_t), 1, fp);
			}
			
			for(int i = 0;i<diskSize;i++)                              // write empty clusters to disk
			{
			fwrite(unusedSector, sizeof(__int128_t), clusterSize, fp); // write (clusterSize) unused sectors per cluster
			}
			
			// initialize root directory
			
			fseek(fp, START_OF_FAT, SEEK_SET);                         // seek to first FAT entry
			__int16_t allocatedCluster[1] = {0};            
			fwrite(allocatedCluster, sizeof(__int16_t), 1, fp);        // write entry for root directory
						
			fseek(fp, 20051, SEEK_SET);                                // seek to the 20051 byte of the file - return to beginning of data
			
			__int8_t entry_type[1]     = {0};                          // indicates if file or directory - 0 - directory 1 - file  - 1 byte
			fwrite(entry_type, sizeof(__int8_t), 1, fp);
			
			__int16_t creation_time[1] = {7};                          // creation time - 2 bytes
			fwrite(creation_time, sizeof(__int16_t), 1, fp);
			
			__int16_t creation_date[1] = {8};                          // creation date - 2 bytes
			fwrite(creation_date, sizeof(__int16_t), 1, fp);
			
			__int8_t length_of_entry_name[1]  = {4};                   // length of entry name 1 bytes
			fwrite(length_of_entry_name, sizeof(__int8_t), 1, fp);
			
			char entryName[32] = {"root"};                             // file-directory name - 32 bytes
			fwrite(entryName, sizeof(char[32]), 1, fp);
			
			__int32_t file_size[1] = {0}; 	                            // file size - 2 bytes - should be zero for directories
			fwrite(file_size, sizeof(__int32_t), 1, fp);
			
			entry_ptr_counter_t newCounter[1] = {pointerCounter};      // pointer counter - 4 bytes - keeps track of the number of children directories / files
			fwrite(newCounter, sizeof(__int32_t), 1, fp);          
			
			//__int8_t pointer_type[1]  = {1};                           // pointer type - 0 = pointer to a file, 1 = pointer to a directory, 2 = pointer to another entry describing more children for this directory)
			//fwrite(pointer_type, sizeof(__int8_t), 1, fp);
			
		//	__int8_t reserved[1]  = {0};                               // reserved - 1 byte
			//fwrite(reserved, sizeof(__int8_t), 1, fp);
			
			//__int32_t start_pointer[1] = {0};                          // points to the start of the entry describing the child - 2 bytes
			//fwrite(start_pointer, sizeof(__int32_t), 1, fp);
			}
			
			
			
			
	   else
		{
	       	return 1;
		}
	   
	fclose(fp);
	return 0;
}





int fs_opendir3(char *diskname, char *absolute_path) // returns an integer that returns a handler to the directory pointed by absolute path ex: /root/new/
{
	FILE *fp;
	int directoryIndex;
	fp=fopen(diskname, "rb+");
	char *s;
	__int32_t pointerCounter[1];
	if(strncmp("/",absolute_path, 1)==0 && strlen(absolute_path)==1) // root directory
	{
		printf("\nAdding to: root directory. Directory index: %d",startOfData); // start of root directory
		return(startOfData);
	}
	else
	{
		__int32_t pointerCounter[1];      // stores number of children pointers in directory
		fseek(fp, startOfData+40, SEEK_SET);// 40 start of pointer - seek to parent directory to set pointer(s) -> need to increment # of pointers + add alll
		fread(pointerCounter, sizeof(__int32_t), 1, fp);
		//printf("%d",(int)pointerCounter[0]);
		fseek(fp, startOfData+FILE_INFO_SIZE_WITH_COUNTER, SEEK_SET);
		entry_ptr_t pointersFromRoot[1];
		//fread(pointersFromRoot, sizeof(entry_ptr_t), 1, fp);  // read first child directory
		//printf("%s",pointersFromRoot[0].childName);
		//	printf("%d",startOfData+FILE_INFO_SIZE_WITH_COUNTER);
		char str[32];
		strcpy(str, absolute_path);
		char *tok = strtok(str, "/");
		//printf("%s",str);
		while (tok != NULL) {                                     // loop through tokens
			if(*pointerCounter>1)
			{
				for(__int32_t i = 1; i <*pointerCounter; i++)             // read through pointers
				{
					fread(pointersFromRoot, sizeof(entry_ptr_t), 1, fp);  // read first child directory
					//printf("%s+%s",pointersFromRoot[0].childName,tok);
					if(strcmp(pointersFromRoot[0].childName, tok)==0)     // if pointer equals a token, the directory was found
					{
						//printf("YAAAAAAAAAAAAAAY");
						directoryIndex = pointersFromRoot[0].start;        // set directory index
						//printf("\nHere is the directory index:%d",directoryIndex);
						fseek(fp, directoryIndex+FILE_INFO_SIZE_WITH_COUNTER, SEEK_SET);  // jump to the next directory and continue pointer search
					}
				}
			
		}
		else // special case where only one directory exists
		{
		
			for(__int32_t i = 1; i <*pointerCounter+1; i++)             // read through pointers
			{
				fread(pointersFromRoot, sizeof(entry_ptr_t), 1, fp);  // read first child directory
				//printf("%s+%s",pointersFromRoot[0].childName,tok);
				if(strcmp(pointersFromRoot[0].childName, tok)==0)     // if pointer equals a token, the directory was found
				{
					//printf("YAAAAAAAAAAAAAAY");
					directoryIndex = pointersFromRoot[0].start;        // set directory index
					//printf("\nHere is the directory index:%d",directoryIndex);
					fseek(fp, directoryIndex+FILE_INFO_SIZE_WITH_COUNTER, SEEK_SET);  // jump to the next directory and continue pointer search
				}
			}
			
		}
		
	   // printf("%s\n", tok);
	    tok = strtok(NULL, "/");
	  }
	fclose(fp);
	printf("\nAdding to: %s directory. Directory index: %d",pointersFromRoot[0].childName,directoryIndex);
	return directoryIndex;
	}
	fclose(fp);
	return 0;
}



void fs_mkdir(char *diskname, int dh, char *child_name) // FAT indicies start from 0 - relative from 50
{
	__int16_t unallocatedCluster[1] = {0xFFFF};
	FILE *fp;
	fp=fopen(diskname, "rb+");
	// step 1: search FAT for unallocated cluster
	__int16_t result[clusters];
	fseek(fp, START_OF_FAT, SEEK_SET);
	fread(result, sizeof(__int16_t), clusters, fp);
	for (int i = 0; i < clusters; i++)
	{
		//printf("%d",result);
		if((int)result[i]==*unallocatedCluster)
		{
			printf("\nFound an empty cluster at index: %d",START_OF_FAT+i);
			entry_t directoryInfo;
			
			// step 2: set directory Info
			directoryInfo.entry_type= 1;                          // indicates this is a directory
			directoryInfo.name_len = strlen(child_name);
			strcpy(directoryInfo.name, child_name);               // set directory name
			directoryInfo.size = 0;
			//printf("%s",directoryInfo.name);
			
			// step 3: set parent directory to point to new child
			__int32_t pointerCounter[1];
			fseek(fp, dh+40, SEEK_SET);                   // 40 start of pointer - seek to parent directory to set pointer(s) -> need to increment # of pointers + add alll
			fread(pointerCounter, sizeof(__int32_t), 1, fp);      // read pointer counter
			if(pointerCounter[0]>=globalClusterSize/sizeof(entry_ptr_t))
			{
				printf("\nMaximum number of children (%d) reached for cluster size: %d\n",pointerCounter[0],globalClusterSize);
				printf("The directory <%s> was not created\n",child_name);
				break;
			}
			__int32_t updatedCounter[1] = {pointerCounter[0]+1};  // increment pointer counter
			fseek(fp, dh+40, SEEK_SET);                           // return to beginning of pointer counter
			fwrite(updatedCounter, sizeof(__int32_t), 1, fp);     // write modified counter back to disk
			fseek(fp, dh+FILE_INFO_SIZE_WITH_COUNTER+((pointerCounter[0])*sizeof(entry_ptr_t)), SEEK_SET); //*** seek to new child directory / file pointer location =  directoryHandler + 44 * (updatedCounter*4)
		//	printf("\n%lu",dh+FILE_INFO_SIZE_WITH_COUNTER+((pointerCounter[0])*sizeof(entry_ptr_t)));
			__int32_t childPointer[1] = {START_OF_FAT+i};
			//write entry pointer
			entry_ptr_t newPointer;
			strcpy(newPointer.childName, child_name);             // set pointer name
			newPointer.type = 1;
			newPointer.reserved = 0;
			int newCluster = i*globalClusterSize;                 // offset from beginning of data: cluster# * cluster size
			newPointer.start = dh+i+newCluster;
			entry_ptr_t pointerToWrite[1] = {newPointer};
			
			//printf("Here it is: %d",newPointer.start);
			fwrite(pointerToWrite, sizeof(entry_ptr_t), 1, fp);       // write pointer = START_OF_FAT+i
			
			// step 4: mark FAT as allocated - directory
			__int16_t allocatedCluster[1] = {0};
			fseek(fp, START_OF_FAT+i*2, SEEK_SET);                // return to FAT Entry  convert index to to 32 bit entry by multiplying by 2
			fwrite(allocatedCluster, sizeof(__int16_t), 1, fp);   // mark as used
			
			// step 5: write new directory to previously unallocated cluster
			
			fseek(fp, startOfData+(i*globalClusterSize), SEEK_SET);                 // return to dh + cluster offset
			printf("\nHere is the number: %d",i);
			entry_t dataToWrite[1] = {directoryInfo};
			fwrite(dataToWrite, sizeof(entry_t), 1, fp);       // write new directory to cluster

			printf("\nNew directory created: %s    Starting at byte: %d",child_name, startOfData+(i*globalClusterSize));
			printf("\nNumber of children in subdirectory: %d\n",pointerCounter[0]+1);
			break;
		}
	}
}

/*

typedef struct __attribute__ ((__packed__)) {
	__uint8_t	type;
	__uint8_t reserved;
	__uint16_t start;
} entry_ptr_t;


typedef struct __attribute__ ((__packed__)) {
	__uint8_t		entry_type;
	__uint16_t	creation_time, creation_data;
	__uint8_t		name_len;
	char		name[16];
	__uint32_t	size;
} entry_t;


*/


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
				fread(result, sizeof(__int16_t), 7, fp); // read the first 6 slots - 12 bytes
				fread(result2, sizeof(char), 32, fp); // read file name - the next 32 bytes
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
	formatDisk(fileToInit,"/Volumes/USB20FD/OSHW4/test.bin",128,8,10000,START_OF_FAT,20000,20051,20);
	readDisk(fileToInit,"MBR","/Volumes/USB20FD/OSHW4/test.bin");


	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "\nCurrent local time and date: %s", asctime (timeinfo) );
	
	time_t currentTime;
			currentTime = time(NULL);
			printf("%s",ctime(&currentTime));
			//char directory[12] = {"/"};
			
	//for(int i = 0; i<1; i++)
		
	//int test = fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/");
	fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/"), "folder");
	//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/"), "new");
	//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/"), "stuff");
	//}
	//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir("/Volumes/USB20FD/OSHW4/test.bin","/"), "new");
	//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir("/Volumes/USB20FD/OSHW4/test.bin","/"), "folder");
	//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir("/Volumes/USB20FD/OSHW4/test.bin","/"), "folder");
		//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir("/Volumes/USB20FD/OSHW4/test.bin","/"), "new");
		//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", fs_opendir("/Volumes/USB20FD/OSHW4/test.bin","/"), "folder");
		
	int index = fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/folder");
	fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", index, "new77");
	
	int index2 = fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/folder/new77");
	fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", index2, "new2");
	//fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", index, "new2");


	int index4 = fs_opendir3("/Volumes/USB20FD/OSHW4/test.bin","/folder/new77");
	fs_mkdir("/Volumes/USB20FD/OSHW4/test.bin", index4, "pictures");
}