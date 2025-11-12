//
// Created by talik on 8/27/2025.
//

#include "PSClient.h"
#include <QDebug>

#include "editor/CodeRunner.h"

PSClient::PSClient(QObject *parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);

    // Connect signals to handle socket events.
    connect(m_socket, &QTcpSocket::connected, this, &PSClient::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &PSClient::onReadyRead);
    connect(this, &PSClient::connected, dynamic_cast<CodeRunner *>(parent), &CodeRunner::statusUpdate);
}

void PSClient::runScript(const QString &script) const
{
    qDebug() << "Attempting to connect to server...";
    m_socket->connectToHost("127.0.0.1", 12345);

    // Wait for the connection to be established.
    if(m_socket->waitForConnected())
    {
        qDebug() << "Connected! Sending script...";
        // Send the script text, followed by a newline character.
        m_socket->write((script + "\n").toUtf8());
        m_socket->flush();
    }
    else
    {
        qDebug() << "Connection failed:" << m_socket->errorString();
    }
}

void PSClient::onConnected()
{
    emit connected( "Running script...");
}

void PSClient::onReadyRead()
{
    // Read the response from the server.
    QByteArray data = m_socket->readAll();
    QString result = QString::fromUtf8(data).trimmed();
    qDebug() << "Received result from C#:" << result;

    // Emit a signal so other parts of your GUI can use the result.
    emit scriptResultReceived(QVariant::fromValue(result));

    // Close the connection after receiving the result.
    m_socket->close();
}
