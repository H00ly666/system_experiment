/*************************************************************************
	> File Name: get_phy_addr.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月29日 星期六 12时09分46秒
 ************************************************************************/

#include<stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// 参见《Linux 获取虚拟地址对应的物理地址》
size_t virtual_to_physical(size_t addr)
{
    /*打开那个进程的pagemap*/
    char ptr[30];
    
    pid_t id = getpid();
    
    sprintf (ptr,"/proc/%d/pagemap", id);
    
    int fd = open(ptr, O_RDONLY);
    if(fd < 0)
    {
        printf("open '/proc/self/pagemap' failed!\n");
        return 0;
    }
    
    /*获取页面大小*/
    size_t pagesize = getpagesize();
    /*获取页面号*/
    size_t offset = (addr / pagesize) * sizeof(uint64_t);
    /*移动文件指针*/
    

    char str[4096];
    read(fd, &str,4096);
    printf("%s\n",str);




    if(lseek(fd, offset, SEEK_SET) < 0)
    {
        printf("lseek() failed!\n");
        close(fd);
        return 0;
    }
    
    /*读取uint64_t这么大字节的数据*/
    uint64_t info;
    if(read(fd, &info, sizeof(uint64_t)) != sizeof(uint64_t))
    {
        printf("read() failed!\n");
        close(fd);
        return 0;
    }
    /*将 64 位的 1 左移 63 位*/
    if((info & (((uint64_t)1) << 63)) == 0)
    {
        printf("page is not present!\n");
        close(fd);
        return 0;
    }
    /*左移55位再减一*/
    size_t frame = info & ((((uint64_t)1) << 55) - 1);
    /*物理地址就是页号*页面大小+地址对页面大小取余*/
    size_t phy = frame * pagesize + addr % pagesize;
    close(fd);
    return phy;
}

int main()
{
    char* str =(char*) malloc(128);
    strcpy(str,"hello,world!");
    printf("original, vir = %p, phy = %p, val = '%s'\n",
        str, (void*)virtual_to_physical((size_t)str), str);
    pid_t pid = fork();
    if(pid < 0)
    {
        printf("fork() failed!\n");
        return 1;
    }
    else if(pid > 0)
    {
        str[0] = 'H';
        printf("father,   vir = %p, phy = %p, val = '%s'\n",
            str, (void*)virtual_to_physical((size_t)str), str);
        wait(0);
    }
    else
    {
        printf("child,    vir = %p, phy = %p, val = '%s'\n",
            str, (void*)virtual_to_physical((size_t)str), str);
    }
    return 0;
}
