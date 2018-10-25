#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "output.h"

typedef unsigned int       u32;
typedef unsigned long long u64;

typedef struct
{
    char name[0xff];
    u32  offset;
    u32  size;
} GRPFile;

typedef struct
{
    char name[0xff];
    u32  num_files;
} GRPFolder;

/*MAGIC VALUES
 * 0
 * 1
 * 10
 * 19
 * 11
 * 5
 * 22
 * ...?
 */

//extract entry to file
void extract(FILE* in, GRPFile* fil)
{
    char file_path[0xff] = "extracted/";
    strcat(file_path, fil->name);
    
    u64 pos = ftell(in);
    
    FILE* out = fopen(file_path, "wb");
    
    fseek(in, fil->offset, SEEK_SET);
    char* buffer = (char*)malloc(fil->size);
    
    fread(buffer,  sizeof(char), fil->size, in);
    fwrite(buffer, sizeof(char), fil->size, out);
    
    fclose(out);
    free(buffer);
    fseek(in, pos, SEEK_SET);
}



int main(int argc, char* argv[])
{
    //check argument validity
    if(argc != 2)
    {
        printf("usage: grp-extractor [INPUT]\n"); return 1;
    }
    
    FILE* in = fopen(argv[1], "rb");
    
    if(in == NULL)
    {
        printf("Invalid agruments\n"); return 2;
    }
    
    bool reading_entries = true;
    bool got_entry_size  = false;
    u32  entries_size    = 0;
    
    //create folder where extracted files will go to
    mkdir("extracted", 0777);
    
    //reading entries
    while(reading_entries)
    {
        //fetch magic number
        u32 magic;     fread(&magic, sizeof(u32), 1, in);
        
        printf("MAGIC?: %i\n", magic);
        
        //get folder name length
        u32 folder_name_length; fread(&folder_name_length, sizeof(u32), 1, in);
        
        //create folder
        GRPFolder fol;
        
        if(folder_name_length != 0)
        {
            //read name
            fread(fol.name, sizeof(char), folder_name_length, in);
            fol.name[folder_name_length] = 0;
        }
        
        //read number of files
        fread(&fol.num_files, sizeof(u32), 1, in);
        
        printf("%sFolder [name: %s] [number of files: %u]%s\n", FNT_RED, fol.name, fol.num_files, FNT_RESET);
        
        //read entries in folder
        for(u32 i = 0; i < fol.num_files; i++)
        {
            //read file name length
            u32 name_length; fread(&name_length, sizeof(u32), 1, in);
            
            //create file
            GRPFile fil;
            
            //read file name
            fread(fil.name, sizeof(char), name_length, in);
            fil.name[name_length] = 0;
            
            //read size and offset
            fread(&fil.offset, sizeof(u32), 1, in);
            if(!got_entry_size)
            {
                got_entry_size = true;
                entries_size = fil.offset;
                printf("%u\n", entries_size);
            }
            fread(&fil.size,   sizeof(u32), 1, in);
            
            printf("%sFile [i: %i] [name: %s] [offset: %u] [size: %u]%s\n", FNT_CYAN, i, fil.name, fil.offset, fil.size, FNT_RESET);
            
            //extract file
            extract(in, &fil);
        }
        
        //end if we reached entries' end
        if(ftell(in) >= entries_size - 4) { reading_entries = false; }
    }
    
    
    fclose(in);
}
