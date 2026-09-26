#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>

#include "backstage/ComHost/ComHost.hpp"
#include "backstage/RWPort/CliRWPort.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    ComHost comHost(COM_HOST_ID_ADMIN);
    CliRWPort* cliRWPort = new CliRWPort("CLI");
    /* 添加CLI通信端口 */
    comHost.AddComPort( [&,cliRWPort]( uint8_t* data , uint32_t len ) -> uint32_t { return cliRWPort->Write(data, len); } ,\
                       [&,cliRWPort]( uint8_t* data , uint32_t len ) -> uint32_t { return cliRWPort->Read(data, len); } ,\
                                                                                                                      cliRWPort->Name() );
    /* 添加路由 */
    comHost.AddRoute( COM_HOST_ID_ADMIN , cliRWPort->Name() );

    com_host_msg_t test_msg;
    test_msg.msg_id = COM_HOST_MSG_ID_PING;
    test_msg.data = nullptr;
    test_msg.data_len = 0;
    test_msg.is_ack = 0;
    test_msg.need_ack = 1;

    comHost.SendMsgAndWaitAck( COM_HOST_ID_ADMIN , &test_msg ,
                              []( uint8_t* ackData , uint16_t dataLen , ComHost::Result result ) -> void {
                                  std::cout << "SendMsgAndWaitAck result: " << result << std::endl;
                              } , 1000 , 1 );

    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    comHost.Stop();
    delete cliRWPort;

    return app.exec();
}
