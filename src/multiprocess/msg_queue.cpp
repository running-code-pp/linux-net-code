/**
 *@brief :消息队列测试
 */

#include <bits/socket.h>
#include<sys/msg.h>

int main(int argc,char**argv)
{
msgget((key_t) IPC_PRIVATE,IPC_CREAT);
    msgsnd((key_t) IPC_PRIVATE,MSG_NOSIGNAL,MSG_NOSIGNAL);
    msgctl((key_t) IPC_PRIVATE,IPC_RMID,NULL);
    msgscv((key_t) IPC_PRIVATE,MSG_NOSIGNAL,0);



    return 0;
}