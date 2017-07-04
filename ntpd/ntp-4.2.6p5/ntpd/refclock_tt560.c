/*
 * refclock_tt560 - clock driver for the TrueTime 560 IRIG-B decoder
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(REFCLOCK) && defined(CLOCK_TT560)

#include "ntpd.h"
#include "ntp_io.h"
#include "ntp_refclock.h"
#include "ntp_unixtime.h"
//#include "sys/tt560_api.h"
#include "ntp_stdlib.h"

#include <stdio.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/*
 * This driver supports the TrueTime 560 IRIG-B decoder for the PCI bus.
 */ 

/*
 * TT560 interface definitions
 */
#define	DEVICE		 "/dev/tt560%d" /* device name and unit */
#define	PRECISION	(-20)	/* precision assumed (1 us) */
#define	REFID		"IRIG"	/* reference ID */
#define	DESCRIPTION	"TrueTime 560 IRIG-B PCI Decoder"
#define UPDATE_MMAP_TIMECODE  21// add by hf  2014.9.12
#define GET_ALARM 14			/*get alarm*/

#define DCLS_BUF_SIZE 15

#define PAGE_SIZE 4096
#define TIME_LEAP_SHED (60)


/*
 * Unit control structure
 */

/*映射到上层的时间, add by hf*/
typedef struct 
{
    unsigned char  DCLS_buf[DCLS_BUF_SIZE];
    unsigned long uIsec;/*整数秒*/
    unsigned long uFsec;/*小数秒*/
}MapTimeFromUser,*PMapTimeFromUser;


struct s5unit {
	int size;
	MapTimeFromUser* s5rawt;	/*  */
};

typedef union byteswap_u
{
    unsigned int long_word;
    unsigned char byte[4];
} byteswap_t;



/*
 * Function prototypes
 */
static	int	tt560_start	(int, struct peer *);
static	void	tt560_shutdown	(int, struct peer *);
static	void	tt560_poll	(int unit, struct peer *);

static unsigned int byte_swap(unsigned int input_num);


/*
 * Transfer vector
 */
struct	refclock refclock_tt560 = {
	tt560_start,		/* clock_start    */
	tt560_shutdown,		/* clock_shutdown */
	tt560_poll,		/* clock_poll     */
	noentry,		/* clock_control (not used) */
	noentry,		/* clock_init    (not used) */
	noentry,		/* clock_buginfo (not used) */
	NOFLAGS			/* clock_flags   (not used) */
};



/***********************************************************************/
/*      将某年转换为天                                                 */
/*      返回值：计算结果  天数                                         */
/***********************************************************************/
unsigned int YearToDay(unsigned int year)
{
	unsigned int LeapYear;
	unsigned int OneYear;
	
	LeapYear = ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0);
	if( LeapYear ) 
	OneYear = 366;
	else
	OneYear = 365;
	return OneYear;
}



/*
 * tt560_start - open the TT560 device and initialize data for processing
 */
static int
tt560_start(
	int unit,
	struct peer *peer
	)
{
	register struct s5unit *up;
	struct refclockproc *pp;
	char device[20];
	int     fd;
    caddr_t membase;

	/*
	 * Open 5S PTP CPLD device
	 */

    memcpy(device,"/dev/PTP_cpld",sizeof("/dev/PTP_cpld"));
    
	//(void)sprintf(device, DEVICE, unit);            
	fd = open(device, O_RDWR); // modify bu hf

	if (fd == -1) {
		msyslog(LOG_ERR, "PTP_start: open of %s", device);
		return (0);
	}

	/*
	 * Map the device registers into user space.
	 */
	//membase = (caddr_t)malloc(TTIME_MEMORY_SIZE); //add by hf

    membase = (caddr_t)mmap(0,PAGE_SIZE, 
                          PROT_READ | PROT_WRITE, MAP_SHARED,fd,  (off_t)0);
    /*
	membase = mmap ((caddr_t) 0, TTIME_MEMORY_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, (off_t)0);
        */

    
	if (membase == (caddr_t) -1) {
		msyslog(LOG_ERR, "tt560_start: mapping of %s", device);
		(void) close(fd);
		return (0);
	}

	/*
	 * Allocate and initialize unit structure
	 */
	if (!(up = (struct s5unit*) emalloc(sizeof(struct s5unit)))) {
		(void) close(fd);
		return (0);
	}
	memset((char *)up, 0, sizeof(struct s5unit));
	up->s5rawt = (PMapTimeFromUser)membase;
    
	pp = peer->procptr;
	pp->io.clock_recv = noentry;
	pp->io.srcclock = (caddr_t)peer;
	pp->io.datalen = 0;
	pp->io.fd = fd;
	pp->unitptr = (caddr_t)up;

	/*
	 * Initialize miscellaneous peer variables
	 */
	peer->precision = PRECISION;
	peer->burst = NSTAGE;
	pp->clockdesc = DESCRIPTION;
	memcpy((char *)&pp->refid, REFID, 4);


    printf("tt560 start sucess .........\n");
	return (1);
}


/*
 * tt560_shutdown - shut down the clock
 */
static void
tt560_shutdown(
	int unit,
	struct peer *peer
	)
{
	register struct s5unit *up;
	struct refclockproc *pp;

	pp = peer->procptr;
	up = (struct s5unit *)pp->unitptr;
	io_closeclock(&pp->io);
	free(up);
}

