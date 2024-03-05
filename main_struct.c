#include <stdio.h>
#include <stdlib.h>

typedef struct fat_BS
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
 
	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];
 
}__attribute__((packed)) fat_BS_t;

int main()
{

    FILE *fp;
    fat_BS_t  boot_record;
	int root_dir, in_root_dir;

    fp= fopen("fat16_4sectorpercluster.img", "rb");
    fseek(fp, 0, SEEK_SET);
    fread(&boot_record, sizeof(fat_BS_t),1, fp);

	//Parte dada pelo professor e que servira de base para o restante das buscas
    printf("Bytes per sector: %hd \n", boot_record.bytes_per_sector);
    printf("Sector per cluster: %x \n", boot_record.sectors_per_cluster);
	printf("Reserved sector: %hd \n", boot_record.reserved_sector_count);
	printf("Number of fats: %x \n", boot_record.table_count);
	printf("sector per fat: %hd \n", boot_record.table_size_16);
	printf("number entries on root dir: %hd \n", boot_record.root_entry_count);

	//==========================================================================
	root_dir = (((short int)boot_record.table_count * boot_record.table_size_16) + boot_record.reserved_sector_count) * boot_record.bytes_per_sector; 
	printf("localização do inicio do root dir: %d\n", root_dir);

	in_root_dir = root_dir;
	while(fseek(fp, in_root_dir, SEEK_SET) != 0){
		
	}
	




    return 0;
}
