// $Id$

#ifndef COMMCLIENT_H
#define COMMCLIENT_H

#include <QObject>

class OpenMSXConnection;
class Command;
class QString;

class CommClient : public QObject
{
	Q_OBJECT
public:
	static CommClient& instance();

	void sendCommand(Command* command);

public slots:
	void connectToOpenMSX(OpenMSXConnection* conn);
	void closeConnection();

signals:
	void connectionReady();
	void connectionTerminated();

	void logParsed(const QString& level, const QString& message);
	void updateParsed(const QString& type, const QString& name, const QString& message);

private:
	CommClient();
	~CommClient();

	OpenMSXConnection* connection;
};

#endif // COMMCLIENT_H
