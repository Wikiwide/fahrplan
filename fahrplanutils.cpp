#include "fahrplanutils.h"

fahrplanUtils::fahrplanUtils()
{
}

QStringList fahrplanUtils::reverseQStringList(const QStringList &list)
{
    if (list.isEmpty())
    {
        return QStringList();
    }

    QStringList reversedList = list;
    const int listSize = list.size();
    const int maxSwap = list.size() / 2;
    for (int i = 0; i < maxSwap; ++i)
    {
        qSwap(reversedList[i], reversedList[listSize - 1 -i]);
    }

    return reversedList;
}

QString fahrplanUtils::leadingZeros(int number, int presision)
{
    QString s;
    s.setNum(number);
    s = s.toUpper();
    presision -= s.length();
    while(presision>0){
        s.prepend('0');
        presision--;
    }
    return s;
}

/*
 * Replace common used umlaut chars to a maybe wrong but
 * readable format (used for calendar export)
 */
QString fahrplanUtils::removeUmlauts(QString text)
{
    text.replace("�", "ae");
    text.replace("�", "oe");
    text.replace("�", "ue");
    text.replace("�", "Ae");
    text.replace("�", "Oe");
    text.replace("�", "�e");
    text.replace("�", "ss");
    text.replace("�", "e");
    text.replace("�", "e");
    text.replace("�", "a");
    text.replace("�", "a");
    text.replace("�", "u");
    text.replace("�", "u");
    text.replace(QChar(160), " "); //space from nbsp
    text.replace("<br>", "\n"); //html linewrap
    text.replace("<br/>", "\n"); //html linewrap
    text.replace("<br />", "\n"); //html linewrap
    return text;
}
