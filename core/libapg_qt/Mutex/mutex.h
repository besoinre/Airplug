/* ============================================================
 *
 * This file is a part of Airplug project
 *
 * Date        : 2020-5-03
 * Description : implementation of mutex
 *
 * 2020 by Jerome RAYNAL & Bastien DESTEPHEN
 *
 * ============================================================ */

#ifndef MUTEX_H
#define MUTEX_H

// Qt include
#include <QString>
#include <QHash>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace AirPlug
{

class Mutex : public QObject
{
public:
    enum MutexType
    {
        request,
        release,
        acknowledgement,
    };

    struct MutexData {
        MutexType type;
        int date;
    };
    // because it's more difficult to keep track of site's order
    QHash<QString, MutexData> mutexTab;
    QString             siteID;
    int date;

    Mutex(const QString& siteID);
    Mutex(const QString& siteID, const QString* otherSiteID); //Initiate Mutex with other siteID

    ~Mutex();


public:

    void Mutex::addMutexData(const QString& siteID, const MutexData data);

//    /**
//     * @brief getSiteID: return siteID
//     * @return
//     */
//    QString getSiteID() const;

//    /**
//     * @brief getValue : return decimal clock value of a particular site
//     * @param siteID
//     * @return
//     */
//    int getValue(const QString& siteID) const;

//public:

//    VectorClock operator= (const VectorClock& other);

//    /**
//     * @brief operator ++: prefix ++ operator
//     * @return
//     */
//    VectorClock operator++ ();

//    /**
//     * @brief operator ++: postfix ++ operator
//     * @return
//     */
//    VectorClock operator++ (int);

//    /**
//     * @brief updateClock: update local clock after receive another's clock
//     * @param other
//     */
//    void updateClock(const VectorClock& other);

//    /**
//     * @brief operator < : order relation between 2 clocks
//     * @param other
//     * @return
//     */
//    bool operator < (const VectorClock& other);

private:

    class Private;
    Private* d;
};

}
#endif // VECTOR_CLOCK_H
