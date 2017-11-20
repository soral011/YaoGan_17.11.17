/***********************************************************
**
** 作者：lth
** 日期：2016-03-10
** 描述：用于打印信息，同时在信息的前面增加文件、函数名、行号；
**      命名为.cpp，以便每次改动文件都会被重新编译。
**
************************************************************/

#ifndef MYDEBUG
#define MYDEBUG
#include <QDebug>
#include <QDateTime>

/*
 * 1.发布后需要屏蔽的打印信息
 * 2.需要打印许多信息时，用法：MY_DEBUG(123<<"abc"<<46)
 */
#define __MY_DEBUG__
#ifdef  __MY_DEBUG__
#define   MY_DEBUG(info) qDebug()<<QDateTime::currentDateTime().toString("hh:mm:ss.zzz ")\
                                 <<"["\
                                 <<__FILE__\
                                 <<__FUNCTION__\
                                 <<__LINE__\
                                 <<"]"\
                                 <<info
#else
#define   MY_DEBUG
#endif

//不需要屏蔽的打印信息,以后会记录到日志文件
#define __MY_LOG__
#ifdef  __MY_LOG__
#define   MY_LOG(info) qCritical()<<"["\
                                <<__FILE__\
                                <<__FUNCTION__\
                                <<__LINE__\
                                <<"]"\
                                <<info
#else
#define   MY_LOG
#endif

#endif // MYDEBUG

