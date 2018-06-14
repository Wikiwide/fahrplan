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

#include "parser_131500comau.h"

void parserTranslink::tsslErrors(const QList<QSslError> & errors)
{
    qDebug() << "SSL errors found";
    foreach( const QSslError &error, errors )
    {
        qDebug() << "SSL Error: " << error.errorString();
    }
}

parser131500ComAu::parser131500ComAu(QObject *parent)
{
     Q_UNUSED(parent);
     http = new QHttp(this);

     QSslSocket* sslSocket = new QSslSocket(this);
     sslSocket->setProtocol(QSsl::SecureProtocols);
     sslSocket->setPeerVerifyName("api.transport.nsw.gov.au");
     http->setSocket(sslSocket);

     connect(http, SIGNAL(requestFinished(int,bool)),
             this, SLOT(httpRequestFinished(int,bool)));
     connect(http, SIGNAL(sslErrors(const QList<QSslError> &)),
             this, SLOT(tsslErrors(const QList<QSslError> &)));
}

bool parser131500ComAu::supportsGps()
{
    return true;
}

QStringList parser131500ComAu::getStationsByName(QString stationName)
{
    qDebug() << "131500: getStationsByName";
    QString fullUrl = "https://api.transport.nsw.gov.au/v1/tp/stop_finder?outputFormat=rapidJSON&type_sf=stop&coordOutputFormat=EPSG%3A4326&TfNSWSF=true&version=10.2.1.42";
    fullUrl.append("&name_sf=" + stationName);
    qDebug() << fullUrl;

    QUrl url(fullUrl);

    http->setHost(url.host(), QHttp::ConnectionModeHttps, url.port() == -1 ? 0 : url.port());//sslErrors???

    filebuffer = new QBuffer();

    if (!filebuffer->open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open Buffer";
    }
    QHttpRequestHeader header;
    header.setRequest("GET", url.path());
    header.setValue("Host", url.host());
    header.setValue("Accept", "application/json");
    header.setValue("Authorization", "apikey NT3I18YtJOmS2xWEtAagMszSiXysEQeC84fY");//Wikiwide's API key
    currentRequestId = http->request(header, "", filebuffer);
    loop.exec();
    filebuffer->close();
    QJson::Parser parser;
    bool ok;
    QStringList result;
    QVariantMap map = parser.parse(filebuffer->buffer(),&ok).toMap();
    qDebug() << "Parsed JSON to Map: " << ok;
    QVariantList locations = map["locations"].toList();
    foreach (QVariant l, locations)
     {
      QVariantMap location = l.toMap();
      QString item = location["name"].toString() + "@" + location["id"].toString();
      bool ok;
      result.append(item);
     }
    delete filebuffer;
    return result;
}

QStringList parser131500ComAu::getTrainRestrictions()
{
    QStringList result;
    result.append(tr("All, except School Buses"));
    result.append(tr("Regular Buses"));
    result.append(tr("Trains"));
    result.append(tr("Ferries"));
    result.append(tr("STA School Bus"));
    return result;
}

QStringList parser131500ComAu::getStationsByGPS(qreal latitude, qreal longitude)
{
    qDebug() << "131500: getStationsByGPS";
    QString fullUrl = "https://api.transport.nsw.gov.au/v1/tp/stop_finder?outputFormat=rapidJSON&type_sf=coord&coordOutputFormat=EPSG%3A4326&TfNSWSF=true&version=10.2.1.42";
    fullUrl.append("&name_sf=" + longitude.toString() + "%3A" + latitude.toString() + "%3AEPSG%3A4326");
    qDebug() << fullUrl;

    QUrl url(fullUrl);

    http->setHost(url.host(), QHttp::ConnectionModeHttps, url.port() == -1 ? 0 : url.port());//sslErrors???

    filebuffer = new QBuffer();

    if (!filebuffer->open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open Buffer";
    }
    QHttpRequestHeader header;
    header.setRequest("GET", url.path());
    header.setValue("Host", url.host());
    header.setValue("Accept", "application/json");
    header.setValue("Authorization", "apikey NT3I18YtJOmS2xWEtAagMszSiXysEQeC84fY");//Wikiwide's API key
    currentRequestId = http->request(header, "", filebuffer);
    loop.exec();
    filebuffer->close();
    QJson::Parser parser;
    bool ok;
    QStringList result;
    QVariantMap map = parser.parse(filebuffer->buffer(),&ok).toMap();
    qDebug() << "Parsed JSON to Map: " << ok;
    QVariantList locations = map["locations"].toList();
    foreach (QVariant l, locations)
     {
      QVariantMap location = l.toMap();
      QString item = location["name"].toString() + "@" + location["id"].toString();
      bool ok;
      result.append(item);
     }
    delete filebuffer;
    return result;
}

