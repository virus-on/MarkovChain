#include "MarcovChain.h"

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
        MarcovChain::Dictionary<QString, 3> dict;
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
                std::list<const QString*> genOutput;
                const QString* genVal =  nullptr;
                do
                {
                   genVal =  dict.getToken<decltype(genOutput)>(genOutput.begin(), genOutput.end());
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
        else {
            qCritical() << "Can't open the input file!";
            exit(0);
        }
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        qInfo() << "<<<<<<<<<<<<<<<<<<<<<<<<";
        qInfo() << "Program Duration:" << duration.count() << "ms";
    }
    exit(0);

    return a.exec();
}
