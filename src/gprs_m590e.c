
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>


#include "gprs_m590e.h"

#define GPRS_DBG(x...)                //  printf("GPRS: "x)

#define  GPRS_M590E_PORT                 "/dev/ttyO4"
#define  GPRS_M590E_LIDSC                25

#define GPRS_RECV_DATA  _IOW('h',0x0001,unsigned int)
#define GPRS_POWER_ENABLE _IOW('h', 0x0002, unsigned int)
#define GPRS_POWER_DISABLE _IOW('h', 0x0003, unsigned int)


static pthread_mutex_t gprs_mutex = PTHREAD_MUTEX_INITIALIZER;

static gprs_driver_api_t gprs_driver = {0};

struct GprsBaudRates {
    unsigned int rate;
    unsigned int cflag;
};

struct  GprsBaudRates GprsBaudrate[]={
    { 921600, B921600 },
    { 460800, B460800 },
    { 230400, B230400 },
    { 115200, B115200 },
    {  57600, B57600  },
    {  38400, B38400  },
    {  19200, B19200  },
    {   9600, B9600   },
    {   4800, B4800   },
    {   2400, B2400   },
    {   1200, B1200   },
    {      0, B38400  }
};


static int gprs_m590e_fd = -1;
static unsigned int GprsUartTimeOut = 0;

static void GprsM590eLock(void)
{
	pthread_mutex_lock(&gprs_mutex);
}

static void GprsM590eUnLock(void)
{
	pthread_mutex_unlock(&gprs_mutex);
}

