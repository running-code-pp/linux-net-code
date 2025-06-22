/**
 * ==================================================
 *  @file demondProcess.cpp
 *  @brief 守护进程
 *  @author ywj
 *  @date 2025-06-21 16:45
 *  @version 1.0
 *  @copyright Copyright (c) 2025 ywj. All Rights Reserved.
 * ==================================================
 */

#include<sys/syslog.h>
#include<unistd.h>
#include<stdio.h>

int main()
{
    /**
    当调用daemon(0,0)的时候会进行一下操作：
    fork() 子进程：父进程退出，子进程继续运行。这样做的目的是让启动程序的 shell 认为命令已经完成，即使实际上子进程仍在后台运行。
    setsid() 创建新会话：这使得子进程成为新的会话领导，脱离任何控制终端。这意味着守护进程不会再收到来自终端的信号，如挂断信号（SIGHUP）。
    再次 fork()：为了确保守护进程不是会话领导（某些系统要求这样做以防止重新获取控制终端），通常会再 fork 一次，并让第二个子进程继续运行，而第一个子进程退出。
    改变工作目录：如果 __nochdir 参数为 0，则调用 chdir("/") 改变当前工作目录为根目录。
    重定向标准文件描述符：如果 __noclose 参数为 0，则关闭标准输入、输出和错误，并将它们重定向到 /dev/null。
    忽略信号：有时还会调整信号处理方式，比如忽略 SIGHUP 等
     */
    daemon(0, 0);
    printf("testtesttest");
    openlog("test_deamon",LOG_NDELAY,LOG_USER);
    syslog(LOG_EMERG, "EMERG LOG MSG"); //系统不可用（最高优先级）这个会显示在任何一个开启的终端中
    syslog(LOG_ALERT, "ALERT LOG MSG"); //必须立即采取行动
    syslog(LOG_CRIT, "CRIT LOG MSG"); //临界条件
    syslog(LOG_ERR, "ERR LOG MSG"); //出错条件
    syslog(LOG_WARNING, "WARNING LOG MSG"); //警告条件
    syslog(LOG_NOTICE, "NOTICE LOG MSG"); //正常然而重要的条件（默认值）
    syslog(LOG_INFO, "INFO LOG MSG"); //通告消息
    syslog(LOG_DEBUG, "DEBUG LOG MSG"); //调试消息（最低优先级）
    printf("testtesttest");
    closelog();
    printf("testtesttest");
    //查看日志：tail -f /var/log/syslog
    return 0;
}
