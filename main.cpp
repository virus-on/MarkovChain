#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#include "dict.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc == 2)
    {
        Dict<QString, 3> dict("", "*END*");
        QFile inputFile(argv[1]);
        if (inputFile.open(QIODevice::ReadOnly))
        {
           QTextStream in(&inputFile);
           while (!in.atEnd())
           {
              QString line = in.readLine();
              line.replace(",", " , ");
              line.replace(".", " . ");
              QList<QString> data(line.split(" ", QString::SkipEmptyParts));
              dict.AddData(data);
           }
           inputFile.close();

           int windowSize = 3;
           for (int i = 0; i < 10; i++)
           {
               QString currentToken = dict.GetFirstToken();
               QStringList tokenList {currentToken};
               try
               {
                   while (currentToken != "*END*" && currentToken != "." )
                   {
                       QList<QString> windowed;
                       if (tokenList.length() <= 2)
                       {
                           windowed = tokenList;
                       }
                       else
                       {
                           windowed = tokenList.mid(tokenList.length() - windowSize);
                       }
                       currentToken = dict.GetNextToken(windowed);
                       tokenList.append(currentToken);
                   }
               }
               catch (const std::exception& ex)
               {
                   qDebug() << ex.what();
               }
               qInfo() << tokenList.join(" ");
           }
        }
    }
    exit(0);

    return a.exec();
}
