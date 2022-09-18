#include "multiplayer/password.h"

#include "coreengine/filesupport.h"
#include <QCryptographicHash>

Password::Password()
{
    setPassword("");
}

Password::Password(QString password)
{
    setPassword(password);
}

void Password::serializeObject(QDataStream& pStream) const
{
    pStream << getVersion();
    Filesupport::writeByteArray(pStream, m_passwordHash);
}

void Password::deserializeObject(QDataStream& pStream)
{
    qint32 version = 0;
    pStream >> version;
    m_passwordHash = Filesupport::readByteArray(pStream);
}

void Password::setPassword(QString password)
{
    if (password.isEmpty())
    {
        m_passwordHash = QByteArray();
    }
    else
    {
        QCryptographicHash myHash(QCryptographicHash::Sha512);
        myHash.addData(password.toUtf8());
        m_passwordHash = myHash.result();
    }
}

void Password::setPassword(const Password & password)
{
    m_passwordHash = password.m_passwordHash;
}

bool Password::isValidPassword(QString password) const
{
    QByteArray data;
    if (!password.isEmpty())
    {
        QCryptographicHash myHash(QCryptographicHash::Sha512);
        myHash.addData(password.toUtf8());
        data = myHash.result();
    }
    return (m_passwordHash == data);
}

bool Password::areEqualPassword(const Password & password) const
{
    return (m_passwordHash == password.m_passwordHash);
}
