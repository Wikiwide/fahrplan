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

parser131500ComAu::parser131500ComAu(QObject *parent)
{
     Q_UNUSED(parent);
     http = new QHttp(this);

     connect(http, SIGNAL(requestFinished(int,bool)),
             this, SLOT(httpRequestFinished(int,bool)));
}

bool parser131500ComAu::supportsGps()
{
    return false;
}

QStringList parser131500ComAu::getStationsByName(QString stationName)
{
    QString fullUrl = "http://www.131500.com.au/plan-your-trip/trip-planner?session=invalidate&itd_cmd=invalid&itd_includedMeans=checkbox&itd_inclMOT_5=1&itd_inclMOT_7=1&itd_inclMOT_1=1&itd_inclMOT_9=1&itd_anyObjFilter_origin=2&itd_anyObjFilter_destination=0&itd_name_destination=Enter+location&x=37&y=12&itd_itdTripDateTimeDepArr=dep&itd_itdTimeHour=-&itd_itdTimeMinute=-&itd_itdTimeAMPM=pm";
    fullUrl.append("&itd_itdDate=" + QDate::currentDate().toString("yyyyMMdd"));
    fullUrl.append("&itd_name_origin=" + stationName);

    qDebug() << "parser131500ComAu::getStationsByName";

    QUrl url(fullUrl);

    http->setHost(url.host(), QHttp::ConnectionModeHttp, url.port() == -1 ? 0 : url.port());

    filebuffer = new QBuffer();

    if (!filebuffer->open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open Buffer";
    }

    currentRequestId = http->get(url.path() + "?" + url.encodedQuery(), filebuffer);

    loop.exec();

    filebuffer->close();

    QRegExp regexp = QRegExp("<select name=\"(.*)\" id=\"from\" size=\"6\" class=\"multiple\">(.*)</select>");
    regexp.setMinimal(true);

    regexp.indexIn(filebuffer->buffer());

    QString element = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><html xmlns=\"http://www.w3.org/1999/xhtml\">\n<body>\n" + regexp.cap(0) + "\n</body>\n</html>\n";

    QBuffer readBuffer;
    readBuffer.setData(element.toAscii());
    readBuffer.open(QIODevice::ReadOnly);

    QXmlQuery query;
    query.bindVariable("path", &readBuffer);
    //Query for more than one result
    query.setQuery("declare default element namespace \"http://www.w3.org/1999/xhtml\"; declare variable $path external; doc($path)/html/body/select/option/string()");

    QStringList result;
    if (!query.evaluateTo(&result))
    {
        qDebug() << "parser131500ComAu::getStationsByName - Query 1 Failed";
    }

    //Remove unneeded stuff from the result
    for (int i = 0; i < result.count(); i++) {
        result[i].replace(" (Location)", "");
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
    Q_UNUSED(latitude);
    Q_UNUSED(longitude);
    QStringList result;
    return result;
}

ResultInfo parser131500ComAu::getJourneyData(QString destinationStation, QString arrivalStation, QString viaStation, QDate date, QTime time, int mode, int trainrestrictions)
{
    Q_UNUSED(viaStation);
    QString modeString = "dep";
    if (mode == 0) {
        modeString = "arr";
    }

    //Request one. (Station selection and receiving an up to date cookie.)
    QString fullUrl = "https://api.transport.nsw.gov.au/v1/tp/trip?outputFormat=rapidJSON&coordOutputFormat=EPSG%3A4326&type_origin=any&type_destination=any&calcNumberOfTrips=6&TfNSWTR=true&version=10.2.1.42";//
    fullUrl.append("&itdDate=" + date.toString("yyyyMMdd"));
    fullUrl.append("&itdTime=" + time.toString("hhmm"));
    fullUrl.append("&depArrMacro=" + modeString);
    fullUrl.append("&name_origin=" + destinationStation);//10101331
    fullUrl.append("&name_destination=" + arrivalStation);//10102027
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
      int changes = 0;
      foreach (QVariant l, legs)
       {
        QVariantMap leg = l.toMap();
        QVariantMap trans = leg["transportation"].toMap();
        QVariantMap pro = trans["product"].toMap();
        QVariantMap tor = pro["class"].toInt();
        if (tor != 100 && tor != 99)
         {
          trains.append(trans["description"].toString());//use tor and QMap!!!
          changes++;
         }
        }
        trains.removeDuplicates();
        item.trainType = trains.join(", ");
        int c = changes - 1;
        if (c < 0)
         {
          c = 0;
         }
        item.changes = QString::number(c);
        item.duration = journey["duration"].toString();
        QJson::Serializer serializer;
        bool ok;
        QByteArray jsonleg = serializer.serialize(journey, &ok);
item.detailsUrl = jsonleg;
result.items.append(item);
}
//QFile file("/home/user/post.txt");
//file.open(QIODevice::WriteOnly);
//file.write(filebuffer->buffer());
//file.close();
return result;

    QStringList departResult;
    if (!query.evaluateTo(&departResult))
    {
        qDebug() << "parser131500ComAu::getJourneyData - Query 1 Failed";
    }

    QStringList arriveResult;
    if (!query.evaluateTo(&arriveResult))
    {
        qDebug() << "parser131500ComAu::getJourneyData - Query 2 Failed";
    }

    QStringList timeResult;
    if (!query.evaluateTo(&timeResult))
    {
        qDebug() << "parser131500ComAu::getJourneyData - Query 3 Failed";
    }

    QStringList trainResult;
    if (!query.evaluateTo(&trainResult))
    {
        qDebug() << "parser131500ComAu::getJourneyData - Query 4 Failed";
    }

    QStringList headerResult;
    if (!query.evaluateTo(&headerResult))
    {
        qDebug() << "parser131500ComAu::getJourneyData - Query 5 Failed";
    }

    delete filebuffer;
    return result;
}

