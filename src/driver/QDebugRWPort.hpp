#pragma once

#include "../backstage/RWPort/RWPort.hpp"

#include <QDebug>
#include <QString>

class QDebugRWPort : public RWPort
{
public:
    explicit QDebugRWPort(const char *name = "QDebug")
        : RWPort(name)
    {
        qDebug() << "QDebugRWPort constructor:" << name;
    }

    ~QDebugRWPort() override
    {
        qDebug() << "QDebugRWPort destructor:" << Name();
    }

    uint32_t Write(uint8_t *data, uint32_t len) override
    {
        QString hex;
        hex.reserve(static_cast<int>(len) * 3);
        for (uint32_t i = 0; i < len; ++i) {
            if (i > 0 && (i % 8) == 0) {
                hex += QLatin1Char('\n');
            }
            hex += QStringLiteral("%1 ").arg(data[i], 2, 16, QLatin1Char('0')).toUpper();
        }
        qDebug().noquote() << QStringLiteral("\nQDebugRWPort(%1) Write (len%2):\n%3")
                                  .arg(Name())
                                  .arg(len)
                                  .arg(hex);
        return len;
    }

    uint32_t Read(uint8_t *data, uint32_t len) override
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }
};
