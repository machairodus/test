[test] <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <scsi/sg.h>

void PRINT(const char *name, unsigned char *buf, int buf_len)
{
    int i = 0;
    //if(debugflag)
    {
        printf("\n-------------PRINT:%s-----------------\n", name);
        for(i=0; i<buf_len; i++)
        {
            printf("%02x ", buf[i]);
            if((i+1)%16 == 0)
                printf("\n");
        }
        printf("\n---------------------------------------\n");
    }
}
int ioctl_cmd(int fd, uint8_t *buf, uint32_t sector, uint32_t len, uint8_t op_code)
{
	int ret=0;
	uint8_t cmd[10]={0};
	struct sg_io_hdr *p_hdr = NULL;
	//cmd init
	cmd[0] = op_code;
	cmd[2] = (uint8_t)((sector>>24)& 0xFF);
	cmd[3] = (uint8_t)((sector>>16)& 0xFF);
	cmd[4] = (uint8_t)((sector>>8)& 0xFF);
	cmd[5] = (uint8_t)(sector & 0xFF);
	if(len%512){
		printf("读写长度必须为512字节整倍数！\n"); 
		return -1;
	}
	cmd[7] = (uint8_t)(((len/512)>>8)& 0xFF);
	cmd[8] = (uint8_t)((len/512)& 0xFF);
	//p_hdr init
	p_hdr = (struct sg_io_hdr *)malloc(sizeof(struct sg_io_hdr));
	p_hdr->interface_id = 'S';			//一般都设置为S
	p_hdr->cmd_len = sizeof(cmd);		//read10和write10都是10byte
	p_hdr->cmdp = cmd;					//SCSI指令
	p_hdr->dxfer_len = len;				//传输数据长度
	p_hdr->dxferp = buf;				//传输数据buffer
	p_hdr->iovec_count = 0; 			//不使用sg
	//传输方向
	if(op_code==0x28)
		p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	else if(op_code==0x2A)
		p_hdr->dxfer_direction = SG_DXFER_TO_DEV;

	ret = ioctl(fd, SG_IO, p_hdr);
	free(p_hdr);
	return ret;
}

int main()
{
	int ret;
	uint8_t buf[1024]={0};
	uint8_t tmp[1024]={0};
	int fd = open("/dev/sdb", O_RDWR);
	if(fd==-1){
		printf("open sdb failed!\n");
		return -1;	
	}
	//read 0 sec
	ret = ioctl_cmd(fd, buf, 0, 512, 0x28);
	PRINT("buf0", buf, 32);
	//read 100 sec
	ret = ioctl_cmd(fd, buf, 100, 512, 0x28);
	PRINT("buf1", buf, 32);
	//write 100 sec
	memset(tmp, 9, 1024);
	ret = ioctl_cmd(fd, tmp, 100, 512, 0x2A);
	//read 100 sec
	ret = ioctl_cmd(fd, buf, 100, 512, 0x28);
	PRINT("buf2", buf, 32);

	close(fd);
	return 0;
}




