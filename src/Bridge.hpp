#pragma once

#include <QObject>
#include <QString>
#include <memory>

class ComHost;
class QDebugRWPort;

#ifndef BRIDGE_COM_HOST_FREQ_HZ
#define BRIDGE_COM_HOST_FREQ_HZ 200
#endif

class Bridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastTestResult READ lastTestResult NOTIFY lastTestResultChanged)

public:
    explicit Bridge(QObject *parent = nullptr);
    ~Bridge() override;

    QString lastTestResult() const { return m_lastTestResult; }

    Q_INVOKABLE QString ComHostTest();

signals:
    void lastTestResultChanged();

private:
    std::unique_ptr<ComHost> m_comHost;
    std::unique_ptr<QDebugRWPort> m_debugPort;
    QString m_lastTestResult;
};