static int GprsM590eOpen(void)
{
	GprsM590eLock();
    if (gprs_m590e_fd == -1)
        gprs_m590e_fd = open(GPRS_M590E_PORT,O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (gprs_m590e_fd < 0)
    {
        printf("GprsM590eOpen failed\n");
        gprs_m590e_fd = -1;
		GprsM590eUnLock();
        return 0; 
    }
	GprsM590eUnLock();
    return 1;
}

static int GprsM590eClose(void)
{
	GprsM590eLock();
    if (gprs_m590e_fd > 0)
    {
        close(gprs_m590e_fd);
        gprs_m590e_fd = -1;
    }
	GprsM590eUnLock();
    return 1;
}

static int GprsM590eIoctl(unsigned int cmd,unsigned long arg)
{
	
	if (ioctl(gprs_m590e_fd,cmd,arg) < 0)
	{
		printf("GprsM590eIoctl err\n");
		GprsM590eUnLock();
		return 0;
	}
	
	return 1;
}


static int GprsM590eSetOptions(enum GprsParityBits parity,unsigned int baudrate)
{
    struct termios opt;
    int i = 0;
    int status = 0;
    int databits = 8;
    int stopbit = 1;
	GprsM590eLock();
    if (gprs_m590e_fd < 0)
    {
        printf("GprsM590eSetOptions failed,The device isn't open\n");
		GprsM590eUnLock();
        return -1;
    }

    if(tcgetattr(gprs_m590e_fd,&opt) != 0)
    {
		GprsM590eUnLock();
        return -1;
    }

    for (i = 0;i < (sizeof(GprsBaudrate)/sizeof(GprsBaudrate[0]));i++)
    {
        if (baudrate == GprsBaudrate[i].rate)
        {
            tcflush(gprs_m590e_fd,TCIOFLUSH);
            //设置输入的波特率
            cfsetispeed(&opt, GprsBaudrate[i].cflag);
            //设置输出的波特率  
            cfsetospeed(&opt, GprsBaudrate[i].cflag);
            status = tcsetattr(gprs_m590e_fd, TCSANOW, &opt);
            if (status != 0)
            {
				GprsM590eUnLock();
                return -1;
            }
			
            tcflush(gprs_m590e_fd,TCIOFLUSH);
			GprsUartTimeOut = (unsigned int)((11 * 3*1000)/(baudrate) + 8);
		
            break;
        }
    }

    opt.c_cflag &= ~CSIZE;

   // opt.c_cflag |= CLOCAL | CREAD;
    switch(databits){
        case 7:
            opt.c_cflag |= CS7;
        break;

        case 8:
        default:
            opt.c_cflag |= CS8;
        break;
    }
    //opt.c_cflag &= ~(PARENB | PARODD);
    //opt.c_cflag &= ~CMSPAR;
    switch (parity)
    {
        case 'n':
        case 'N':
            opt.c_cflag &= ~PARENB;   /* Clear parity enable */
            opt.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            opt.c_cflag |= (PARODD | PARENB);
            opt.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            opt.c_cflag |= PARENB;     /* Enable parity */
            opt.c_cflag &= ~PARODD;
            opt.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            opt.c_cflag &= ~PARENB;
            opt.c_cflag &= ~CSTOPB;break;
        default:
            fprintf(stderr,"Unsupported parity\n");
			GprsM590eUnLock();
            return -1;
    }

    switch (stopbit)
    {
        case 1:
            opt.c_cflag &= ~CSTOPB;
            break;
        case 2:
            opt.c_cflag |= CSTOPB;
           break;
        default:
             fprintf(stderr,"Unsupported stop bits\n");
			 GprsM590eUnLock();
             return -1;
    }
    /* Set input parity option */
    if (parity != 'n')
        opt.c_iflag |= INPCK;
    tcflush(gprs_m590e_fd,TCIFLUSH);
    opt.c_cc[VTIME] = 0xff; /*设置超时时间为150秒*/
    opt.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    /*设置非终端模式,支持read,否则发送的时候就会自动接收回复*/
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
    opt.c_oflag &= ~OPOST; /*Output*/
    //opt.c_oflag &= CRTSCTS; //硬件流控
    //opt.c_cline = 0;
    opt.c_iflag &= ~( IXOFF );
    opt.c_iflag &= ~IXON;
//    opt.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    if (tcsetattr(gprs_m590e_fd,TCSANOW,&opt) != 0)
    {
      //  close(fd);
	  GprsM590eUnLock();
        return -1;
    }
	GprsM590eUnLock();
    //close(fd);
    return 1;
}


static int GprsM590eSetLine(void)
{
    int ldisc = GPRS_M590E_LIDSC;
	GprsM590eLock();
    if (gprs_m590e_fd < 0)
    {
        printf("GprsM590eSetLine error\n");
		GprsM590eUnLock();
        return 0;
    }
    if (ioctl(gprs_m590e_fd, TIOCSETD, &ldisc)) 
    {
        printf("GprsM590eSetLine failed\n");
		GprsM590eUnLock();
        return 0;
    }
	GprsM590eUnLock();
    return 1;
}

static int GprsM590eWrite(unsigned char *buffer,unsigned int length)
{
	
    if (gprs_m590e_fd < 0)
    {
        printf("GprsM590eWrite failed\n");
		
        return 0;
    }

    return write(gprs_m590e_fd,buffer,length);
}


static unsigned int GprsM590eRead(unsigned char *buffer,unsigned int length)
{
    unsigned int pos = 0;
    unsigned int index = 0;    
	int times = 0;
    if (gprs_m590e_fd < 0)
    {
        printf("GprsM590eRead failed\n");
        return 0;
    }
	
    while(1) 
    {
		read_times:
		pos = read(gprs_m590e_fd,&buffer[index],length - index);
		
		index += pos;
		if (pos > 0)
		{
			times = 4;
			if (index >= length)
				goto read_out;
			usleep(GprsUartTimeOut * 1000);
			goto read_times;
		}
		else {
			if (times > 0)
			{
				if (index >= length)
				goto read_out;
				usleep(GprsUartTimeOut * 1000 );
				times--;
				goto read_times;
			}
			goto read_out;
		}
	}
	
	read_out:

    return  index;
}

static unsigned int GprsM590eReadWithoutDelay(unsigned char *buffer,unsigned int length)
{
   
    if (gprs_m590e_fd < 0)
    {
        printf("GprsM590eRead failed\n");
        return 0;
    }
	return read(gprs_m590e_fd,buffer,length);

}

static int GprsGetCharIndex(char *str,char c)
{
	int i = 0;
	while(str[i] != '\0')
	{
		if(str[i] == c)
			return i;
		else
			i++;
	}
	if (str[i] == c)
		return i;
	else
		return -1;
}

//------------------GPRS 状态 --------------------------------
static int AT_EXP_OK(const unsigned char *RESP)
{
	if (RESP == NULL)
		return (0);
  
	if (strstr((char *)RESP, "OK") != NULL) {
		return (1);
	}
  
	return (0);
}

static int AT_EXP_READY(const unsigned char *RESP)
{
	if (RESP == NULL)
		return (0);
  
	if (strstr((char *)RESP, "READY") != NULL) {  
		return (1);
	}
  
	return (0);
}

static int AT_EXP_CSQ(const unsigned char *RESP)
{
	if (RESP == NULL)
		return (0);
 
	if (strstr((char *)RESP, "+CSQ: 99,99") != NULL) {
    
		return (0);
	}
	else if (strstr((char *)RESP, "+CSQ:") != NULL)
	{
		return (1);
	}
	return (0);
}

static int AT_EXP_CREG(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
	if ( (strstr((char *)RESP, "+CREG: 0,1") != NULL) ||  
       (strstr((char *)RESP, "+CREG: 0,5") != NULL) )
	{
		return (1);
	}
  
	return (0);
}

static int AT_EXP_CGATT(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
	if (strstr((char *)RESP,"+CGATT: 1") != NULL) {
    
		return (1);
	}
  
	return (0);
}

static int AT_EXP_IPSTATUS(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
	if (strstr((char *)RESP,"+IPSTATUS:") != NULL) {
    
		return (1);
	}
  
	return (0);
}

static int AT_EXP_XIIC(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
	if (strstr((char *)RESP, "+XIIC:    1") != NULL) {
    
		return (1);
	}
  
	return (0);
}

static int AT_EXP_CONNECT(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
	if (strstr((char *)RESP, "2000") != NULL) {
    
		return (1);
	}
  
	return (0);
}

static int AT_EXP_TCPSETUP(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
	if (strstr((char *)RESP, "+TCPSETUP:0,OK") != NULL) {
    
		return (1);
	}
  
	if (strstr((char *)RESP, "+TCPSETUP:1,OK") != NULL) {
    
		return (1);
	}
  
	if (strstr((char *)RESP, "+TCPSETUP:2,OK") != NULL) {
     
		return (1);
	}
  
	if (strstr((char *)RESP, "+TCPSETUP:3,OK") != NULL) {
    
		return (1);
	}
  
	return (0);
}

static int AT_EXP_TCPSND(const unsigned char *RESP)
{
	if (RESP == NULL) {
    
		return (0);
	}
  
#ifdef AT_CMD_SGCC
	if (strstr((char *)RESP, "$MYNETWRITE:") != NULL) {
    
		return (1);
	}
#else 
	if (strstr((char *)RESP, ">") != NULL) {
    
		return (1);
	}
#endif
  
	return (0);
}


//--------------------end 状态-------------------------------

static int IsDigital(char n)
{
	int ret = 0;
	
	if ((n >= '0') && (n <= '9'))
	{
		ret = 1;
	}
	return ret;
}

static unsigned int GprsGetReadNumber(const unsigned char *str,unsigned int *size)
{
	unsigned int k = 0;
	unsigned char buffer[20]={0};
	if(str == NULL)
		return 0;
	while((str != NULL) && (IsDigital(*str)))
	{
		buffer[k++] = *str++;
	}
	*size = atoi((char*)buffer);
	
	return k;
}

static unsigned int GprsGetReadDigitalNumber(const unsigned char *str,unsigned int *size)
{
	unsigned int k = 0;
	unsigned char buffer[20]={0};
	if(str == NULL)
		return 0;
	while(str != NULL)
	{
		if ( IsDigital(*str))
		buffer[k++] = *str++;
		else
		break;
	}
	*size = atoi((char*)buffer);
	
	return k;
}

static int GprsSendAT(void)
{
	int count = 0;
	int timeout = 400;//ms
	unsigned char buffer[128]={0};
	
	unsigned int index = 0;
	int ret = 0;
	GprsM590eLock();
	
	GprsM590eWrite((unsigned char*)"AT\r",strlen("AT\r"));
	
	for (count = 0; count < timeout;count++)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],128 - index);
		if ((AT_EXP_READY(buffer) == 1) || (index >= 128))
		{
			ret = 1;
			break;
		}
		usleep(5000);
	}
	
	GPRS_DBG("at=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//获取厂商信息
static int GprsGetVendorInformation(gprs_vendor_t *vendor)
{
	int count = 0;
	int timeout = 400;//ms
	unsigned char buffer[128]={0};
	
	unsigned int index = 0;
	int ret = 0;
	int pos = 0;
	int old_pos = 0;
	GprsM590eLock();
	memset((void*)vendor,0x0,sizeof(vendor));
	
	GprsM590eWrite((unsigned char*)"ATI\r",strlen("ATI\r"));
	
	for (count = 0; count < timeout;count++)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],128 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 128))
		{
			ret = 1;
			break;
		}
		usleep(5000);
	}
	if (ret)
	{
		for (count = 0;count < 4;count++)
		{
			pos += GprsGetCharIndex((char*)&buffer[old_pos],'\n');
			
			if (count == 1) {
				strncpy((char*)vendor->name,(const char*)&buffer[old_pos],6);
				strcat((char*)&vendor->name[6],"\0");
				pos += 1;
			}
			else if(count == 2) {
				strncpy((char*)vendor->module,(const char*)&buffer[old_pos],7);
				strcat((char*)&vendor->module[7],"\0");
				pos += 1;
			}
			else if(count == 3){
				strncpy((char*)vendor->version,(const char*)&buffer[old_pos],13);
				strcat((char*)&vendor->version[13],"\0");
				pos += 1;
			}
			old_pos = pos+1;
		}
	}

	GPRS_DBG("%s,%s,%s\n",vendor->name,vendor->module,vendor->version);
	GprsM590eUnLock();
	return ret;
}

