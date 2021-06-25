//
// Simple FIle System
// Student Name : 이세인
// Student Number : B811129
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* optional */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
/***********/

#include "sfs_types.h"
#include "sfs_func.h"
#include "sfs_disk.h"
#include "sfs.h"

void dump_directory();

/* BIT operation Macros */
/* a=target variable, b=bit number to act upon 0-n */
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))

static struct sfs_super spb;	// superblock
static struct sfs_dir sd_cwd = { SFS_NOINO }; // current working directory

struct sfs_bit{
	u_int8_t bitline[512];
};
struct partfile
{
	u_int8_t buffer[512];
};

int eight =0;
int eight2=0;

void error_message(const char *message, const char *path, int error_code) {
	switch (error_code) {
	case -1:
		printf("%s: %s: No such file or directory\n",message, path); return;
	case -2:
		printf("%s: %s: Not a directory\n",message, path); return;
	case -3:
		printf("%s: %s: Directory full\n",message, path); return;
	case -4:
		printf("%s: %s: No block available\n",message, path); return;
	case -5:
		printf("%s: %s: Not a directory\n",message, path); return;
	case -6:
		printf("%s: %s: Already exists\n",message, path); return;
	case -7:
		printf("%s: %s: Directory not empty\n",message, path); return;
	case -8:
		printf("%s: %s: Invalid argument\n",message, path); return;
	case -9:
		printf("%s: %s: Is a directory\n",message, path); return;
	case -10:
		printf("%s: %s: Is not a file\n",message, path); return;
	default:
		printf("unknown error code\n");
		return;
	}
}
int bitset(const char *mess, const char *path){
	int a = SFS_BITBLOCKS(spb.sp_nblocks);
	int ishere=0;
	int newblock_ino, m,n;
	for(m=0;m<a;m++){
		struct sfs_bit bit;
		disk_read(&bit,m+2);
		for(n=0;n<512;n++){
			if(BIT_CHECK(bit.bitline[n],7)==0){
				//printf("bitline# that have 0: %d\n", bit.bitline[n]);
				int k;
				for(k=0;k<8;k++){
					if(BIT_CHECK(bit.bitline[n],k)==0){
						newblock_ino=n*8+k;
						BIT_SET(bit.bitline[n],k);
						disk_write(&bit,m+2);
						ishere=1;
						return newblock_ino;
					}
				}				
			}
		}
	}
	if(ishere==0){
		error_message(mess,path,-4);//inode 꽉참
		return;
	}

}

void sfs_mount(const char* path)
{
	if( sd_cwd.sfd_ino !=  SFS_NOINO )
	{
		//umount
		disk_close();
		printf("%s, unmounted\n", spb.sp_volname);
		bzero(&spb, sizeof(struct sfs_super));
		sd_cwd.sfd_ino = SFS_NOINO;
	}

	printf("Disk image: %s\n", path);

	disk_open(path);
	disk_read( &spb, SFS_SB_LOCATION );

	printf("Superblock magic: %x\n", spb.sp_magic);

	assert( spb.sp_magic == SFS_MAGIC );
	
	printf("Number of blocks: %d\n", spb.sp_nblocks);
	printf("Volume name: %s\n", spb.sp_volname);
	printf("%s, mounted\n", spb.sp_volname);
	
	sd_cwd.sfd_ino = 1;		//init at root
	sd_cwd.sfd_name[0] = '/';
	sd_cwd.sfd_name[1] = '\0';
}

void sfs_umount() {

	if( sd_cwd.sfd_ino !=  SFS_NOINO )
	{
		//umount
		disk_close();
		printf("%s, unmounted\n", spb.sp_volname);
		bzero(&spb, sizeof(struct sfs_super));
		sd_cwd.sfd_ino = SFS_NOINO;
	}
}

