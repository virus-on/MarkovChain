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
        Dict<QString, 1> dict("", "*END*");
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

           QString currentToken = dict.GetFirstToken();
           QStringList tokenList {currentToken};
           while (currentToken != "*END*")
           {
               currentToken = dict.GetNextToken(currentToken);
               tokenList.append(currentToken);
           }
           qInfo() << tokenList.join(" ");
           inputFile.close();
        }
    }
    exit(0);

    return a.exec();
}
