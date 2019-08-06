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
        Dict<QString, 2> dict("", "*END*");
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

           int windowSize = 2;
           for (int i = 0; i < 10; i++)
           {
               QString currentToken = dict.GetFirstToken();
               QStringList tokenList {currentToken};
               while (currentToken != "*END*")
               {
                   QList<QString> windowed;
                   if (tokenList.length() <= 2)
                   {
                       windowed = tokenList;
                   }
                   else
                   {
                       windowed = tokenList.mid(tokenList.length() - windowSize, tokenList.length() - windowSize + 1);
                   }

                   currentToken = dict.GetNextToken(windowed);
                   tokenList.append(currentToken);
               }
               qInfo() << tokenList.join(" ");
           }
        }
    }
    exit(0);

    return a.exec();
}
