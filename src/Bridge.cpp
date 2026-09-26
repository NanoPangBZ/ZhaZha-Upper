#include "Bridge.hpp"

#include "backstage/ComHost/ComHost.hpp"
#include "driver/QDebugRWPort.hpp"

Bridge::Bridge(QObject *parent)
    : QObject(parent)
{
    m_comHost = std::make_unique<ComHost>(COM_HOST_ID_ADMIN);
    m_debugPort = std::make_unique<QDebugRWPort>("QDebug");

    QDebugRWPort *port = m_debugPort.get();
    m_comHost->AddComPort(
        [port](uint8_t *data, uint32_t len) -> uint32_t { return port->Write(data, len); },
        [port](uint8_t *data, uint32_t len) -> uint32_t { return port->Read(data, len); },
        port->Name());

    m_comHost->AddRoute(COM_HOST_ID_ADMIN, port->Name());
    m_comHost->Start(BRIDGE_COM_HOST_FREQ_HZ);
}

Bridge::~Bridge()
{
    if (m_comHost) {
        m_comHost->Stop();
    }
}

QString Bridge::ComHostTest()
{
    com_host_msg_t test_msg;
    test_msg.msg_id = COM_HOST_MSG_ID_PING;
    test_msg.data = nullptr;
    test_msg.data_len = 0;
    test_msg.is_ack = 0;
    test_msg.need_ack = 1;

    ComHost::Result result = m_comHost->SendMsgAndWaitAck(
        COM_HOST_ID_ADMIN,
        &test_msg,
        nullptr,
        1000,
        1);

    QString text;
    switch (result) {
    case ComHost::COM_SUCCEEDED:
        text = QStringLiteral("Ping 成功");
        break;
    case ComHost::COM_TIMEOUT:
        text = QStringLiteral("Ping 超时");
        break;
    case ComHost::COM_FAILED:
    default:
        text = QStringLiteral("Ping 失败");
        break;
    }

    if (m_lastTestResult != text) {
        m_lastTestResult = text;
        emit lastTestResultChanged();
    }
    return text;
}