void sfs_touch(const char* path)
{
	struct sfs_inode si;
	disk_read( &si, sd_cwd.sfd_ino );//현재 디렉토리 아이노드
	assert( si.sfi_type == SFS_TYPE_DIR );
		
	int i,j;
	char mess[6]="touch";
	int ishere=0;

	struct sfs_inode c_inode;
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,path)==0){						
						ishere=1;
						i=15;break;
					}	
				}
			}
		}
	}
	if(ishere==1){//path 이미 있는 경우
		error_message(mess,path,-6);
		return;
	}ishere =0;
	
	int a = SFS_BITBLOCKS(spb.sp_nblocks);//printf("bitmap#: %d\n",a);
	int makenew=0;
	int newblock_ino,newblock_ino2,m,n;
	
	int ii;
	for(i=0;i<15;i++){
		struct sfs_dir sd[8];
		disk_read(sd, si.sfi_direct[i]);		
		int j;
		for(j=0;j<8;j++){
			if(sd[j].sfd_ino==0){
				//newblock_ino=bitset(mess, path);

				for(m=0;m<a;m++){
					struct sfs_bit bit;
					disk_read(&bit,m+2);
					for(n=0;n<512;n++){
						if(BIT_CHECK(bit.bitline[n],7)==0){
							//printf("bitline# that have 0: %d\n", bit.bitline[n]);
							int k;
							for(k=0;k<8;k++){
								if(BIT_CHECK(bit.bitline[n],k)==0){
									newblock_ino=n*8+k;
									//printf("dir i# : %d\n",newblock_ino);
									BIT_SET(bit.bitline[n],k);
									disk_write(&bit,m+2);
									ishere=1;
									n=512;m=a;break;
								}
							}				
						}
					}
				}
				if(ishere==0){
					error_message(mess,path,-4);//inode 꽉참
					return;
				}ishere=0;

				if(eight==1){
					eight=0;
					//printf("zero, %d\n",i);
					makenew=1;
					ii=i;
					i=15; break;
				}
				if(j==7)
					eight=1;
				sd[j].sfd_ino=newblock_ino;
				strncpy(sd[j].sfd_name, path, SFS_NAMELEN );
				disk_write(sd,si.sfi_direct[i]);
				ishere=1;
				i=15; break;
			}
		}
	}
	if(makenew==1){
		//newblock_ino2=bitset(mess, path);
		for(m=0;m<a;m++){
			struct sfs_bit bit;
			disk_read(&bit,m+2);
			for(n=0;n<512;n++){
				if(BIT_CHECK(bit.bitline[n],7)==0){
					//printf("bitline# that have 0: %d\n", bit.bitline[n]);
					int k;
					for(k=0;k<8;k++){
						if(BIT_CHECK(bit.bitline[n],k)==0){
							newblock_ino2=n*8+k;
							//printf("dir2 i# : %d\n",newblock_ino2);
							BIT_SET(bit.bitline[n],k);
							disk_write(&bit,m+2);
							ishere=1;
							n=512;m=a;break;
						}
					}				
				}
			}
		}
		if(ishere==0){
			error_message(mess,path,-4);//inode 꽉참
			return;
		}
							
		si.sfi_direct[ii]=newblock_ino;
		struct sfs_dir dir_entry[8];
		for(i=0;i<8;i++){
			dir_entry[i].sfd_ino=SFS_NOINO;
			strncpy(dir_entry[i].sfd_name, "", SFS_NAMELEN );
		}
		dir_entry[0].sfd_ino=newblock_ino2;
		strncpy(dir_entry[0].sfd_name, path, SFS_NAMELEN );	
		disk_write(dir_entry, newblock_ino);		
	}
			
	if(ishere==0){//디렉토리 꽉참
		error_message(mess,path,-3);
		return;
	}ishere=0;
	
	
	si.sfi_size += sizeof(struct sfs_dir);
	disk_write( &si, sd_cwd.sfd_ino );

	struct sfs_inode newinode;
	bzero(&newinode,SFS_BLOCKSIZE);
	newinode.sfi_size = 0;
	newinode.sfi_type = SFS_TYPE_FILE;

	if(makenew==0)disk_write( &newinode, newblock_ino);
	if(makenew==1)disk_write( &newinode, newblock_ino2);	
}

void sfs_cd(const char* path)
{
	char mess[3]="cd";
	u_int32_t a;
	int ishere=0;
	if(path == NULL){
        sd_cwd.sfd_ino=SFS_ROOT_LOCATION;
		return;
	}

	int i;
	struct sfs_inode c_inode;
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기
		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,path)==0){
						struct sfs_inode check;
						disk_read(&check, dir_entry[j].sfd_ino);
						if(check.sfi_type==SFS_TYPE_DIR){						
							ishere=1;
							a=dir_entry[j].sfd_ino;
							i=15;break;
						}
						else{
							error_message(mess,path,-2);
							return;
						}					
					}	
				}
			}

		}
	}
		
	if(ishere==0){
		error_message(mess,path,-1);
		return;
	}
    sd_cwd.sfd_ino=a;
	strncpy(sd_cwd.sfd_name, path, SFS_NAMELEN );
}

