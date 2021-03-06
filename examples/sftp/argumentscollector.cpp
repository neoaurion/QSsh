/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: http://www.qt-project.org/
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**************************************************************************/

#include "argumentscollector.h"

#include <iostream>

using namespace QSsh;

using namespace std;

ArgumentsCollector::ArgumentsCollector(const QStringList &args)
    : m_arguments(args)
{
}

Parameters ArgumentsCollector::collect(bool &success) const
{
    Parameters parameters;
    parameters.sshParams.options &= ~SshIgnoreDefaultProxy;
    try {
        bool authTypeGiven = false;
        bool portGiven = false;
        bool timeoutGiven = false;
        bool smallFileCountGiven = false;
        bool bigFileSizeGiven = false;
        bool proxySettingGiven = false;
        int pos;
        int port;
        QString host;
        QString user;
        QString path;
        for (pos = 1; pos < m_arguments.count() - 1; ++pos) {
            if (checkAndSetStringArg(pos, host, "-h") || checkAndSetStringArg(pos, user, "-u")) {
                parameters.sshParams.setHost(host);
                parameters.sshParams.setUserName(user);
                continue;
            }
            if (checkAndSetIntArg(pos, port, portGiven, "-p")
                || checkAndSetIntArg(pos, parameters.sshParams.timeout, timeoutGiven, "-t")
                || checkAndSetIntArg(pos, parameters.smallFileCount, smallFileCountGiven, "-c")
                || checkAndSetIntArg(pos, parameters.bigFileSize, bigFileSizeGiven, "-s"))
                continue;
            QString pass;
            if (checkAndSetStringArg(pos, pass, "-pwd")) {
                parameters.sshParams.setPassword(pass);
                if (!parameters.sshParams.privateKeyFile.isEmpty())
                    throw ArgumentErrorException(QLatin1String("-pwd and -k are mutually exclusive."));
                parameters.sshParams.authenticationType
                    = SshConnectionParameters::AuthenticationTypeTryAllPasswordBasedMethods;
                authTypeGiven = true;
                continue;
            }
            if (checkAndSetStringArg(pos, parameters.remotePath, "-d")) {
                continue;
            }
            if (checkAndSetStringArg(pos, parameters.sshParams.privateKeyFile, "-k")) {
                if (!parameters.sshParams.password().isEmpty())
                    throw ArgumentErrorException(QLatin1String("-pwd and -k are mutually exclusive."));
                parameters.sshParams.authenticationType
                    = SshConnectionParameters::AuthenticationTypePublicKey;
                authTypeGiven = true;
                continue;
            }
            if (!checkForNoProxy(pos, parameters.sshParams.options, proxySettingGiven))
                throw ArgumentErrorException(QLatin1String("unknown option ") + m_arguments.at(pos));
        }

        Q_ASSERT(pos <= m_arguments.count());
        if (pos == m_arguments.count() - 1) {
            if (!checkForNoProxy(pos, parameters.sshParams.options, proxySettingGiven))
                throw ArgumentErrorException(QLatin1String("unknown option ") + m_arguments.at(pos));
        }

        if (!authTypeGiven)
            throw ArgumentErrorException(QLatin1String("No authentication argument given."));
        if (parameters.sshParams.host().isEmpty())
            throw ArgumentErrorException(QLatin1String("No host given."));
        if (parameters.sshParams.userName().isEmpty())
            throw ArgumentErrorException(QLatin1String("No user name given."));

        parameters.sshParams.setPort(portGiven ? port : 22);
        if (!timeoutGiven)
            parameters.sshParams.timeout = 30;
        if (!smallFileCountGiven)
            parameters.smallFileCount = 10;//00;
        if (!bigFileSizeGiven)
            parameters.bigFileSize = 1024;
        if (parameters.remotePath.isEmpty()) {
            parameters.remotePath = "/tmp/";
        } else if (!parameters.remotePath.endsWith('/')) {
            parameters.remotePath += '/';
        }
        success = true;
    } catch (ArgumentErrorException &ex) {
        cerr << "Error: " << qPrintable(ex.error) << endl;
        printUsage();
        success = false;
    }
    return parameters;
}

void ArgumentsCollector::printUsage() const
{
    cerr << "Usage: " << qPrintable(m_arguments.first())
        << " -h <host> -u <user> "
        << "-pwd <password> | -k <private key file> [ -p <port> ] "
        << "[ -t <timeout> ] [ -c <small file count> ] "
        << "[ -d <remote dir> ] "
        << "[ -s <big file size in MB> ] [ -no-proxy ]" << endl;
}

bool ArgumentsCollector::checkAndSetStringArg(int &pos, QString &arg, const char *opt) const
{
    if (m_arguments.at(pos) == QLatin1String(opt)) {
        if (!arg.isEmpty()) {
            throw ArgumentErrorException(QLatin1String("option ") + opt
                + QLatin1String(" was given twice."));
        }
        arg = m_arguments.at(++pos);
        if (arg.isEmpty() && QLatin1String(opt) != QLatin1String("-pwd"))
            throw ArgumentErrorException(QLatin1String("empty argument not allowed here."));
        return true;
    }
    return false;
}

bool ArgumentsCollector::checkAndSetIntArg(int &pos, int &val,
    bool &alreadyGiven, const char *opt) const
{
    if (m_arguments.at(pos) == QLatin1String(opt)) {
        if (alreadyGiven) {
            throw ArgumentErrorException(QLatin1String("option ") + opt
                + QLatin1String(" was given twice."));
        }
        bool isNumber;
        val = m_arguments.at(++pos).toInt(&isNumber);
        if (!isNumber) {
            throw ArgumentErrorException(QLatin1String("option ") + opt
                 + QLatin1String(" needs integer argument"));
        }
        alreadyGiven = true;
        return true;
    }
    return false;
}

bool ArgumentsCollector::checkForNoProxy(int &pos, SshConnectionOptions &options,
                                         bool &alreadyGiven) const
{
    if (m_arguments.at(pos) == QLatin1String("-no-proxy")) {
        if (alreadyGiven)
            throw ArgumentErrorException(QLatin1String("proxy setting given twice."));
        options |= SshIgnoreDefaultProxy;
        alreadyGiven = true;
        return true;
    }
    return false;
}
