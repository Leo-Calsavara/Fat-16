#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stdbool.h>
#include <vector>

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

typedef struct root_dir
{
	unsigned char file_name[11];
	unsigned char atribute;
	unsigned char reserved_nt;
	unsigned char creation_time_tenths;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_access;
	unsigned short high_first_cluster;
	unsigned short last_modified_time;
	unsigned short last_modified_date;
	unsigned short low_first_cluster;
	unsigned int size_file;

	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	//unsigned char		extended_section[54];//Questionar professor
 
}__attribute__((packed)) root_dir_t;

void ReadRootDir(int pos_rootdir, root_dir_t root_dir, FILE *fp, std::vector<short> &file_first_cluster, std::vector<int> &tamanho){
	bool is_zero = false;
     
	
	fseek(fp, pos_rootdir, SEEK_SET);

	while(!is_zero){
		fread(&root_dir, sizeof(root_dir_t),1, fp);
		if(root_dir.file_name[0] == 0){
			is_zero = true;
		}else if(root_dir.file_name[11] == 15 || root_dir.file_name[0] == 229){
			pos_rootdir += 32;
			fseek(fp, pos_rootdir, SEEK_SET);
		} else{
			printf("Nome: ");
			for(int i=0; i<8; i++){
				printf("%c", root_dir.file_name[i]);
			}
			printf("\n");
			printf("Extensão: ");
			for(int i=8; i<11; i++){
				printf("%c", root_dir.file_name[i]);
			}
			printf("\n");
			printf("Tipo: %x\n", root_dir.atribute);
			printf("First Cluster: %hd\n", root_dir.low_first_cluster);
			printf("Tamanho: %d\n", root_dir.size_file);

            file_first_cluster.push_back(root_dir.low_first_cluster);
			tamanho.push_back(root_dir.size_file);
            pos_rootdir += 32;
			fseek(fp, pos_rootdir, SEEK_SET);
			printf("\n");
		}
	}
	
	return;

}

void ReadBootRecord(fat_BS_t boot_record, FILE *fp){
	fseek(fp, 0, SEEK_SET);
    fread(&boot_record, sizeof(fat_BS_t),1, fp);

	//Parte dada pelo professor e que servira de base para o restante das buscas
	printf("Bytes por Setor: %hd \n", boot_record.bytes_per_sector);
    printf("Setores por Cluster: %x \n", boot_record.sectors_per_cluster);
	printf("Setores reservados: %hd \n", boot_record.reserved_sector_count);
	printf("Numero de FAT's na imagem: %x \n", boot_record.table_count);
	printf("Setores por FAT: %hd \n", boot_record.table_size_16);
	printf("Numero de entradas no root_dir: %hd \n\n", boot_record.root_entry_count);
	
	return;
}

void ReadData(short file_cluster, int pos_fat1, FILE *fp, fat_BS_t boot_record, int tamanho, int firs_root_dir){
    std::vector<short> cluster_data;

    while(file_cluster != -1){
        cluster_data.push_back(file_cluster);
        fseek(fp, (pos_fat1+(file_cluster*2)), SEEK_SET);
        fread(&file_cluster, 2, 1, fp);
    }
    
    
    int aux;
    char content;
    for(int i=0; i<=tamanho/boot_record.bytes_per_sector; i++){
        aux = (cluster_data[i]-2) * boot_record.sectors_per_cluster*boot_record.bytes_per_sector;
        aux += (((boot_record.root_entry_count*32)/boot_record.bytes_per_sector) * boot_record.bytes_per_sector) + firs_root_dir; 
        
        for(int j=0; j<boot_record.bytes_per_sector; j++){
            fseek(fp, aux, SEEK_SET);
            fread(&content, 1, 1, fp);
            printf("%c", content);
             aux++;
       }
    }
    return;
}

int main()
{

    FILE *fp;
    fat_BS_t  boot_record;
	root_dir_t root_dir;
	int firs_root_dir, begin_fat1;
    std::vector<short> file_first_cluster;
    std::vector<int> tamanho;

    fp= fopen("fat16_1sectorpercluster.img", "rb");
	fseek(fp, 0, SEEK_SET);
    fread(&boot_record, sizeof(fat_BS_t),1, fp);

	printf("==========\tDADOS DO BOOT RECORD\t==========\n\n");
	ReadBootRecord(boot_record, fp);

	firs_root_dir = (((short int)boot_record.table_count * boot_record.table_size_16) + boot_record.reserved_sector_count) * boot_record.bytes_per_sector; 

	printf("=======\tENTRADAS FORMAT 8.3 DO ROOT_DIR\t========\n\n");
	ReadRootDir(firs_root_dir, root_dir, fp, file_first_cluster, tamanho);

    begin_fat1 = boot_record.reserved_sector_count * boot_record.bytes_per_sector;
    
    ReadData(file_first_cluster[1], begin_fat1, fp, boot_record, tamanho[1], firs_root_dir);

    fclose(fp);
    return 0;
}
