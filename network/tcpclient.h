#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "network/NetworkInterface.h"
#include "network/rxtask.h"
#include "network/txtask.h"

class QTcpSocket;

class TCPClient;
using spTCPClient = oxygine::intrusive_ptr<TCPClient>;

class TCPClient : public NetworkInterface
{
    Q_OBJECT
public:
    TCPClient(QObject* pParent);
    TCPClient(QObject* pParent, spRxTask pRXTask, spTxTask pTXTask, QTcpSocket* pSocket, quint64 socketId);
    virtual ~TCPClient();
    /**
     * @brief moveClientToThread
     * @param pThread
     */
    void moveClientToThread(QThread* pThread);
    spRxTask getRXTask() const;

    spTxTask getTXTask() const;

    virtual void setSocketID(const quint64 &socketID) override;
public slots:
    virtual void connectTCP(QString adress, quint16 port, bool secure = false) override;
    virtual void disconnectTCP() override;
    virtual QVector<quint64> getConnectedSockets() override;
    virtual void changeThread(quint64 socketID, QThread* pThread) override;
protected slots:
    void connected();
private:
    spRxTask m_pRXTask;
    spTxTask m_pTXTask;
    std::shared_ptr<QTcpSocket> m_pSocket;
    bool m_onServer{false};
};

#endif // TCPCLIENT_H
