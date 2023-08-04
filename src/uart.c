/*
** File: uart.c
**
** Description:
** Provides an RS-232 interface that is very similar to the CVI provided
** interface library
*/

#include "uart.h"

/*this array hold information about each port we have opened */
struct PortInfo ports[13] = 
{
	{"/dev/ttyUSB0", 0},
	{"/dev/ttyUSB1", 0},
	{"/dev/ttyUSB2", 0}, 
	{"/dev/ttyUSB3", 0},
	{"/dev/ttyUSB4", 0},
	{"/dev/ttyUSB5", 0},
	{"/dev/ttyUSB6", 0},
	{"/dev/ttyUSB7", 0},
	{"/dev/ttyUSB8", 0},
	{"/dev/ttyUSB9", 0},
	{"/dev/ttyUSB10", 0},
	{"/dev/ttyUSB11", 0},
	{"/dev/ttyUSB12", 0},
};

int spd_arr[] = \
{B2000000, B1500000, B576000, B500000, B460800, B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400};

int name_arr[] = \
{ 2000000, 1500000,  576000,  500000,  460800,  230400,  115200,  57600,  38400,  19200,  9600,  4800,  2400 };
/////////////////////////////////////////////////////////////////////////////////////////
/**
*@brief  ���ô���ͨ������
*@param  fd    ���� int �򿪴��ڵ��ļ����
*@param  speed ���� int �����ٶ�
*@return  void
*/
int set_speed(int fd, int speed)
{
	int   i; 
	int   status; 
	struct termios Opt;
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(spd_arr) / sizeof(int);  i++) { 
		if(speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, spd_arr[i]);  
			cfsetospeed(&Opt, spd_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0) {        
				printf("tcsetattr failed");  
				return 1;     
			}
			tcflush(fd,TCIOFLUSH);   
		}
	}
	// printf("set_speed\n");
	return 0;
}

/**
*@brief  ���ô�������λ��ֹͣλ��Ч��λ
*@param  fd       ����  int �򿪵Ĵ����ļ����
*@param  databits ����  int ����λ   ȡֵΪ 7 ����8
*@param  stopbits ����  int ֹͣλ   ȡֵΪ 1 ����2
*@param  parity   ����  int Ч������ ȡֵΪN,E,O,,S
*/
int set_Parity(int fd, int databits, int parity, int stopbits, int RTSCTS)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*��������λ��*/
	{   
	case 7:		
		options.c_cflag |= CS7; 
		break;
	case 8:     
		options.c_cflag |= CS8;
		break;   
	default:    
		fprintf(stderr,"Unsupported data size\n"); 
        return -1;  
	}

    options.c_iflag |= INPCK;
    cfmakeraw(&options);
    //options.c_lflag |= (ICANON | ECHO | ECHOE);
    //options.c_lflag &= ~(ICANON | ECHO | ECHOE);
    //options.c_iflag &= ~(IXON | IXOFF);
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); /* ����Ϊ��Ч��*/  
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;     /* Enable parity */    
			options.c_cflag &= ~PARODD;   /* ת��ΪżЧ��*/     
			break;
		case 'S': 
		case 's':  /*as no parity*/   
		    options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
	        break;  
		default:   
			fprintf(stderr,"Unsupported parity\n");    
			return -1;  
	}  

	/* ����ֹͣλ*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
		   break;
		default:    
			 fprintf(stderr,"Unsupported stop bits\n");  
			 return -1; 
	} 

	/* Set rts/cts */ 
	if (RTSCTS)
	{
	    printf("Set rts/cts");
		options.c_cflag |= CRTSCTS;
	}

	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* ���ó�ʱ15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		printf("SetupSerial failed");   
		return -1;  
	} 
	// printf("set_Parity\n");
	return 0;  
}

//������Ϣ��β�ӻ��з�
int serial_set_line_input(int fd)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}

    options.c_lflag |= ICANON;

    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* ���ó�ʱ15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
    	perror("SetupSerial 3");   
    	return -1;  
    } 
    return 0;  
}

/**
*@brief  
*@param  port ���ڶ˿ں�
*@param  spd  �����ٶ�
*@param  databits,parity,stopbits,RTSCTS,�ֱ�Ϊ����λ,У��λ,ֹͣλ,rtsctsλ
*@param  need_line_input�������ݽ�β�Ƿ�ӻ��з�?
*/
int serial_init(int port, int spd, int databits, int parity, 
				int stopbits, int RTSCTS, int need_line_input)
{
    int fd;

	if(port < 13)
	{
   		// printf("open port:%d\n", port);
	}
	else {
		printf("error: port:%d > MAX_PORTS\n", port);
		return -1;
	}
												 //	   	                	
    fd = open(ports[port].name, O_RDWR|O_NOCTTY);//O_NONBLOCK ������, O_WRONLY ֻ��д, O_RDONLY ֻ��, O_RDWR ��д,O_NOCTTY ����

	if (-1 == fd) { 
        printf("init %s failed\n", ports[port].name);
        return -1;
    }

    set_speed(fd, spd);
	
    set_Parity(fd, databits, parity, stopbits, RTSCTS);
	
    if (need_line_input) {
		serial_set_line_input(fd);
    }

	ports[port].port_fd = fd;
	
    return fd;
}

/**
*@brief  
*@param  fd   ���ڶ˿ں��ļ�������
*@param  src  ��Ҫͨ�����ڷ��͵�����
*@param  len  ��Ҫ���͵����ݳ���
*@param  �ɹ�����0, ���򷵻�-1
*/
int serial_write(int fd, void *src, int len)
{
    int ret = write(fd, src, len);
	
    if (len != ret) {
		perror("oh, write serial failed!");
		return -1;
    }
    return 0;
}

/**
*@brief  
*@param  fd   ���ڶ˿ں��ļ�������
*@param  src  ���ڽ������ݵ�ָ��
*@param  len  ��Ҫ���յ����ݳ���
*@param  �ɹ�����0, ���򷵻�-1
*/
int serial_read(int fd, char *buf, int len)
{
    int ret = read(fd, buf, len-1);
    if (-1 == ret) {
		perror("oh, read serial failed!");
		return -1;
    }
    buf[ret] = '\0';
    return ret;
}

int serial_recv(int fd,char *rbuf,int rbuf_len, int timeout)
{      
    int retval;
    fd_set  rset;
    struct timeval time_out;
    if(!rbuf || rbuf_len <= 0)
    {  
        // printf("serial_recv Invalid parameter.\n");
        return -1;
    }  
    if(timeout) //ָ����ʱ�ȴ�
    {    
        time_out.tv_sec = (time_t)(timeout / 1000);
        time_out.tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(fd,&rset);

        retval = select(fd,&rset,NULL,NULL,&time_out);
        if(retval < 0)
        {
            // printf("%s,Select failed:%s\n",strerror(errno));
            return -2;
        }
        else if(0 == retval)
        {
            // printf("Time Out.\n");
            return 0;
        }

    }
    // usleep(1000);
    retval = read(fd, rbuf, rbuf_len);
    if( retval <= 0)
    {
        // printf("Read failed:%s\n",strerror(errno));
        return -3;
    }
    return retval;

} 