ResultInfo parser131500ComAu::getJourneyData(QString destinationStation, QString arrivalStation, QString viaStation, QDate date, QTime time, int mode, int trainrestrictions)
{
    Q_UNUSED(viaStation);
    const QMap<int, QString> modes = {
        { 1, "train" },
        { 4, "light rail" },
        { 5, "bus" },
        { 7, "coach" },
        { 9, "ferry" },
        { 11, "school bus" },
        { 99, "walking" },
        { 100, "walking (footpath)" },
        { 101, "bicycle" },
        { 102, "take bicycle on public transport" },
        { 103, "kiss&ride" },
        { 104, "park&ride" },
        { 105, "taxi" },
        { 106, "car" },
    };
    QString modeString = "dep";
    if (mode == 0) {
        modeString = "arr";
    }

    //Request one. (Station selection and receiving an up to date cookie.)
    QString fullUrl = "https://api.transport.nsw.gov.au/v1/tp/trip?outputFormat=rapidJSON&coordOutputFormat=EPSG%3A4326&type_origin=any&type_destination=any&calcNumberOfTrips=6&TfNSWTR=true&version=10.2.1.42";//
    fullUrl.append("&itdDate=" + date.toString("yyyyMMdd"));
    fullUrl.append("&itdTime=" + time.toString("hhmm"));
    fullUrl.append("&depArrMacro=" + modeString);
    fullUrl.append("&name_origin=" + destinationStation.section('@',1,1));//10101331
    fullUrl.append("&name_destination=" + arrivalStation.section('@',1,1));//10102027
    fullUrl.append("&excludedMeans=checkbox");

    // itd_inclMOT_5 = bus
    // itd_inclMOT_1 = train
    // exclMOT_4 = light rail
    // exclMOT_7 = coach
    // itd_inclMOT_9 = ferry
    // itd_inclMOT_11 = school bus
    if (trainrestrictions == 0) {//everything except school buses
       fullUrl.append("&exclMOT_11=1");
    }
    if (trainrestrictions == 1) {//only buses (and coaches)
       fullUrl.append("&exclMOT_1=1&exclMOT_4=1&exclMOT_9=1&exclMOT_11=1");
    }
    if (trainrestrictions == 2) {//only trains (and light rail)
       fullUrl.append("&exclMOT_5=1&exclMOT_7=1&exclMOT_9=1&exclMOT_11=1");
    }
    if (trainrestrictions == 3) {//only ferries
       fullUrl.append("&exclMOT_1=1&exclMOT_4=1&exclMOT_5=1&exclMOT_7=1&exclMOT_11=1");
    }
    if (trainrestrictions == 4) {//only school buses
       fullUrl.append("&exclMOT_1=1&exclMOT_4=1&exclMOT_5=1&exclMOT_7=1&exclMOT_9=1");
    }

    qDebug() << fullUrl;

    QUrl url(fullUrl);

    http->setHost(url.host(), QHttp::ConnectionModeHttps, url.port() == -1 ? 0 : url.port());//sslErrors???

    filebuffer = new QBuffer();

    if (!filebuffer->open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open Buffer";
    }
    QHttpRequestHeader header;
    header.setRequest("GET", url.path());
    header.setValue("Host", url.host());
    header.setValue("Accept", "application/json");
    header.setValue("Authorization", "apikey NT3I18YtJOmS2xWEtAagMszSiXysEQeC84fY");//Wikiwide's API key
    currentRequestId = http->request(header, "", filebuffer);
    loop.exec();
    filebuffer->close();
    QJson::Parser parser;

    bool ok;
    ResultInfo result;
    QVariantMap map = parser.parse(filebuffer->buffer(),&ok).toMap();
    qDebug() << "Parsed JSON to Map: " << ok;
//    QVariantMap options = map["traveloptions"].toMap();//???
//    QVariantMap enquiry = options["OriginalEnquiry"].toMap();//???
//    result.fromStation = enquiry["Start"].toString();//???
//    result.toStation = enquiry["End"].toString();//???
//    QString sdate = options["searchDate"].toString();//???
//    result.timeInfo = sdate.section(' ',0,0);//???
    QVariantList journeys = map["journeys"].toList();
    foreach (QVariant j, journeys)
    {
        QVariantMap journey = j.toMap();
        ResultItem item;
        QStringList trains;
        QVariantList legs = journey["legs"].toList();
        int dur=0;
        foreach (QVariant l, legs)
        {
            QVariantMap leg = l.toMap();
            QVariantMap trans = leg["transportation"].toMap();
            dur=dur+leg["duration"].toInt();
            QVariantMap pro = trans["product"].toMap();
            QVariantMap tor = pro["class"].toInt();
            if (tor != 100 && tor != 99)
            {
                trains.append(modes[tor]);//use tor and QMap!!!
            }
        }
        trains.removeDuplicates();
        item.trainType = trains.join(", ");
        item.changes = QString::number(journey["interchanges"].toInt());
        item.duration = dur.toString();
        bool ok;
        QJson::Serializer serializer;
        QByteArray jsonleg = serializer.serialize(journey, &ok);
        qDebug() << "Serialized a journey: " << ok;
        item.detailsUrl = jsonleg;
        result.items.append(item);
    }
//QFile file("/home/user/post.txt");
//file.open(QIODevice::WriteOnly);
//file.write(filebuffer->buffer());
//file.close();
    delete filebuffer;
    return result;
}