void sfs_ls(const char* path)
{	
	char mess[3]="ls";
	u_int32_t a;
	int ishere = 0;
	if(path == NULL)
		a=sd_cwd.sfd_ino;

	else{
		int i;
		struct sfs_inode c_inode;
		disk_read(&c_inode, sd_cwd.sfd_ino);
		for(i=0;i<15;i++){//direct 15개 살펴보기
		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,path)==0){
						struct sfs_inode check;
						disk_read(&check, dir_entry[j].sfd_ino);
						if(check.sfi_type!=SFS_TYPE_DIR){
							printf("%s\n", path);
							return;
						}
						ishere=1;
						a=dir_entry[j].sfd_ino;
						i=15;break;
					}	
				}
			}

		}
		}
		
		if(ishere==0){
			error_message(mess,path,-1);
			return;
		}
	}
	ishere=0;
	int i;
	struct sfs_inode c_inode;
	disk_read(&c_inode, a);
	for(i=0;i<15;i++){//direct 15개 살펴보기
		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					printf("%s",dir_entry[j].sfd_name);
					struct sfs_inode inode;
					disk_read(&inode,dir_entry[j].sfd_ino);
					if (inode.sfi_type == SFS_TYPE_DIR)//디렉토리 타입이면 /붙여서 출력
						printf("/");

					printf("\t");
				}
			}

		}
	}			
	printf("\n");
	
}

void sfs_mkdir(const char* org_path) 
{	
	struct sfs_inode si;
	disk_read( &si, sd_cwd.sfd_ino );//현재 디렉토리 아이노드
	assert( si.sfi_type == SFS_TYPE_DIR );
	
	int i,j;
	char mess[6]="mkdir";
	int ishere=0;

	struct sfs_inode c_inode;
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,org_path)==0){						
						ishere=1;
						i=15;break;
					}	
				}
			}
		}
	}
	if(ishere==1){//path 이미 있는 경우
		error_message(mess,org_path,-6);
		return;
	}ishere=0;

	int a = SFS_BITBLOCKS(spb.sp_nblocks);
	int makenew=0;
	int newblock_ino, newblock_ino2, newblock_ino3, m,n;	
	
	int ii;
	for(i=0;i<15;i++){
		struct sfs_dir sd[8];
		disk_read(sd, si.sfi_direct[i]);		
		int j;

		for(j=0;j<8;j++){
			if(sd[j].sfd_ino==0){
				//newblock_ino=bitset(mess,org_path);
				//newblock_ino2=bitset(mess,org_path);
				for(m=0;m<a;m++){
				struct sfs_bit bit;
				disk_read(&bit,m+2);
				for(n=0;n<512;n++){
					if(BIT_CHECK(bit.bitline[n],7)==0){
						int k;
						for(k=0;k<8;k++){
							if(BIT_CHECK(bit.bitline[n],k)==0){
								newblock_ino=n*8+k;
								//printf("dir i# : %d\n",newblock_ino);
								BIT_SET(bit.bitline[n],k);
								disk_write(&bit,m+2);
								ishere=1;
								n=512;m=a;break;
							}
						}				
					}
				}
			}
			for(m=0;m<a;m++){
				struct sfs_bit bit;
				disk_read(&bit,m+2);
				for(n=0;n<512;n++){
					if(BIT_CHECK(bit.bitline[n],7)==0){
						int k;
						for(k=0;k<8;k++){
							if(BIT_CHECK(bit.bitline[n],k)==0){
								newblock_ino2=n*8+k;
								//printf("dir i2# : %d\n",newblock_ino2);
								BIT_SET(bit.bitline[n],k);
								disk_write(&bit,m+2);
								ishere=1;
								n=512;m=a;break;
							}
						}				
					}
				}
			}
			if(ishere==0){
				error_message(mess,org_path,-4);//inode 꽉참
				return;
			}ishere=0;


			if(eight==1){
				eight=0;
				//printf("zero, %d\n",i);
				makenew=1;
				ii=i;
				i=15; break;
			}
			if(j==7)
				eight=1;
			sd[j].sfd_ino=newblock_ino;
			strncpy(sd[j].sfd_name, org_path, SFS_NAMELEN );
			//printf("%d name : %s\n", sd[j].sfd_ino, org_path);
			disk_write(sd,si.sfi_direct[i]);
			ishere=1;
			i=15; break;
			}
		}
	}

	if(makenew==1){
		//newblock_ino3=bitset(mess,org_path);
		for(m=0;m<a;m++){
			struct sfs_bit bit;
			disk_read(&bit,m+2);
			for(n=0;n<512;n++){
				if(BIT_CHECK(bit.bitline[n],7)==0){
					int k;
					for(k=0;k<8;k++){
						if(BIT_CHECK(bit.bitline[n],k)==0){
							newblock_ino3=n*8+k;
							//printf("i3# : %d\n",newblock_ino3);
							BIT_SET(bit.bitline[n],k);
							disk_write(&bit,m+2);
							ishere=1;
							n=512;m=a;break;
						}
					}				
				}
			}
		}
		if(ishere==0){
			error_message(mess,org_path,-4);//inode 꽉참
			return;
		}

		si.sfi_direct[ii]=newblock_ino;//22

		struct sfs_dir dir_entry[8];
		for(i=0;i<8;i++){
			dir_entry[i].sfd_ino=SFS_NOINO;
			strncpy(dir_entry[i].sfd_name, "", SFS_NAMELEN );
		}
		dir_entry[0].sfd_ino=newblock_ino2;//23
		strncpy(dir_entry[0].sfd_name, org_path, SFS_NAMELEN );	
		disk_write(dir_entry, newblock_ino);

		struct sfs_inode newinode;		
	}

	if(ishere==0){//디렉토리 꽉참
		error_message(mess,org_path,-3);
		return;
	}ishere==0;

	si.sfi_size += sizeof(struct sfs_dir);
	disk_write( &si, sd_cwd.sfd_ino );

	struct sfs_inode newinode;
	bzero(&newinode,SFS_BLOCKSIZE);
	newinode.sfi_size = 128;
	newinode.sfi_type = SFS_TYPE_DIR;
	if(makenew==0)newinode.sfi_direct[0]=newblock_ino2;
	if(makenew==1)newinode.sfi_direct[0]=newblock_ino3;

	if(makenew==0)disk_write( &newinode, newblock_ino);
	if(makenew==1)disk_write( &newinode, newblock_ino2);
	
	struct sfs_dir dir_entry[8];
	for(i=0;i<8;i++){
		dir_entry[i].sfd_ino=SFS_NOINO;
		strncpy(dir_entry[i].sfd_name, "", SFS_NAMELEN );
	}
	strncpy(dir_entry[0].sfd_name, ".", SFS_NAMELEN );
	strncpy(dir_entry[1].sfd_name, "..", SFS_NAMELEN );

	if(makenew==0)dir_entry[0].sfd_ino=newblock_ino;
	if(makenew==1)dir_entry[0].sfd_ino=newblock_ino2;

	dir_entry[1].sfd_ino=sd_cwd.sfd_ino;

	if(makenew==0)disk_write(dir_entry, newblock_ino2);
	if(makenew==1)disk_write(dir_entry, newblock_ino3);
}
	

