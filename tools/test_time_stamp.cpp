// Qt includes
#include <QCoreApplication>

#include <QString>
#include <QDebug>

// Local includes
#include "time_stamp.h"

using namespace AirPlug;

int main(int argc, char *argv[])
{
    TimeStamp s0(0), s1(1), s2(3);

    s0++; s0++;
    qDebug() << "s0" << s0.getTime() << s0.getSiteID();

    qDebug() << "s1" << s1.getTime() << s1.getSiteID();

    s2++;
    qDebug() << "s2" << s2.getTime() << s2.getSiteID();

    qDebug() << "(s0<s2)" << (s0<s2);

    qDebug() << "(s1<s2)" << (s1<s2);

    qDebug() << "(s1<s0)" << (s1<s0);

    return 0;
}