//获取软件版本
static int GprsGetSoftWareVersion(gprs_software_ver_t *ver)
{
	int count = 0;
	unsigned int index = 0;
	unsigned char buffer[60]={0};
	int ret = 0;
	int pos = 0;
	char *p;
	GprsM590eLock();
	GprsM590eWrite((unsigned char*)"AT+GMR\r",strlen("AT+GMR\r"));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],60 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 60))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	if (ret)
	{
		p = strstr((char*)buffer,"+GMR: ");
		if(p != NULL){
			pos = p - (char*)buffer;
			memcpy((void*)ver->ver,(const void*)&buffer[pos+6],23);
			ver->ver[23] = '\0';
			GprsM590eUnLock();
			return 1;
		}
	}
	GprsM590eUnLock();
	return 0;
}

static int GprsSignalToRssi(int signal)
{
	if ((signal < 4) || (signal >= 99))
		return 0;
	else if (signal < 10)
		return 1;
	else if(signal < 16)
		return 2;
	else if (signal < 22)
		return 3;
	else if (signal < 28)
		return 4;
	else
		return 5;
}

//获取信号强度
static int GprsGetSignalCSQ(gprs_csq_t *csq)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	char *p;
	int pos = 0;
	char tmp[3];
	//int pos1 = 0;
	GprsM590eLock();
	GprsM590eWrite((unsigned char*)"AT+CSQ\r",strlen("AT+CSQ\r"));
	
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	if (ret) 
	{
		ret = 0;
		if (AT_EXP_CSQ(buffer) == 1)
		{
			p = strstr((const char*)buffer,"+CSQ: ");
			if (p != NULL)
			{
				pos = p - (char*)buffer;
				//pos1 = GprsGetCharIndex((char*)&buffer[pos + 6],',');
				tmp[0] = (char)buffer[pos+6];
				tmp[1] = (char)buffer[pos+7];
				tmp[2] = '\0';
				csq->signal = atoi((const char*)tmp);
				csq->rssi = GprsSignalToRssi(csq->signal);
				tmp[0] = (char)buffer[pos+9];
				tmp[1] = (char)buffer[pos+10];
				tmp[2] = '\0';
				csq->ber = atoi(tmp);
				ret = 1;
			}
		}
	}
	
	
	GPRS_DBG("CSQ=%d,%d,%d\n",csq->signal,csq->ber,csq->rssi);
	GprsM590eUnLock();
	return ret;
}


