#include "Dictionary.h"
#include "dict.h"

#include <chrono>

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

int main(int argc, char *argv[])
{
    using namespace std::chrono;
    QCoreApplication a(argc, argv);

    if (argc == 2)
    {
        auto start = high_resolution_clock::now();
        Dictionary<QString, 3> dict;
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
                dict.addDataToDictionary<QList<QString>>(data.begin(), data.end());
            }
            inputFile.close();

            for (int i = 0; i < 100; ++i)
            {
                int windowSize = 2;
                std::vector<const QString*> genOutput;
                const QString* genVal =  nullptr;
                do
                {
                   genVal =  dict.getToken<decltype(genOutput)>(genOutput.begin(), genOutput.end(), windowSize);
                   if (not genVal)
                       break;
                   genOutput.push_back(genVal);
                } while(genVal);

                QStringList tokenList;

                for (const auto& token: genOutput)
                    tokenList.push_back(*token);

                qInfo() << tokenList.join(" ");
            }
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        qInfo() << "<<<<<<<<<<<<<<<<<<<<<<<<";

        start = high_resolution_clock::now();
        Dict<QString, 3> dict1("", "*END*");
        QFile inputFile1(argv[1]);
        if (inputFile1.open(QIODevice::ReadOnly))
        {
            QTextStream in(&inputFile1);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                line.replace(",", " , ");
                line.replace(".", " . ");
                QList<QString> data(line.split(" ", QString::SkipEmptyParts));
                dict1.AddData(data);
            }
            inputFile1.close();

            int windowSize = 2;
            for (int i = 0; i < 100; i++)
            {
                QString currentToken = dict1.GetFirstToken();
                QStringList tokenList {currentToken};
                try
                {
                    while (currentToken != "*END*" )
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
                        currentToken = dict1.GetNextToken(windowed);
                        tokenList.append(currentToken);
                    }
                }
                catch (const std::exception& ex)
                {
                    qDebug() << ex.what();
                }
                qInfo() << i << tokenList.join(" ");
            }
        }
        stop = high_resolution_clock::now();
        auto duration1 = duration_cast<milliseconds>(stop - start);
        qInfo() << "<<<<<<<<<<<<<<<<<<<<<<<<";

        qInfo() << "std variant:" << duration.count();
        qInfo() << "Naive variant:" << duration1.count();
    }
    exit(0);

    return a.exec();
}
