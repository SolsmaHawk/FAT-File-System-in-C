// John Solsma 2014

#include <stdio.h>
#include <time.h>
#include <string.h>

#define START_OF_FAT 50


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
			fwrite(sector, sizeof(__int16_t), 1, fp);   //0
			fwrite(cluster, sizeof(__int16_t), 1, fp);  //1
			fwrite(disk, sizeof(__int16_t), 1, fp);     //2
			fwrite(fatS, sizeof(__int16_t), 1, fp);     //3
			fwrite(fatL, sizeof(__int16_t), 1, fp);     //4
			fwrite(dataS, sizeof(__int16_t), 1, fp);    //5
			fwrite(dataL, sizeof(__int16_t), 1, fp);    //6
			fwrite(diskName, sizeof(char[32]), 1, fp);  //7 - Disk Name
	    	// total MBR size = 46 bytes
			
			__int8_t buffer[1] = {0};
			
			fwrite(buffer, sizeof(__int8_t), 3, fp); // write in 3 buffer bytes to set MBR at 49 bytes
		
			__int16_t unallocatedCluster[1] = {0xFFFF};
			__int128_t unusedSector[1] = {0xFFFF};
			
			for(int i = 0;i<diskSize;i++) // write FAT entry for each cluster
			{
				fwrite(unallocatedCluster, sizeof(__int16_t), 1, fp);
			}
			
			for(int i = 0;i<diskSize;i++) // write empty clusters to disk
			{
			fwrite(unusedSector, sizeof(__int128_t), clusterSize, fp); // write (clusterSize) unused sectors per cluster
			}
			
			// initialize root directory
			
			fseek(fp, START_OF_FAT, SEEK_SET);                    // seek to first FAT entry
			__int16_t allocatedCluster[1] = {0};            
			fwrite(allocatedCluster, sizeof(__int16_t), 1, fp);   // write entry for root directory
						
			fseek(fp, 20051, SEEK_SET);                    // seek to the 20051 byte of the file - return to beginning of data
			
			__int8_t entry_type[1]     = {0};              // indicates if file or directory - 0 - directory 1 - file  - 1 byte
			fwrite(entry_type, sizeof(__int8_t), 1, fp);
			
			__int16_t creation_time[1] = {7};              // creation time - 2 bytes
			fwrite(creation_time, sizeof(__int16_t), 1, fp);
			
			__int16_t creation_date[1] = {8};              // creation date - 2 bytes
			fwrite(creation_date, sizeof(__int16_t), 1, fp);
			
			__int8_t length_of_entry_name[1]  = {4};       // length of entry name 2 bytes
			fwrite(length_of_entry_name, sizeof(__int8_t), 1, fp);
			
			char entryName[16] = {"root"};                 // file-directory name - 16 bytes
			fwrite(entryName, sizeof(char[16]), 1, fp);
			
			__int32_t file_size[1] = {0}; 	                // file size - 2 bytes - should be zero for directories
			fwrite(file_size, sizeof(__int32_t), 1, fp);
			
			__int8_t pointer_type[1]  = {1};              // pointer type - 0 = pointer to a file, 1 = pointer to a directory, 2 = pointer to another entry describing more children for this directory)
			fwrite(pointer_type, sizeof(__int8_t), 1, fp);
			
			__int8_t reserved[1]  = {0};                  // reserved - 1 byte
			fwrite(reserved, sizeof(__int8_t), 1, fp);
			
			__int32_t start_pointer[1] = {0};             // - points to the start of the entry describing the child - 2 bytes
			fwrite(start_pointer, sizeof(__int32_t), 1, fp);
			}
			
			
			
			
	   else
		{
	       	return 1;
		}
	   
	fclose(fp);
	return 0;
}




int fs_opendir(char *diskname, char *absolute_path) // returns an integer that returns a handler to the directory pointed by absolute path ex: /root/new/
{
	FILE *fp;
	fp=fopen(diskname, "wb+");
	char* s;
	s = strtok(absolute_path, "/");
	//printf("%s", s);
	
	/* walk through other tokens */
	   while( s != NULL ) 
	   {
	      printf( "%s\n", s );
	    
	      s = strtok(NULL, "/");
	   }
	fclose(fp);
	return 0;
}


void fs_mkdir(int dh, char *child_name)
{
	
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
	printf ( "\n Current local time and date: %s", asctime (timeinfo) );
	
	time_t currentTime;
			currentTime = time(NULL);
			printf("%s",ctime(&currentTime));
			char directory[12] = {"/root/hello"};
			fs_opendir("/Volumes/USB20FD/OSHW4/test.bin",directory);
	
}