void sfs_rmdir(const char* org_path) 
{
	struct sfs_inode si;
	disk_read( &si, sd_cwd.sfd_ino );//현재 디렉토리 아이노드
	assert( si.sfi_type == SFS_TYPE_DIR );
	
	int i,j;
	char mess[6]="rmdir";
	int ishere=0;

	if(strcmp(".",org_path)==0){
		error_message(mess,org_path,-8);
		return;
	}

	int rmnum=-1;
	int ii,jj;
	struct sfs_inode c_inode;
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,org_path)==0){	
						struct sfs_inode check;
						disk_read(&check, 
						dir_entry[j].sfd_ino);
						if(check.sfi_type==SFS_TYPE_FILE){
							ishere=2;i=15;break;
						}
						ii=i; jj=j;//0, 4
						rmnum=dir_entry[j].sfd_ino;
						ishere=1;i=15;break;
					}
				}
			}
		}
	}
	if(ishere==2){//디렉토리 아닌 경우
		error_message(mess,org_path,-2);
		return;
	}
	if(ishere==0){//path가 없는 경우
		error_message(mess,org_path,-1);
		return;
	}ishere=0;

	disk_read(&c_inode,rmnum);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){				
					if(strcmp(dir_entry[j].sfd_name,".")!=0&&
					strcmp(dir_entry[j].sfd_name,"..")!=0){
						ishere=1;
						i=15;break;
					}	
				}
			}
		}
	}
	if(ishere==1){//디렉토리가 비어있지 않은 경우
		error_message(mess,org_path,-7);
		return;
	}
	//printf("%d, %d, %d\n",ii,jj,si.sfi_direct[ii]);
	si.sfi_size -= sizeof(struct sfs_dir);
	disk_write( &si, sd_cwd.sfd_ino );

	struct sfs_dir dir_entry[8];
	disk_read(dir_entry, si.sfi_direct[ii]);
	dir_entry[jj].sfd_ino=0;
	disk_write(dir_entry,si.sfi_direct[ii]);
	
	int a = rmnum/SFS_BLOCKSIZE;//몇번째 블록인지
	int b = (rmnum%SFS_BLOCKSIZE)/8;//몇번째 줄인지(0-511)
	int c = (rmnum%SFS_BLOCKSIZE)%8;//몇번째인지(0-7)

	struct sfs_bit bit;
	disk_read(&bit,a+2);
	/*#define BIT_SET(a,b) ((a) |= (1<<(b)))
	#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
	#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))
	#define BIT_CHECK(a,b) ((a) & (1<<(b)))*/
	BIT_CLEAR(bit.bitline[b],c);
	if(c!=7)BIT_CLEAR(bit.bitline[b],c+1);
	if(c==7)BIT_CLEAR(bit.bitline[b+1],0);
	disk_write(&bit,a+2);	

}

