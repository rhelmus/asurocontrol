/***************************************************************************
 *   Copyright (C) 2009 by Rick Helmus   *
 *   rhelmus_AT_gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QTcpSocket>

#include "tcputil.h"

CTcpWriter::CTcpWriter(QTcpSocket *s) : socket(s)
{
    dataStream = new QDataStream(&block, QIODevice::WriteOnly);
    dataStream->setVersion(QDataStream::Qt_4_5);
    *dataStream << (quint32)0; // Reserve place for size
}

CTcpWriter::~CTcpWriter()
{
    delete dataStream;
}

void CTcpWriter::write()
{
    // Put in the size
    dataStream->device()->seek(0);
    *dataStream << (quint32)(block.size() - sizeof(quint32));
    
    socket->write(block);
    block.clear();
}