//获取GPRS注册状态 0:未注册,1：注册 ,GSM状态
static int GprsGetCREGStatus(void)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	GprsM590eWrite((unsigned char*)"AT+CREG?\r",strlen("AT+CREG?\r"));
	
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	if (ret)
	{
		ret = 0;
		if(AT_EXP_CREG(buffer) == 1)
			ret = 1;
	}
	GPRS_DBG("%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//查询sim PIN码状态 ,1 :ready,
static int GprsGetGSMCPIN(void)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	GprsM590eWrite((unsigned char*)"AT+CPIN?\r",strlen("AT+CPIN?\r"));
	
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	if (ret)
	{
		if(AT_EXP_READY(buffer))
			ret = 1;
		else 
			ret = 0;
	}
	GPRS_DBG("CPIN=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//模块状态查询,比如没插卡
static int GprsGetGSMCPAS(void)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	GprsM590eWrite((unsigned char*)"AT+CPAS=?\r",strlen("AT+CPAS=?\r"));
	
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("CPAS=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//设置GPRS PDP工作模式(APN) 1:设置ok
static int GprsSetPDP_APN(gprs_pdp_apn_t *apn)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	index = snprintf((char*)buffer,sizeof(buffer),"AT+CGDCONT=%d,\"%s\",\"%s\"\r",apn->cid,apn->type,apn->apn);
	
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("APN=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//获取GPRS状态
static int GprsGetCGATT(void)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	index = snprintf((char*)buffer,sizeof(buffer),"AT+CGATT?");
	GprsM590eWrite(buffer,index);
	index = 0;
    memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("CGATT=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//设置GPRS附着 ,1 :enable
static int GprsSetCGATT(bool state)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	index = snprintf((char*)buffer,sizeof(buffer),"AT+CGATT=%d\r",state);
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("SetCGATT=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//获取SIM 卡序列号
static int GprsGetCCID(void)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYCCID\r");
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+CCID\r");
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("CCID=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//主动上报开关
static int GprsMYNETURC(bool onoff)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETURC=%d\r",onoff);
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+XISP=0\r");
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("NETURC=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//设置网络链接初始化参数
static int GprsSetMYNETCON(gprs_pdp_apn_t *apn)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETCON=%d,\"APN\",\"%s\"\r",apn->cid,apn->apn);
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+CGDCONT=%d,\"%s\",\"%s\"\r",apn->cid,apn->type,apn->apn);
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("NETCON=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//设置pwd type
static int GprsSetUserAUTH_Type(auth_t *auth)
{
	int count = 0;
	int ret = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETCON=%d,\"AUTH\",%d\r",auth->cid,auth->auth);
	
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("PWD=%s\n",buffer);
	#endif
	GprsM590eUnLock();
	return ret;
}

//设置pwd 
static int GprsSetUserPassWord(auth_t *auth)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();

	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETCON=%d,\"USERPWD\",\"%s,%s\"\r",auth->cid, auth->name, auth->psw);
	#else
	index = snprintf((char*)buffer,sizeof(buffer),"AT+XGAUTH=%d,%d,\"%s\",\"%s\"\r", auth->cid, auth->auth, auth->name, auth->psw);
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("PWD=%s\n",buffer);
	
	GprsM590eUnLock();
	return ret;
}

//激活网络链接,PPP链接,激活第几通道
static int GprsSetMYNETACT(gprs_tcp_ip_t *tcp)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETACT=%d,1\r",tcp->channel);
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+XIIC=1\r");
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("NETACT=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

// 去激活网络链接,PPP链接,激活第几通道
static int GprsSetMYNETACTClose(gprs_tcp_ip_t *tcp)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETACT=%d,0\r",tcp->channel);
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+XIIC=0\r");
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("NETACT Close=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//获取网络链接状态
static int GprsGetMYNETACT(gprs_tcp_status_t *status)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	char *p = NULL;
	int pos = 0;
	int pos1 = 0;
	
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETACT?\r");
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+XIIC?\r");
	#endif
	memset(status,0x0,sizeof(gprs_tcp_status_t));
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			
			p = strstr((char*)buffer,"$MYNETACT: ");
			if (p != NULL)
			{
				pos = p - (char*)buffer;
				pos += strlen("$MYNETACT: ");
				pos1 = GprsGetReadDigitalNumber(&buffer[pos],(unsigned int)&status->channel);
				pos = pos + pos1;
				pos += 1; // ,
				GprsGetReadDigitalNumber(&buffer[pos],(unsigned int)&status->status);
				ret = 1;
			}
			
			break;
		}
		usleep(5000);
		count++;
	}
	
	GPRS_DBG("GNETACT=%s,chn =%d,st=%d\n",buffer,status->channel,status->status);
	GprsM590eUnLock();
	return ret;
}


//设置TCP/IP服务参数
static int GprsSetTcpIpMYNETSRV(gprs_tcp_ip_t *tcp)
{
	int count = 0;
	int ret = 0;
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETSRV=%d,%d,%d,%d,\"%s:%s\"\r", tcp->channel, tcp->sockid, tcp->nettype, tcp->mode, tcp->ip, tcp->port);
	#else 
	index = snprintf((char*)buffer,sizeof(buffer),"AT+TCPSETUP=%d,%s,%s\r", tcp->channel, tcp->ip, tcp->port);
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_TCPSETUP(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("MYNETSRV=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//开启TCP/UDP服务
static int GprsSetTcpIpMYNETOPEN(gprs_tcp_ip_t *tcp)
{
	int count = 0;
	int ret = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETOPEN=%d\r",tcp->sockid);
	
	
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_CONNECT(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("NETOPEN=%s\n",buffer);
	#endif
	GprsM590eUnLock();
	return ret;
}

static int GprsIsConnectNormal(const unsigned char *rcvbuf)
{
 
#ifdef AT_CMD_SGCC
  
  if (strstr((char *)rcvbuf, "$MYURCCLOSE:") != NULL) 
#else  
  if (strstr((char *)rcvbuf, "+TCPCLOSE:") != NULL) 
#endif
  {

    return (0);
  }

  return (1);
  
}

static int GprsStateIsReceived(const unsigned char *rcvbuf)
{
#ifdef AT_CMD_SGCC
  
  if (strstr((char *)rcvbuf, "$MYURCREAD:") != NULL) 
#else
    
  if (strstr((char *)rcvbuf, "+TCPRECV:") != NULL) 
#endif
  {
    
    return (1);
  }
  
  return (0);
}



static unsigned int GprsReadAccess(gprs_tcp_ip_t *tcp,unsigned char *rcvbuf,unsigned int bufferSize)
{
  unsigned int k = 0;
  unsigned int size = 0;
  unsigned int num = 0;
  int cmdlength = 0;
  unsigned char cmd[25] = "";
  int count = 0;
  char *p = NULL;
  
#ifdef AT_CMD_SGCC
  char *s = "$MYNETREAD: ";
  
  if (rcvbuf == NULL) {
    return (0);
  }
  
 // k = sprintf(cmd,"%s","AT$MYNETREAD=1,2048\r");
  k = snprintf((char*)cmd,sizeof(cmd),"AT$MYNETREAD=%d,2048\r",tcp->sockid);
  cmdlength = k;
  //"\r\n"
  cmdlength += 2;
  GprsM590eWrite(cmd, k);
  k = 0;
  while(count < 800)
  {
	k += GprsM590eRead(rcvbuf,bufferSize);
	if ((p = strstr((char *)rcvbuf, s)) != NULL)
	{
		//printf("read\n");
		 p += strlen(s);
		/*  link  */
		p++;
		if (*p == ',')
		{
		  p++;
		} 
		else {
		  return (0);
		}
		num = GprsGetReadNumber((unsigned char*)p,&size);
		//printf("size=%d\n",size);
		cmdlength += strlen(s);
		cmdlength += num;
		 //"\r\n"
		cmdlength += 2;
		//"\r\n"
		cmdlength += 2;
		//"OK\r\n"
		cmdlength += 4;
		
		if ((k >= bufferSize) || (k >= (size + cmdlength))){
			if (size > bufferSize)
				size = bufferSize;
			p += (num +2);
			
			memcpy((void*)rcvbuf,p,size);
			
			return size;
		}
	}
	
	count++;
	usleep(5000);
  }
  
  return (0);
#else 
  char *s = "+TCPRECV:";

  return (0);
#endif
 
}

//读取数据
static int GprsTcpReceiveData(gprs_tcp_ip_t *tcp,unsigned char *recvBuffer,unsigned int bufferSize,unsigned int *RecvLength)
{
	
	unsigned int length = 0;
	GprsM590eLock();
	GprsM590eIoctl(GPRS_RECV_DATA,1);
	memset(recvBuffer,'\0',bufferSize);
	
	*RecvLength = 0;
	length = GprsM590eRead(recvBuffer,bufferSize);
	if (length > 0)
	{
	
		if(GprsIsConnectNormal(recvBuffer) == 0)
		{
			//printf("GprsIsConnectNormal\n");
			GprsM590eIoctl(GPRS_RECV_DATA,0);
			GprsM590eUnLock();
			return -1;
		}
		if (GprsStateIsReceived(recvBuffer) == 0)
		{
			GprsM590eIoctl(GPRS_RECV_DATA,0);
			GprsM590eUnLock();
			return 0;
		}
		#ifdef AT_CMD_SGCC
		memset(recvBuffer,'\0',bufferSize);
		*RecvLength = GprsReadAccess(tcp,recvBuffer,bufferSize);
		GPRS_DBG("recv=%lu\n",(unsigned long)*RecvLength);
		#else
		*RecvLength = GprsReadAccess(tcp,recvBuffer,length);
		#endif
		GprsM590eIoctl(GPRS_RECV_DATA,0);
		GprsM590eUnLock();
        return *RecvLength;
	}
	GprsM590eIoctl(GPRS_RECV_DATA,0);
	GprsM590eUnLock();
	return 0;
}

//发送数据
static int GprsTCPSendData(gprs_tcp_ip_t *tcp,unsigned char *sendbuf,unsigned int sendLength)
{
	int count = 0;
	int ret = 0;
	
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETWRITE=%d,%d\r",tcp->sockid,sendLength);
	#else
	index = snprintf((char*)buffer,sizeof(buffer),"AT+TCPSEND=%d,%d\r",tcp->sockid,sendLength);
	#endif
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_TCPSND(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	if (ret)
	{
		GprsM590eWrite(sendbuf,sendLength);
		index = 0;
		count = 0;
		ret = 0;
		memset(buffer,0x0,sizeof(buffer));
		while(count < 400)
		{
			index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
			if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
			{
				ret = 1;
				break;
			}
			usleep(5000);
			count++;
		}
	}
	GprsM590eUnLock();
	return ret;
}

//关闭TCP链接
static int GprsSetTcpIpClose(gprs_tcp_ip_t *tcp)
{
	int count = 0;
	int ret = 0;
	
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	#ifdef AT_CMD_SGCC
	index = snprintf((char*)buffer,sizeof(buffer),"AT$MYNETCLOSE=%d\r",tcp->sockid);
	#else
	index = snprintf((char*)buffer,sizeof(buffer),"AT+TCPCLOSE=%d\r",tcp->sockid);
	#endif
	
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if ((AT_EXP_OK(buffer) == 1) || (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("TCP CLOSE=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//查询TCP/UDP链路状态
static int GprsGetTcpUdpLinkStatus(void)
{
	int count = 0;
	int ret = 0;
	
	unsigned char buffer[2048] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	index = snprintf((char*)buffer,sizeof(buffer),"AT+IPSTATUS=0\r");
	
	
	GprsM590eWrite(buffer,index);
	index = 0;
	memset(buffer,0x0,sizeof(buffer));
	while(count < 400)
	{
		index += GprsM590eReadWithoutDelay(&buffer[index],2048 - index);
		if (/*(AT_EXP_IPSTATUS(buffer) == 1) ||*/ (index >= 2048))
		{
			ret = 1;
			break;
		}
		usleep(5000);
		count++;
	}
	
	
	GPRS_DBG("LinkStatus=%s\n",buffer);
	GprsM590eUnLock();
	return ret;
}

//接受侦听请求

//查询接受侦听请求状态

//关闭ppp链接
static int GprsSetPPPClose(void)
{
	
	int ret = 0;
	
	unsigned char buffer[200] = {0};
	unsigned int index = 0;
	GprsM590eLock();
	index = snprintf((char*)buffer,sizeof(buffer),"AT+XIIC=0\r");

	GprsM590eWrite(buffer,index);
	GprsM590eUnLock();
	return ret;
}



void GprsDriverInit(void)
{
	gprs_driver_api_t *gprs = &gprs_driver;
	memset((void*)gprs,0x0,sizeof(gprs_driver_api_t));
	
	gprs->Open  = GprsM590eOpen;
	gprs->Close = GprsM590eClose;
	gprs->SetOptions = GprsM590eSetOptions;
	gprs->SetLine = GprsM590eSetLine;
	gprs->SendAtCmd = GprsSendAT;
	gprs->GetVendorInfo = GprsGetVendorInformation;
	gprs->GetSoftVersion = GprsGetSoftWareVersion;
	gprs->GetSignalCSQ = GprsGetSignalCSQ;
	gprs->GetRegisterStatus = GprsGetCREGStatus;
	gprs->GetSimPinStatus = GprsGetGSMCPIN;
	gprs->GetGsmCpas = GprsGetGSMCPAS;
	gprs->SetPdpApn = GprsSetPDP_APN;
	gprs->GetCGATT = GprsGetCGATT;
	gprs->SetCGATT = GprsSetCGATT;
	gprs->GetCCID = GprsGetCCID;
	gprs->SetNetURC = GprsMYNETURC;
	gprs->SetNetCON = GprsSetMYNETCON;
	gprs->SetAuthType = GprsSetUserAUTH_Type;
	gprs->SetPassWord = GprsSetUserPassWord;
	gprs->SetNetACT = GprsSetMYNETACT;
	gprs->CloseNetAct = GprsSetMYNETACTClose;
	gprs->GetNetActStatus = GprsGetMYNETACT;
	gprs->SetTcpIpNetSrv = GprsSetTcpIpMYNETSRV;
	gprs->SetTcpIpNetOpen = GprsSetTcpIpMYNETOPEN;
	gprs->TcpIpClose = GprsSetTcpIpClose;
	gprs->GetTcpUdpLinkStatus = GprsGetTcpUdpLinkStatus;
	gprs->PPPClose = GprsSetPPPClose;
	gprs->Send = GprsTCPSendData;
	gprs->Receive = GprsTcpReceiveData;
	
}


gprs_driver_api_t *getGprsDriverContext(void)
{
	return &gprs_driver;
}

pthread_t senddata_thread_id, recv_thread_id;

#if 0
void GprsTest(void)
{
    unsigned char buffer[600];
    unsigned int length = 0;
    unsigned int i = 0;
	gprs_vendor_t vendor={0};
	
	gprs_software_ver_t version;
	gprs_csq_t csq;
	gprs_pdp_apn_t apn;
	
	gprs_tcp_ip_t tcp;
	tcp.channel = TCP_CHN_1;
	tcp.sockid = TCP_SOCKID_1;
	tcp.nettype = TCP_Client;
	tcp.mode = TCP_TRANS_HEX;
	sprintf((char*)tcp.ip,"%s","116.24.23.232");
	sprintf((char*)tcp.port,"%s","19998");
	
    GprsM590eOpen();
    GprsM590eSetOptions(GPRS_PBIT_n,9600);
    GprsM590eSetLine();
	
	GprsSendAT();
	GprsGetVendorInformation(&vendor);
	
	GprsGetSoftWareVersion(&version);
	//printf("ver=%s\n",version.ver);
	
	GprsGetSignalCSQ(&csq);
	
	GprsGetCREGStatus();

	GprsGetGSMCPIN();
	GprsGetCCID();
	GprsGetGSMCPAS();

	apn.cid = 1;
	sprintf(apn.type,"%s","IP");
	sprintf(apn.apn,"%s","CMNET");
	GprsSetPDP_APN(&apn);
	//GprsSetCGATT(0);
	//GprsGetCGATT();
	GprsMYNETURC(1);
	GprsGetMYNETACT();
	GprsSetMYNETACT(&tcp);
	GprsSetMYNETCON(&apn);
	auth_t auth;
	auth.cid = 1;
	auth.auth = AUTH_PAP;
	sprintf(auth.name,"%s","user");
	sprintf(auth.psw,"%s","123456");
	GprsSetUserAUTH_Type(&auth);
	//GprsSetUserPassWord(&auth);
	
	GprsSetTcpIpMYNETSRV(&tcp);
	GprsSetTcpIpMYNETOPEN(&tcp);
	
	
	while(1)
	{
		if(GprsTcpReceiveData(&tcp,buffer,sizeof(buffer)/sizeof(buffer[0]),&length))
		{
			if (length > 0)
			{
				GprsTCPSendData(&tcp,buffer,length);
				length = 0;
			}
		}
		
	}
   /* while (1) 
    {
        length = GprsM590eRead(buffer,sizeof(buffer)/sizeof(buffer[0]));
        if (length > 0)
        {
			printf("length=%d\n",length);
			//for(i = 0;i < length;i++)
            printf("rx:%s\n",buffer);
            length = 0;
        }
//        GprsM590eWrite("gprs test",9);
  //      sleep(1);  
    }
	*/
	GprsSetTcpIpClose(&tcp);
    GprsM590eClose();
	
}
#else
	
static void *recv_thread(void *arg)
{
	gprs_tcp_ip_t tcp;
	gprs_driver_api_t *gprs = NULL;
	int ret = 0;
	gprs = getGprsDriverContext();
	unsigned char Buffer[2050] = {0};
	unsigned int  Length = 0;
	tcp.channel = TCP_CHN_1;
	tcp.sockid = TCP_SOCKID_1;
	tcp.nettype = TCP_Client;
	tcp.mode = TCP_TRANS_HEX;
	sprintf((char*)tcp.ip,"%s","116.24.217.33");
	sprintf((char*)tcp.port,"%s","19998");
	while(1)
	{
		if (gprs->Receive)
			ret = gprs->Receive(&tcp,Buffer,sizeof(Buffer)/sizeof(Buffer[0]),&Length);
		if ((ret) && (Length > 0))
		{
			if (gprs->Send)
				gprs->Send(&tcp,Buffer,Length);
			ret = 0;
			Length = 0;
			
		}
	}
	return 0;
}

static void *getstatus_thread(void *arg)
{
	gprs_csq_t csq = {0};
	gprs_tcp_status_t st ={0};
	while(1)
	{
		GprsGetSignalCSQ(&csq);
		printf("csq.rssi=%d,csq.signal=%d\n",csq.rssi,csq.signal);
		GprsGetMYNETACT(&st);
		printf("st.channel=%d,st.status=%d\n",st.channel,st.status);
		memset(&csq,0x0,sizeof(gprs_csq_t));
		memset(&st,0x0,sizeof(gprs_tcp_status_t));
		sleep(3);
	}
	
	return 0;
}

void GprsTest(void)
{
	unsigned char Buffer[2050] = {0};
	unsigned int  Length = 0;
	int ret = 0;
	gprs_driver_api_t *gprs = NULL;
	GprsDriverInit();
	gprs = getGprsDriverContext();
	gprs_tcp_status_t st = {0};
	gprs_vendor_t vendor={0};
	gprs_software_ver_t version;
	gprs_csq_t csq;
	gprs_pdp_apn_t apn;
	gprs_tcp_ip_t tcp;
	auth_t auth;
	
	if (gprs == NULL)
		return;
	
	tcp.channel = TCP_CHN_1;
	tcp.sockid = TCP_SOCKID_1;
	tcp.nettype = TCP_Client;
	tcp.mode = TCP_TRANS_HEX;
	sprintf((char*)tcp.ip,"%s","116.24.217.33");
	sprintf((char*)tcp.port,"%s","19998");
	
	apn.cid = 1;
	sprintf(apn.type,"%s","IP");
	sprintf(apn.apn,"%s","CMNET");
	
	auth.cid = 1;
	auth.auth = AUTH_PAP;
	sprintf(auth.name,"%s","user");
	sprintf(auth.psw,"%s","123456");
	
	if (gprs->Open)
		gprs->Open();
	if (gprs->SetOptions)
		gprs->SetOptions(GPRS_PBIT_n,9600);
	if (gprs->SetLine)
		gprs->SetLine();
	if (gprs->SendAtCmd)
		gprs->SendAtCmd();
	if (gprs->GetVendorInfo)
		gprs->GetVendorInfo(&vendor);
	if (gprs->GetSoftVersion)
		gprs->GetSoftVersion(&version);
	if (gprs->GetSignalCSQ)
		gprs->GetSignalCSQ(&csq);
	if (gprs->GetRegisterStatus)
		gprs->GetRegisterStatus();
	if (gprs->GetSimPinStatus)
		gprs->GetSimPinStatus();
	if (gprs->GetCCID)
		gprs->GetCCID();
	if (gprs->GetGsmCpas)
		gprs->GetGsmCpas();
	
	if (gprs->SetPdpApn)
		gprs->SetPdpApn(&apn);
	if (gprs->SetNetURC)
		gprs->SetNetURC(1); // ON
	if (gprs->GetNetActStatus)
		gprs->GetNetActStatus(&st);
	if (gprs->SetNetACT)
		gprs->SetNetACT(&tcp);
	if (gprs->SetNetCON)
		gprs->SetNetCON(&apn);
	if (gprs->SetAuthType)
		gprs->SetAuthType(&auth);
	
	if (gprs->SetTcpIpNetSrv)
		gprs->SetTcpIpNetSrv(&tcp);
	
	if (gprs->SetTcpIpNetOpen)
		gprs->SetTcpIpNetOpen(&tcp);
	
	printf("==============>Client<==============\n");
	pthread_create(&senddata_thread_id, NULL, getstatus_thread, (void *)NULL);
	pthread_create(&recv_thread_id, NULL, recv_thread, (void *)NULL);
	while(1)
	{
		/*if (gprs->Receive)
			ret = gprs->Receive(&tcp,Buffer,sizeof(Buffer)/sizeof(Buffer[0]),&Length);
		if ((ret) && (Length > 0))
		{
			if (gprs->Send)
				gprs->Send(&tcp,Buffer,Length);
			ret = 0;
			Length = 0;
			
		}*/
	}
	printf("==============>end<==============\n");
	if (gprs->TcpIpClose)
		gprs->TcpIpClose(&tcp);
	if (gprs->Close)
		gprs->Close();
	 pthread_join(senddata_thread_id, NULL);
	pthread_join(recv_thread_id, NULL);
	
}
#endif



/// add by wd
void gprs_module_enable(void) /// give a interface to use;
{
	;
}

void gprs_module_disable(void) //// give a interface to use;
{
	;
}