void sfs_mv(const char* src_name, const char* dst_name) 
{
	struct sfs_inode si;
	disk_read( &si, sd_cwd.sfd_ino );//현재 디렉토리 아이노드
	assert( si.sfi_type == SFS_TYPE_DIR );
	
	int i,j;
	char mess[3]="mv";
	int ishere=0;

	struct sfs_inode c_inode;
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,dst_name)==0){						
						ishere=1;
						i=15;break;
					}
				}
			}
		}
	}
	if(ishere==1){//path2 이미 있는 경우
		error_message(mess,dst_name,-6);
		return;
	}

	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,src_name)==0){						
						ishere=1;
						i=15;break;
					}
				}
			}
		}
	}
	if(ishere==0){//path1 없는 경우
		error_message(mess,src_name,-1);
		return;
	}ishere=0;

	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,src_name)==0){						
						strncpy(dir_entry[j].sfd_name,dst_name,SFS_NAMELEN);
						disk_write(dir_entry,si.sfi_direct[i]);
						i=15;break;
					}	
				}
			}
		}
	}
}

void sfs_rm(const char* path) 
{
	struct sfs_inode si;
	disk_read( &si, sd_cwd.sfd_ino );//현재 디렉토리 아이노드
	assert( si.sfi_type == SFS_TYPE_DIR );
	
	int i,j;
	char mess[3]="rm";
	int ishere=0;

	int rmnum=-1;
	int ii,jj;
	struct sfs_inode c_inode;
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,path)==0){	
						struct sfs_inode check;
						disk_read(&check,dir_entry[j].sfd_ino);
						if(check.sfi_type==SFS_TYPE_DIR){
							ishere=2;i=15;break;
						}
						ii=i; jj=j;
						rmnum=dir_entry[j].sfd_ino;//10
						ishere=1;i=15;break;
					}
				}
			}
		}
	}
	if(ishere==2){//파일 아닌 경우
		error_message(mess,path,-9);
		return;
	}
	if(ishere==0){//path가 없는 경우
		error_message(mess,path,-1);
		return;
	}ishere=0;

	disk_read(&c_inode,rmnum);
	//printf("rm : %d\n", rmnum);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			int a = c_inode.sfi_direct[i]/SFS_BLOCKSIZE;
			int b = (c_inode.sfi_direct[i]%SFS_BLOCKSIZE)/8;
			int c = (c_inode.sfi_direct[i]%SFS_BLOCKSIZE)%8;
			struct sfs_bit bit;
			disk_read(&bit,a+2);
			BIT_CLEAR(bit.bitline[b],c);
			disk_write(&bit,a+2);									
		}
	}
	if(c_inode.sfi_indirect!=0){
		int a = c_inode.sfi_indirect/SFS_BLOCKSIZE;
		int b = (c_inode.sfi_indirect%SFS_BLOCKSIZE)/8;
		int c = (c_inode.sfi_indirect%SFS_BLOCKSIZE)%8;
		struct sfs_bit bit;
		disk_read(&bit,a+2);
		BIT_CLEAR(bit.bitline[b],c);
		disk_write(&bit,a+2);									
	}

	si.sfi_size -= sizeof(struct sfs_dir);
	disk_write( &si, sd_cwd.sfd_ino );

	struct sfs_dir dir_entry[8];
	disk_read(dir_entry, si.sfi_direct[ii]);
	dir_entry[jj].sfd_ino=0;
	disk_write(dir_entry,si.sfi_direct[ii]);
	
	int a = rmnum/SFS_BLOCKSIZE;//몇번째 블록인지
	int b = (rmnum%SFS_BLOCKSIZE)/8;//몇번째 줄인지(0-511)
	int c = (rmnum%SFS_BLOCKSIZE)%8;//몇번째인지(0-7)

	struct sfs_bit bit;
	disk_read(&bit,a+2);
	BIT_CLEAR(bit.bitline[b],c);
	disk_write(&bit,a+2);
}