static void 
tt560_input_time(MapTimeFromUser *pmap_data, char *pout_a_lastcode)
{
    unsigned char *DCLS_buf = pmap_data->DCLS_buf;
    unsigned int year = 0;
    unsigned int day = 0;
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    //unsigned int all_day = 0;
    unsigned long nsecond = 0;
    //unsigned int i;
    unsigned char timezone;
    
    second = (DCLS_buf[0]&0x0F) + ((DCLS_buf[0]>>5)*10) ;
    second += 1; /*补偿一秒*/
    
    minute = (DCLS_buf[1]&0x0F) + (((DCLS_buf[1]>>5)|((DCLS_buf[2]&0x01)<<3))*10);
   
    hour = ((DCLS_buf[2]>>1)&0x0F) + (((DCLS_buf[2]>>6)|((DCLS_buf[3]&0x03)<<2))*10);
   
    day = ((DCLS_buf[3]>>2)&0x0F) + (((DCLS_buf[3]>>7)|((DCLS_buf[4]&0x07)<<1))*10)   + ((DCLS_buf[4]&0x18)>>3)*100;
   
    year = (DCLS_buf[5]>>4) + (((DCLS_buf[6]&0x1e)>>1)*10)+2000;
 
    nsecond = (pmap_data->uFsec + 200000)* 1000;
 
    timezone = (DCLS_buf[7]&0x3C)>>2;
    printf("time z0ne %d\n",timezone);
    
    if((DCLS_buf[7]&0x02) == 0x02)
    {
        hour += timezone;
    }
    else
    {
        hour -= timezone;
    }
  
    sprintf(pout_a_lastcode,"%03d %02d:%02d:%02d.%9ld %x",day,hour,minute,second,nsecond,0x06);

}

static void 
tt560_check_time(MapTimeFromUser *pmap_data,struct refclockproc *pp)
{
 

}


/*
 * tt560_poll - called by the transmit procedure
 */
static void
tt560_poll(
	int unit,
	struct peer *peer
	)
{
	register struct s5unit    *up;
	struct refclockproc       *pp;
    MapTimeFromUser           *pMap;
    unsigned char             CPLD_alarm_reg;
	/*
	 * This is the main routine. It snatches the time from the TT560
	 * board and tacks on a local timestamp.
	 */
	pp = peer->procptr;
	up = (struct s5unit *)pp->unitptr;
    pMap = up->s5rawt;    

    /*控制驱动更新时间*/
    if(!ioctl(pp->io.fd,UPDATE_MMAP_TIMECODE,0))
    {
        printf("5s NTP ioctl error\n");
        return;
    }
    /*从内核映射得到时间，并保存到固定格式当中*/
	tt560_input_time(pMap,pp->a_lastcode);
    
    /*获得系统时间*/
	get_systime(&pp->lastrec);
	pp->polls++;

    pp->lencode = strlen(pp->a_lastcode);

#ifdef DEBUG
	if (debug)
		printf("tt560: time %s timecode %d %s\n",
		   ulfptoa(&pp->lastrec, 6), pp->lencode,
		   pp->a_lastcode);
#endif
    
	if (sscanf(pp->a_lastcode, "%3d %2d:%2d:%2d.%9ld", 
                  &pp->day, &pp->hour, &pp->minute, &pp->second, &pp->nsec)
	    != 5) {
		refclock_report(peer, CEVNT_BADTIME);

        //printf("1111111111111\n");
		return;
	}

    /*
    CPLD_alarm_reg = ioctl(pp->io.fd,GET_ALARM,0);
    printf("`````````````%x\n",CPLD_alarm_reg);
    if(((CPLD_alarm_reg & 0x40) == 0x40) && ((CPLD_alarm_reg & 0x80)== 0x80)
        || ((CPLD_alarm_reg & 0x10) == 0x10))
    {
        pp->leap = LEAP_NOTINSYNC;
    }
    else
    {
        pp->leap = LEAP_NOWARNING;
    }
    */
    pp->leap = LEAP_NOWARNING;
    /*leap flag*/
    /*
	if ((pMap->DCLS_buf[8] & 0x06) != 0x06)
		pp->leap = LEAP_NOTINSYNC;
	else
		pp->leap = LEAP_NOWARNING;
    */
	if (!refclock_process(pp)) {
		refclock_report(peer, CEVNT_BADTIME);
        //printf("2222222222222\n");
		return;
	}
	if (peer->burst > 0)
    {
        if (debug)
            printf("peer-> burst > 0\n");
		return;
    }

	if (pp->coderecv == pp->codeproc) 
    {
        //printf("444444444444444\n");
		refclock_report(peer, CEVNT_TIMEOUT);
		return;
	}
	record_clock_stats(&peer->srcadr, pp->a_lastcode);
	refclock_receive(peer);
    peer->burst = NSTAGE;

    if(debug)
        printf("DCLS update sys time success\n");
}

/******************************************************************
 *
 *  byte_swap
 *
 *  Inputs: 32 bit integer
 *
 *  Output: byte swapped 32 bit integer.
 *
 *  This routine is used to compensate for the byte alignment
 *  differences between big-endian and little-endian integers.
 *
 ******************************************************************/
static unsigned int
byte_swap(unsigned int input_num)
{
    byteswap_t    byte_swap;
    unsigned char temp;

    byte_swap.long_word = input_num;

    temp              = byte_swap.byte[3];
    byte_swap.byte[3] = byte_swap.byte[0];
    byte_swap.byte[0] = temp;

    temp              = byte_swap.byte[2];
    byte_swap.byte[2] = byte_swap.byte[1];
    byte_swap.byte[1] = temp;

    return (byte_swap.long_word);
}

#else
int refclock_tt560_bs;
#endif /* REFCLOCK */
