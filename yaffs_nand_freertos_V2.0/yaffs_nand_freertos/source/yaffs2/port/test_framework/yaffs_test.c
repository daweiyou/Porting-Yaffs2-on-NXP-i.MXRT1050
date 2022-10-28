#include "yaffsfs.h"
#include "fsl_debug_console.h"  

void error_trap(const char *error_info)
{
    printf("error trap:%s\r\n",error_info);
    while(1);
}
void ListDir(const char *DirName)
{
	int f = 0;
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(DirName);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		printf("%s\r\n", DirName);
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",DirName,de->d_name);
			yaffs_lstat(str,&s);
			printf("%20s        %8d     %5d    ", de->d_name, s.st_size, s.st_mode);
			
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: 
					printf("data file"); 
					f = yaffs_open(str, O_RDWR, S_IREAD);
					yaffs_read(f,str,100);
					str[99] = 0;
					printf("\nyaffs_read %s\r\n",str);

					break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink");   break;
				default: printf("unknown"); break;
			}
		
			printf("\r\n");		
		}
		
		yaffs_closedir(d);
	}
	
	printf("FreeSpace: %d\r\n\n", yaffs_freespace(DirName));

}


void yaffs_test_main(const char *mountpt)
{
	int i = 0;
	int f = 0;
	unsigned char buf[100];
    int retVal = -1;
	//struct yaffs_stat s;

	for(i=0;i<100;i++)
	{
		buf[i] = i;
	}

    printf("Format first\r\n");
    retVal = yaffs_format(mountpt,0,0,0);
    
	retVal = yaffs_mount(mountpt);

    if(retVal != 0)
    {
        printf("mount fail, format it\r\n");
        retVal = yaffs_format(mountpt,0,0,0);
        if(retVal!=0)
        {
            error_trap("format fail\r\n");
        }
        printf("format successfully, mount again\r\n");
        retVal = yaffs_mount(mountpt);
        if(retVal != 0)
        {
            error_trap("mount again fail\r\n");
        }
    }

	//yaffs_mkdir("/yaffs_nxp/mydoc", 0);
	//yaffs_mkdir("/yaffs_nxp/data0", 0);
	//yaffs_mkdir("/yaffs_nxp/data1", 0);
	
	ListDir("/yaffs_nxp");
	yaffs_mkdir("/yaffs_nxp/mydoc", 0);
	ListDir("/yaffs_nxp");

//	f = yaffs_open("/yaffs_nxp/data0/file1.gsk", O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
//	
//	yaffs_write(f,buf,100);
//
//	yaffs_close(f);
//	
//	f = yaffs_open("/yaffs_nxp/data0/file2.gsk", O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
//	yaffs_write(f,buf,100);
//
//	yaffs_close(f);


//	ListDir("/yaffs_nxp");
//	ListDir("/yaffs_nxp/data0");
}