void sfs_cpin(const char* local_path, const char* path) 
{
	struct sfs_inode si;
	disk_read( &si, sd_cwd.sfd_ino );//현재 디렉토리 아이노드
	assert( si.sfi_type == SFS_TYPE_DIR );
		
	int i,j;
	char mess[5]="cpin";
	int ishere=0;

	struct sfs_inode c_inode;
	bzero(&c_inode,SFS_BLOCKSIZE);
	disk_read(&c_inode, sd_cwd.sfd_ino);
	for(i=0;i<15;i++){//direct 15개 살펴보기		
		if(c_inode.sfi_direct[i]!=0){
			struct sfs_dir dir_entry[8];
			bzero(dir_entry,512);
			disk_read(dir_entry, c_inode.sfi_direct[i]);
			int j;
			for(j=0;j<8;j++){
				if(dir_entry[j].sfd_ino!=0){
					if(strcmp(dir_entry[j].sfd_name,local_path)==0){						
						ishere=1;
						i=15;break;
					}	
				}
			}
		}
	}
	if(ishere==1){//local_path 이미 있는 경우
		error_message(mess,local_path,-6);
		return;
	}ishere =0;

	FILE *fp2;
	fp2 = fopen(path,"r");
	if(fp2 == NULL){//path파일 없는 경우
		printf("%s: can't open %s input file\n",mess, path);
		return;
	}

	int size; 
	fseek(fp2, 0, SEEK_END);
    size = ftell(fp2);//printf("size : %d\n",size);
	fclose(fp2);
	if(size > 73216){//사이즈를 초과한경우
		printf("%s: input file size exceeds the max file size\n",mess);
		return;
	}

	int usein=0;
	int a = size/SFS_BLOCKSIZE;//printf("a : %d\n",a);
	int b= size%SFS_BLOCKSIZE;//printf("b : %d\n",b);
	if(a>14){
		usein=1; a=15; b=size-(a*512); 
		//printf("a : %d\n",a);printf("b : %d\n",b);
	}
	
	int aa = SFS_BITBLOCKS(spb.sp_nblocks);
	int newblock_ino, newblock_ino2, m,n;
	int makenew=0; int ii;

	for(i=0;i<15;i++){//direct 15개 살펴보기		
		struct sfs_dir dir_entry[8];
		bzero(dir_entry,512);
		disk_read(dir_entry, si.sfi_direct[i]);
		int j;
		for(j=0;j<8;j++){
			if(dir_entry[j].sfd_ino==0){		
					
				for(m=0;m<aa;m++){
					struct sfs_bit bit;
					bzero(&bit,512);
					disk_read(&bit,m+2);
					for(n=0;n<512;n++){
						if(BIT_CHECK(bit.bitline[n],7)==0){
							int k;
							for(k=0;k<8;k++){
								if(BIT_CHECK(bit.bitline[n],k)==0){
									newblock_ino=n*8+k;
									//printf("1: %d\n",newblock_ino);
									BIT_SET(bit.bitline[n],k);
									disk_write(&bit,m+2);
									ishere=1;
									n=512;m=aa;break;
								}
							}				
						}
					}
				}
				if(ishere==0){//inode 할당 못받는 경우
					error_message(mess,local_path,-4);
					return;
				}ishere=0;

				if(eight2==1){
					eight=0;
					makenew=1;
					ii=i;
					i=15; break;
				}
				if(j==7)
					eight2=1;

				dir_entry[j].sfd_ino=newblock_ino;
				strncpy(dir_entry[j].sfd_name,local_path,SFS_NAMELEN);
				disk_write(dir_entry,si.sfi_direct[i]);					
				ishere=1;
				i=15;break;						
			}
		}
	}
	

	if(makenew==1){
		for(m=0;m<aa;m++){
			struct sfs_bit bit;
			bzero(&bit,512);
			disk_read(&bit,m+2);
			for(n=0;n<512;n++){
				if(BIT_CHECK(bit.bitline[n],7)==0){
					//printf("bitline# that have 0: %d\n", bit.bitline[n]);
					int k;
					for(k=0;k<8;k++){
						if(BIT_CHECK(bit.bitline[n],k)==0){
							newblock_ino2=n*8+k;
							//printf("dir2 i# : %d\n",newblock_ino2);
							BIT_SET(bit.bitline[n],k);
							disk_write(&bit,m+2);
							ishere=1;
							n=512;m=aa;break;
						}
					}				
				}
			}
		}
		if(ishere==0){
			error_message(mess,local_path,-4);//inode 꽉참
			return;
		}
							
		bzero(&si,SFS_BLOCKSIZE);
		disk_read( &si, sd_cwd.sfd_ino );
		si.sfi_direct[ii]=newblock_ino;//printf("ii : %d\n",ii);
		disk_write(&si,sd_cwd.sfd_ino);
		struct sfs_dir dir_entry[8];
		bzero(dir_entry,512);
		for(i=0;i<8;i++){
			dir_entry[i].sfd_ino=SFS_NOINO;
			strncpy(dir_entry[i].sfd_name, "", SFS_NAMELEN );
		}
		dir_entry[0].sfd_ino=newblock_ino2;
		strncpy(dir_entry[0].sfd_name, path, SFS_NAMELEN );	
		disk_write(dir_entry, newblock_ino);		
	}

	if(ishere==0){//디렉토리 꽉참
		error_message(mess,local_path,-3);
		return;
	}ishere=0;

	bzero(&si,SFS_BLOCKSIZE);
	disk_read( &si, sd_cwd.sfd_ino );
	si.sfi_size +=sizeof(struct sfs_dir);
	disk_write(&si,sd_cwd.sfd_ino);

	struct sfs_inode check;
	bzero(&check,SFS_BLOCKSIZE);
	check.sfi_type=SFS_TYPE_FILE;
	check.sfi_size=0;
	for(m=0;m<15;m++){
		check.sfi_direct[m]=SFS_TYPE_INVAL;
	}
	check.sfi_indirect=SFS_TYPE_INVAL;
	if(makenew==0)disk_write(&check,newblock_ino);
	if(makenew==1)disk_write(&check,newblock_ino2);


	//int newblock_ino2; 3으로 고치기
	FILE *fp;
	fp = fopen(path,"r");
	u_int8_t buffer[512];
	memset(buffer,0,512);

	bzero(&c_inode,SFS_BLOCKSIZE);
	disk_read(&c_inode,newblock_ino);//14
	c_inode.sfi_size=size;
	disk_write(&c_inode,newblock_ino);

	for(i=0;i<a;i++){

		for(m=0;m<aa;m++){
			struct sfs_bit bit;
			bzero(&bit,512);
			disk_read(&bit,m+2);
			for(n=0;n<512;n++){
				if(BIT_CHECK(bit.bitline[n],7)==0){
					int k;
					for(k=0;k<8;k++){
						if(BIT_CHECK(bit.bitline[n],k)==0){
							newblock_ino2=n*8+k;//15
							//printf("2: %d\n",newblock_ino2);
							BIT_SET(bit.bitline[n],k);
							disk_write(&bit,m+2);
							ishere=1;
							n=512;m=aa;break;
						}
					}				
				}
			}
		}
		if(ishere==0){//inode 할당 못받는 경우
			error_message(mess,local_path,-4);
			return;
		}ishere=0;

		/*fread(buffer,sizeof(u_int8_t),512,fp);

		struct partfile pf;
		bzero(&pf,512);
		disk_write(&pf,newblock_ino2);	
				
		disk_write(&buffer,newblock_ino2);*/
		memset(buffer,0,512);

		
		bzero(&c_inode,SFS_BLOCKSIZE);
		disk_read(&c_inode,newblock_ino);
		c_inode.sfi_direct[i]=newblock_ino2;
		disk_write(&c_inode,newblock_ino);		
	}

	if(b!=0&&usein==0){
		for(m=0;m<aa;m++){
			struct sfs_bit bit;
			bzero(&bit,512);
			disk_read(&bit,m+2);
			for(n=0;n<512;n++){
				if(BIT_CHECK(bit.bitline[n],7)==0){
					int k;
					for(k=0;k<8;k++){
						if(BIT_CHECK(bit.bitline[n],k)==0){
							newblock_ino2=n*8+k;//15
							BIT_SET(bit.bitline[n],k);
							disk_write(&bit,m+2);
							ishere=1;m=aa;break;
						}
					}				
				}
			}
		}
		if(ishere==0){//inode 할당 못받는 경우
			error_message(mess,local_path,-4);
			return;
		}ishere=0;
		/*fread(buffer,sizeof(u_int8_t),b,fp);
		struct partfile pf;
		bzero(&pf,512);
		disk_write(&pf,newblock_ino2);
		disk_write(&buffer,newblock_ino2);*/
		memset(buffer,0,512);

		bzero(&c_inode,SFS_BLOCKSIZE);
		disk_read(&c_inode,newblock_ino);
		c_inode.sfi_direct[a+1]=newblock_ino2;
		disk_write(&c_inode,newblock_ino);
	}

	int b1 =b/512;//printf("b1 : %d\n",b1);
	int b2 =b%512;
	int newblock_ino3;

	if(usein==1){
		for(m=0;m<aa;m++){
			struct sfs_bit bit;
			bzero(&bit,512);
			disk_read(&bit,m+2);
			for(n=0;n<512;n++){
				if(BIT_CHECK(bit.bitline[n],7)==0){
					int k;
					for(k=0;k<8;k++){
						if(BIT_CHECK(bit.bitline[n],k)==0){
							newblock_ino3=n*8+k;
							//printf("3: %d\n",newblock_ino3);
							BIT_SET(bit.bitline[n],k);
							disk_write(&bit,m+2);
							ishere=1;
							n=512;m=aa;break;
						}
					}				
				}
			}
		}
		if(ishere==0){//inode 할당 못받는 경우
			error_message(mess,local_path,-4);
			return;
		}ishere=0;

		bzero(&c_inode,SFS_BLOCKSIZE);
		disk_read(&c_inode,newblock_ino);
		c_inode.sfi_indirect=newblock_ino3;
		disk_write(&c_inode,newblock_ino);

		for(i=0;i<(b1+1);i++){
			for(m=0;m<aa;m++){
				struct sfs_bit bit;
				bzero(&bit,512);
				disk_read(&bit,m+2);
				for(n=0;n<512;n++){
					if(BIT_CHECK(bit.bitline[n],7)==0){
						int k;
						for(k=0;k<8;k++){
							if(BIT_CHECK(bit.bitline[n],k)==0){
								newblock_ino3=n*8+k;//15
								//printf("3_2: %d\n",newblock_ino3);
								BIT_SET(bit.bitline[n],k);
								disk_write(&bit,m+2);
								ishere=1;
								n=512;m=aa;break;
							}
						}				
					}
				}
			}
			if(ishere==0){//inode 할당 못받는 경우
				error_message(mess,local_path,-4);
				return;
			}ishere=0;
		}
	}
	fclose(fp);
}

