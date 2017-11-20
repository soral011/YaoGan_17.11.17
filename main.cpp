#include <QApplication>
#include "MainWindow.h"
#include <QDebug>
#include <QSettings>
#include <QDataStream>
#include <QDir>
#include <QCryptographicHash>
#include "PlateRecogRegion.h"
#include "Global.h"
#include "results/Print.h"
#include "MyDebug.cpp"
#include "ControlCenter.h"
#include "SocketClient.h"
#include <QFont>
#include <QNetworkInterface>
#include <QDirModel>
#include <QTreeView>
#include <windows.h>
#include "tools/ChooseFileDialog.h"
#include "user_management/UserManagement.h"


using namespace std;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type)
    {
    case QtCriticalMsg:
        if(0)
        {
            fprintf(stderr, "%s", msg.toUtf8().data());
        }
        else
        {
            g_log(msg);
        }
        break;

    default:
        qDebug("%s L:%d %s \n", context.file, context.line, msg.toUtf8().data());
        break;

    }
}


int main(int argc, char **argv)
{
    //会造成调试的时候不能及时输出
//    qInstallMessageHandler(myMessageOutput);

    MY_LOG("app start !");

    QApplication a(argc, argv);

    g_qAppInit();

    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    a.setFont(font);

    ControlCenter controlCenter;

//    Camera *camera = Camera::getInstance();
//    MY_DEBUG("getAllOnlineIP="<<camera->getAllOnlineIP());
//    MY_DEBUG("getSameSectionIp="<<camera->getSameSectionIp(camera->getAllOnlineIP().first()));
//    MY_DEBUG("getAllOnlineCamera="<<camera->getAllOnlineCamera());

//    QStringList localIpList = camera->getAllOnlineIP();
//    foreach(QString localIp, localIpList)
//    {
//        MY_DEBUG("IP_section="<<localIp.remove(QRegExp("[0-9]{1,3}$")));
//    }

//    QStringList args;
//    QString cmd = QString("for /L %G in (1 1 255) do start /b ping -n 1 %1.%G").arg("192.168.1");
//    MY_DEBUG("cmd="<<cmd);
//    args <<"/c" << cmd;

//    QProcess process;
//    process.start("cmd", args);
//     MY_DEBUG("");
//    process.waitForFinished();
//    MY_DEBUG("process.state()="<<process.state());
//    MY_DEBUG("process.errorString()="<<process.errorString());
////    while(!process.bytesAvailable());
//    g_msleep(2000);
//    MY_DEBUG("process.bytesAvailable()="<<process.bytesAvailable());

//    return 0;

//    QStringList localIpList;
//    QList<QHostAddress> hostAddressList = QNetworkInterface::allAddresses();
//    foreach(QHostAddress address, hostAddressList)
//    {
//        //169.254.0.0-169.254.255.255 这段是私有保留地址
//        if(address.toString().contains("127.0.0.1") ||
//                address.toString().startsWith("169.254.") ||
//                address.toString().startsWith("192.0.0."))
//        {
//            continue;
//        }

//        if(address.protocol() == QAbstractSocket::IPv4Protocol)
//        {
//            localIpList.append(address.toString());
//        }
//    }
//    MY_DEBUG("localIpList="<<localIpList);
//    return 0;


    return a.exec();
}

