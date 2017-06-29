#include "manager.h"


#define CTL_WORD_DISCOVERY  0x10
#define CTL_WORD_RESPONSE   0x11
#define CTL_WORD_REQ        0x05
#define CTL_WORD_ACK        0x06
#define CTL_WORD_DATA       0x07
#define CTL_WORD_SET        0x08
#define CTL_WORD_NAK        0x15

#define HEAD_FRAME_LENGTH   12
#define SEND_BUFF_LENGTH    1500


#define CMD_HEART_BEAT          0x0000
#define CMD_DEV_INFORMATION     0x0001
#define CMD_NET_WORK_ADDRESS    0x0002
#define CMD_VERSION_INFROMATION 0x0003
#define CMD_GPS_STATUS          0x0004
#define CMD_SYS_SET             0x0005

#define CMD_PTP_CFG_ALL         0x0010
#define CMD_PTP_CFG_NORMAL      0x0011
#define CMD_PTP_CFG_SLAVE       0x0012
#define CMD_PTP_CFG_MASTER      0x0013
#define CMD_PTP_CFG_UNICAST     0x0014

#define CMD_NTP_CFG_NORMAL      0x0020
#define CMD_NTP_CFG_MD5_ENABLE  0x0021
#define CMD_NTP_CFG_MD5_KEY     0x0022
#define CMD_NTP_CFG_BLACKLIST   0x0023
#define CMD_NTP_CFG_WHITELSIT   0x0024


struct Head_Frame
{
    char h1;
    char h2;
    char saddr;
    char daddr;
    short index;
    char ctype;
    char pad_type;
    int length;
};


struct Discovery_Frame
{
    char h1;
    char h2;
    char saddr;
    char daddr;
    short index;
    char ctype;
    char end1;
    char end2;
};

struct Response_Frame
{
    int ipaddr;
    int port;
};

struct Req_Frame
{
    short cmd;
};

void msgUpPackHead(char *buf,struct Head_Frame *pHead)
{
    struct Head_Frame *ptmpHead = (struct Head_Frame *)buf;
    pHead->h1 = ptmpHead->h1;
    pHead->h2 = ptmpHead->h2;
    pHead->saddr = ptmpHead->saddr;
    pHead->daddr = ptmpHead->daddr;
    pHead->index = ntohs(ptmpHead->index);
    pHead->ctype = ptmpHead->ctype;
    pHead->pad_type = ptmpHead->pad_type;
    pHead->length = ntohl(ptmpHead->length);
}


void msgPackHead(struct Head_Frame *oHead,char saddr,char daddr,short index
    ,char ctype,char pad_type,int length)
{
    oHead->h1 = '$';
    oHead->h2 = '<';
    oHead->saddr =  saddr;
    oHead->daddr = daddr;
    oHead->index = htons(index);
    oHead->ctype = ctype;
    oHead->pad_type = pad_type;
    oHead->length = htonl(length);
}

int msgPackFrame(char *buf,struct Head_Frame *iHead,void *sendMsg,int msglen)
{
    int iOffset = 0;
    memcpy(buf+iOffset,iHead,sizeof(struct Head_Frame));
    iOffset += sizeof(struct Head_Frame);
    memcpy(buf+iOffset,sendMsg,msglen);
    iOffset += msglen;
    buf[iOffset++] = 0x0d;
    buf[iOffset++] = 0x0a;

    return iOffset;
}

void handle_discovery_message(struct root_data *pRootData,char *buf,int len)
{
    struct Head_Frame msgHead;
    struct Response_Frame responseMsg;
    char sendBuf[SEND_BUFF_LENGTH];
    int sendlen;
    
    if(buf[0] != '$' && buf[1] != '<')
        return;

    if(buf[6] != CTL_WORD_DISCOVERY)
        return;

    struct Discovery_Frame *pDiscoveryFrame = (struct Discovery_Frame *)buf;
    
    msgPackHead(&msgHead,pDiscoveryFrame->daddr,pDiscoveryFrame->saddr
        ,ntohs(pDiscoveryFrame->index),CTL_WORD_RESPONSE,0x01,8);

    responseMsg.ipaddr = htonl(pRootData->dev[ENUM_PC_DISCOVER].net_attr.ip);
    responseMsg.port  = htonl(pRootData->dev[ENUM_PC_DISCOVER].net_attr.sin_port);

    memset(sendBuf,0,SEND_BUFF_LENGTH);
    sendlen = msgPackFrame(sendBuf,&msgHead,&responseMsg,sizeof(responseMsg));
    
    AddData_ToSendList(pRootData,ENUM_PC_DISCOVER,sendBuf,sendlen);
    
}

void process_req_ptp_cfg_all(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        

    }
    else
    {
        
    }
    
}

void process_pc_ctl_req(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;
    struct Req_Frame *pReqFrame = (struct Req_Frame *)(buf+HEAD_FRAME_LENGTH);
    short cmd_type = ntohs(pReqFrame->cmd);


    switch(cmd_type)
    {

     case CMD_DEV_INFORMATION:
        break;
     case CMD_NET_WORK_ADDRESS:
        break;
     case CMD_VERSION_INFROMATION:
        break;
     case CMD_GPS_STATUS:
        break;
     case CMD_SYS_SET:
        break;
     case CMD_PTP_CFG_ALL:
        
        break;
     case CMD_NTP_CFG_NORMAL:
        break;
     case CMD_NTP_CFG_BLACKLIST:
        break;
     case CMD_NTP_CFG_WHITELSIT:
        break;
       
    }

}

void process_pc_ctl_set(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;
    struct Req_Frame *pReqFrame = (struct Req_Frame *)(buf+HEAD_FRAME_LENGTH);
    short cmd_type = ntohs(pReqFrame->cmd);

    switch(cmd_type)
    {

    case CMD_SYS_SET:
       break;
    case CMD_PTP_CFG_ALL:
       break;
    case CMD_PTP_CFG_NORMAL:
       break;
    case CMD_PTP_CFG_SLAVE:
       break;
    case CMD_PTP_CFG_MASTER:
       break;
    case CMD_PTP_CFG_UNICAST:
       break;
    case CMD_NTP_CFG_NORMAL:
       break;
    case CMD_NTP_CFG_MD5_ENABLE:
       break;
    case CMD_NTP_CFG_MD5_KEY:
       break;
    case CMD_NTP_CFG_BLACKLIST:
       break;
    case CMD_NTP_CFG_WHITELSIT:
       break;       
    }

}

void handle_pc_ctl_message(struct root_data *pRootData,char *buf,int len)
{
    struct Head_Frame msgHead;
    
    if(buf[0] != '$' && buf[1] != '<')
        return;

    msgUpPackHead(buf,&msgHead);

    switch(msgHead.ctype)
    {
        case CTL_WORD_REQ:
            process_pc_ctl_req(pRootData,&msgHead,buf);
            
            break;
        case CTL_WORD_ACK:
            break;
        case CTL_WORD_DATA:
            break;
        case CTL_WORD_SET:
            process_pc_ctl_set(pRootData,&msgHead,buf);
            break;
        case CTL_WORD_NAK:
            break;
        default:
            break;
         
    }
    
}