void sfs_cpout(const char* local_path, const char* path) 
{
	printf("Not Implemented\n");
}

void dump_inode(struct sfs_inode inode) {
	int i;
	struct sfs_dir dir_entry[SFS_DENTRYPERBLOCK];

	printf("size %d type %d direct ", inode.sfi_size, inode.sfi_type);
	for(i=0; i < SFS_NDIRECT; i++) {
		printf(" %d ", inode.sfi_direct[i]);
	}
	printf(" indirect %d",inode.sfi_indirect);
	printf("\n");

	if (inode.sfi_type == SFS_TYPE_DIR) {
		for(i=0; i < SFS_NDIRECT; i++) {
			if (inode.sfi_direct[i] == 0) break;
			disk_read(dir_entry, inode.sfi_direct[i]);
			dump_directory(dir_entry);
		}
	}

}

void dump_directory(struct sfs_dir dir_entry[]) {
	int i;
	struct sfs_inode inode;
	for(i=0; i < SFS_DENTRYPERBLOCK;i++) {
		printf("%d %s\n",dir_entry[i].sfd_ino, dir_entry[i].sfd_name);
		disk_read(&inode,dir_entry[i].sfd_ino);
		if (inode.sfi_type == SFS_TYPE_FILE) {
			printf("\t");
			dump_inode(inode);
		}
	}
}

void sfs_dump() {
	// dump the current directory structure
	struct sfs_inode c_inode;

	disk_read(&c_inode, sd_cwd.sfd_ino);
	printf("cwd inode %d name %s\n",sd_cwd.sfd_ino,sd_cwd.sfd_name);
	dump_inode(c_inode);
	printf("\n");

}

