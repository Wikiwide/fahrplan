/*******************************************************************************

    This file is a part of Fahrplan for maemo 2009-2010

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#ifndef PARSER_XMLSBBCH_H
#define PARSER_XMLSBBCH_H

#include "parser_hafasxml.h"

class parserXmlSbbCh : public parserHafasXml
{
    Q_OBJECT
public:
    explicit parserXmlSbbCh(QObject *parent = 0);
    QStringList getTrainRestrictions();
    QString getTrainRestrictionsCodes(int trainrestrictions);

signals:

public slots:

private:
};

#endif // PARSER_XMLSBBCH_H