ResultInfo parser131500ComAu::getJourneyData(QString queryUrl)
{
    Q_UNUSED(queryUrl);
    ResultInfo result;
    return result;
}

//We using a fake url, in fact we don't use an url at all, we use the details
//data directly because the data is already present after visiting the search results.
DetailResultInfo parser131500ComAu::getJourneyDetailsData(QString queryUrl)
{
    QStringList detailResults = queryUrl.split("<linesep>");

    DetailResultInfo result;

    QDate journeydate;

    for (int i = 0; i < detailResults.count(); i++) {
        DetailResultItem item;
        QRegExp regexp = QRegExp("(Take the |Walk to |Header: )(.*)$");
        regexp.setMinimal(true);
        regexp.indexIn(detailResults[i].trimmed());

        if (regexp.cap(1) == "Header: ") {
            //qDebug()<<"HEADER: "<<regexp.cap(2).trimmed();
            QRegExp regexp2 = QRegExp("<duration>(.*)</duration><date>(.*), (\\d\\d) (.*) (\\d\\d\\d\\d)</date>");
            regexp2.setMinimal(true);
            regexp2.indexIn(regexp.cap(2).trimmed());
            result.duration = regexp2.cap(1).trimmed();
            QLocale enLocale = QLocale(QLocale::English, QLocale::UnitedStates);
            int month = 1;
            for (month = 1; month < 10; month++) {
                if (regexp2.cap(4).trimmed() == enLocale.standaloneMonthName(month)) {
                    break;
                }
            }
            journeydate = QDate::fromString(regexp2.cap(3).trimmed() + " " + QString::number(month) + " " + regexp2.cap(5).trimmed(), "dd M yyyy");
        }
        if (regexp.cap(1) == "Take the ") {
            //qDebug()<<"Regular: "<<regexp.cap(2).trimmed();
            QRegExp regexp2 = QRegExp("(.*)Dep: (\\d:\\d\\d|\\d\\d:\\d\\d)(am|pm) (.*)Arr: (\\d:\\d\\d|\\d\\d:\\d\\d)(am|pm) (.*)(\\t+.*)$");
            regexp2.setMinimal(true);
            regexp2.indexIn(regexp.cap(2).trimmed());
            //qDebug()<<"***";
            if (regexp2.matchedLength() == -1) {
                regexp2 = QRegExp("(.*)Dep: (\\d:\\d\\d|\\d\\d:\\d\\d)(am|pm) (.*)Arr: (\\d:\\d\\d|\\d\\d:\\d\\d)(am|pm) (.*)$");
                regexp2.setMinimal(true);
                regexp2.indexIn(regexp.cap(2).trimmed());
            }
            /*
            qDebug()<<"Train:"<<regexp2.cap(1).trimmed();
            qDebug()<<"Time1:"<<regexp2.cap(2).trimmed();
            qDebug()<<"Time1b:"<<regexp2.cap(3).trimmed();
            qDebug()<<"Station1:"<<regexp2.cap(4).trimmed();
            qDebug()<<"Time2:"<<regexp2.cap(5).trimmed();
            qDebug()<<"Time2b:"<<regexp2.cap(6).trimmed();
            qDebug()<<"Station2:"<<regexp2.cap(7).trimmed();
            qDebug()<<"Alt:"<<regexp2.cap(8).trimmed();
            */
            item.fromStation = regexp2.cap(4).trimmed();
            item.toStation   = regexp2.cap(7).trimmed();
            item.info        = regexp2.cap(8).trimmed();
            item.train       = regexp2.cap(1).trimmed();
            QTime fromTime   = QTime::fromString(regexp2.cap(2).trimmed() + regexp2.cap(3).trimmed(), "h:map");
            QTime toTime     = QTime::fromString(regexp2.cap(5).trimmed() + regexp2.cap(6).trimmed(), "h:map");

            item.fromTime.setDate(journeydate);
            item.fromTime.setTime(fromTime);
            item.toTime.setDate(journeydate);
            item.toTime.setTime(toTime);

            if (item.toTime.toTime_t() < item.fromTime.toTime_t()) {
                item.toTime.addDays(1);
                journeydate.addDays(1);
            }

            item.fromInfo = item.fromTime.time().toString("hh:mm");
            item.toInfo = item.toTime.time().toString("hh:mm");

            result.items.append(item);
        }
        if (regexp.cap(1) == "Walk to ") {
            //qDebug()<<"Walking: "<<regexp.cap(2).trimmed();
            QRegExp regexp2 = QRegExp("(.*) - (.+) (.*)$");
            regexp2.setMinimal(true);
            regexp2.indexIn(regexp.cap(2).trimmed());
            /*
            qDebug()<<"***";
            qDebug()<<"Station1:"<<regexp2.cap(1).trimmed();
            qDebug()<<"WalkDist1:"<<regexp2.cap(2).trimmed();
            qDebug()<<"WalkDist2:"<<regexp2.cap(3).trimmed();
            */
            item.fromStation = "";
            if (result.items.count() > 0) {
                item.fromStation = result.items.last().toStation;
                item.fromInfo    = result.items.last().toInfo;
                item.fromTime    = result.items.last().toTime;
                item.toTime      = result.items.last().toTime;
            }
            item.toStation   = regexp2.cap(1).trimmed();
            item.info        = "Walking " + regexp2.cap(2).trimmed() + " " + regexp2.cap(3).trimmed();

            //Don't add WalkTo infos as first item
            if (result.items.count() > 0) {
                result.items.append(item);
            }
        }
    }
    return result;
}
