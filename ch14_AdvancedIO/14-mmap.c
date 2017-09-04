#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define COPYSIZE (1024*1024*1024)

int main(int argc, char const *argv[])
{
	int 		fin, fout;
	void		*src, *dst;
	size_t 		copysz;
	struct stat	sbuf;
	off_t 		fsz = 0;

	if(argc != 3){
		printf("usage: %s <fromfile> <tofile>\n", argv[0]);
		return -1;
	}

	if((fin = open(argv[1], O_RDWR)) < 0){
		perror("open error");
		return -1;
	}

	if((fout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
		perror("open error");
		return -1;
	}

	if(fstat(fin, &sbuf) < 0){
		perror("fstat error");
		return -1;
	}

	if(ftruncate(fout, sbuf.st_size) < 0){
		perror("ftruncate error");
		return -1;
	}

	while(fsz < sbuf.st_size){
		if((sbuf.st_size - fsz) > COPYSIZE)
			copysz = COPYSIZE;
		else
			copysz = sbuf.st_size -fsz;

		if((src = mmap(0, copysz, PROT_READ|PROT_WRITE, MAP_SHARED, fin, fsz)) == MAP_FAILED){
			perror("map fin error");
			return -1;
		}

		if((dst = mmap(0, copysz, PROT_READ|PROT_WRITE, MAP_SHARED, fin, fsz)) == MAP_FAILED){
			perror("map fin error");
			return -1;
		}
		memcpy(dst, src, copysz);
		munmap(src, copysz);
		munmap(dst, copysz);
		fsz += copysz;
	}


	return 0;
}



/*
1:原型
	void *mmap(void *addr, size_t len, int prot, int flag, int fd, off_t off);
	函数将将一个已经open的文件映射到内存中,操作内存就像操作文件本身一样,可以越过read/write来读写文件
2:参数
	addr:指定文件映射之后内存起始地址,通常指定NULL,让内核返回选择并返回起始地址
	 len:映射区域的长度
	prot:表示映射后内存区域的操作权限,四种取值(一般取值PROT_READ | PROT_WRITE,并且映射区域的权限要和open文件时的权限一致)
		 1:PROT_READ   --> 映射区域可读
		 2:PROT_WRITE  --> 映射区域可写
		 3:PROT_EXEC   --> 映射区域可执行
		 4:PROT_NONE   --> 映射区域不可访问
	flag:描述了映射区域的一些属性,
		 1:MAP_FIXED   --> 返回地址等于addr,如果addr=NULL,指定这个属性将造成困扰,不推荐使用,即使addr!=NULL,内核也只是将addr作为映射位置的参考,
		 最后的映射区域起始位置不一定就是addr指向的位置
		 2:MAP_SHARED  --> 表示对映射区域的修改操作同步到真实文件
		 3:MAP_PRIVATE --> 表示对映射区域的修改是存在某个副本中,不影响真实文件(MAP_SHARED和MAP_PRIVATE必须指定其一)
	  fd:已打开的文件描述符
	 off:表示映射文件的偏移量,一般为0表示从文件起始开始映射整个文件
*/