ResultInfo parser131500ComAu::getJourneyData(QString queryUrl)//Which url???
{
    Q_UNUSED(queryUrl);
    ResultInfo result;
    return result;
}

//We using a fake url, in fact we don't use an url at all, we use the details
//data directly because the data is already present after visiting the search results.
DetailResultInfo parser131500ComAu::getJourneyDetailsData(QString queryUrl)//Which url??? !!!
{
    QJson::Parser parser;

    bool ok;
    DetailResultInfo result;
    QVariantMap journey = parser.parse(queryUrl.toAscii(),&ok).toMap();
    int dur=0;
    QVariantList legs = journey["legs"].toList();
    foreach (QVariant l, legs)
    {
        DetailResultItem item;
        QVariantMap leg = l.toMap();
        QString route = "";
        QVariantMap trans = leg["transportation"].toMap();
        QVariantMap pro = trans["product"].toMap();
        QVariantMap tor = pro["class"].toInt();
        if (tor == 100 or tor == 99)
        {
            route = "Walk ";
        }else{
            route = "";
        }
        route=modes[tor];
        dur=dur+leg["duration"].toInt();
        item.train = route;
        item.fromStation = leg["origin"].toString();//???
        item.toStation = leg["destination"].toString();
        item.fromInfo = leg["origin"].toString();//???
        item.toInfo = leg["destination"].toString();
        QString duration = "Duration: ";
        duration.append(leg["duration"].toString());//seconds!!!
        item.info = duration;
        result.items.append(item);
    }
    result.duration = dur.toString();

//QFile file("/home/user/post.txt");
//file.open(QIODevice::WriteOnly);
//file.write(filebuffer->buffer());
//file.close();
    delete filebuffer;
    return result;
}
