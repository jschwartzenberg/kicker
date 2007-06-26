/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_SENSORAGENT_H
#define KSG_SENSORAGENT_H

#include <QObject>
#include <QQueue>

#include <kdemacros.h>

class QString;

namespace KSGRD {

class SensorClient;
class SensorManager;
class SensorRequest;

/**
  The SensorAgent depending on the type of requested connection
  starts a ksysguardd process or connects through a tcp connection to
  a running ksysguardd and handles the asynchronous communication. It
  keeps a list of pending requests that have not been answered yet by
  ksysguardd. The current implementation only allowes one pending
  requests. Incoming requests are queued in an input FIFO.
*/
class SensorAgent : public QObject
{
  Q_OBJECT

  public:
    explicit SensorAgent( SensorManager *sm );
    virtual ~SensorAgent();

    virtual bool start( const QString &host, const QString &shell,
                        const QString &command = "", int port = -1 ) = 0;

    /**
      This function should only be used by the the SensorManager and
      never by the SensorClients directly since the pointer returned by
      engaged is not guaranteed to be valid. Only the SensorManager knows
      whether a SensorAgent pointer is still valid or not.

      This function sends out a command to the sensor and notifies the
      agent to return the answer to 'client'. The 'id' can be used by the
      client to identify the answer. It is only passed through and never
      used by the SensorAgent. So it can be any value the client suits to
      use.
     */
    void sendRequest( const QString &req, SensorClient *client );

    virtual void hostInfo( QString &sh, QString &cmd, int &port ) const = 0;

    void disconnectClient( SensorClient *client );

    const QString &hostName() const;
	
  Q_SIGNALS:
    void reconfigure( const SensorAgent* );

  protected:
    void processAnswer( const char *buf, int buflen );
    void executeCommand();

    SensorManager *sensorManager();

    void setDaemonOnLine( bool value );
    bool daemonOnLine() const;

    void setTransmitting( bool value );
    bool transmitting() const;

    void setHostName( const QString &hostName );

  private:
    virtual bool writeMsg( const char *msg, int len ) = 0;
    virtual bool txReady() = 0;

    bool m_foundError;
    QQueue< SensorRequest* > m_inputFIFO;
    QQueue< SensorRequest* > m_processingFIFO;
    QList<QByteArray> m_answerBuffer;  ///A single reply can be on multiple lines.  
    QByteArray m_leftOverBuffer; ///Any data read in but not terminated is copied into here, awaiting the next load of data

    SensorManager *m_sensorManager;

    bool m_daemonOnLine;
    bool m_transmitting;
    QString m_hostName;
};

/**
  This auxilliary class is used to store requests during their processing.
*/
class SensorRequest
{
  public:
    SensorRequest( const QString &request, SensorClient *client );
    ~SensorRequest();

    void setRequest( const QString& );
    QString request() const;

    void setClient( SensorClient* );
    SensorClient *client();

  private:
    QString m_request;
    SensorClient *m_client;
};

}
	
#endif