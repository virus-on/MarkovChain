#pragma once
#include <QtContainerFwd>
#include <QString>
#include <QRandomGenerator>
#include <QDateTime>
#include <QtDebug>

#include <math.h>

template<typename T, int maxWindowSize>
class Dict {
public:
    /// CTORS
    Dict(T startToken, T endToken)
        : random_ (QDateTime::currentMSecsSinceEpoch())
        , startToken_(startToken)
        , endToken_(endToken) { }
    Dict(T startToken, T endToken, const QList<T>& inputData)
        : random_ (QDateTime::currentMSecsSinceEpoch())
        , startToken_(startToken)
        , endToken_(endToken)
    {
        AddData(inputData);
    }

    /// Public functions
    T GetNextToken(const T& currentToken)
    {
        if (firstLvlDataMap_.contains(currentToken))
        {
            int totalIdxCount = 0;
            for (const uint& newToken: firstLvlDataMap_.value(currentToken))
            {
                totalIdxCount += newToken;
            }
            int idx = GetRandomIdx(totalIdxCount);
            qDebug() << "Token:" << currentToken << "It's values:" << firstLvlDataMap_[currentToken] << "total idx Count:" << totalIdxCount << "idx:" << idx;

            totalIdxCount = 0;
            for (auto it = firstLvlDataMap_[currentToken].begin(); it != firstLvlDataMap_[currentToken].end(); it++)
            {
                totalIdxCount += it.value();

                if ((it + 1) == firstLvlDataMap_[currentToken].end())
                    return it.key();
                else if (idx < totalIdxCount)
                    return it.key();
            }
        }
        else
        {
            throw std::runtime_error("No such token in DataMap");
        }
    }

    T GetNextToken(const QList<T>& currentTokenList);

    T GetFirstToken()
    {
        return GetNextToken(startToken_);
    }

    void    AddData(const QList<T>& inputData)
    {
        if ( firstLvlDataMap_.contains(startToken_) )
            firstLvlDataMap_[startToken_].insert(*(inputData.begin()), 1);
        else
            firstLvlDataMap_.insert(startToken_, {{*(inputData.begin()), 1}});

        qDebug() << "Inserting to start:" << *(inputData.begin());
        for (auto it = inputData.begin(); it != inputData.end(); it++)
        {
            if (firstLvlDataMap_.contains(*it))
            {
                auto nextElement = it + 1;
                if (nextElement != inputData.end())
                {
                    if (firstLvlDataMap_[*it].contains(*nextElement))
                        firstLvlDataMap_[*it][*nextElement] += 1;
                    else
                        firstLvlDataMap_[*it].insert(*nextElement, 1);
                }
                else
                {
                    if (firstLvlDataMap_[*it].contains(endToken_))
                        firstLvlDataMap_[*it][endToken_] += 1;
                    else
                        firstLvlDataMap_[*it].insert(endToken_, 1);
                }
            }
            else
            {
                auto nextElement = it + 1;
                if (nextElement != inputData.end())
                {
                    firstLvlDataMap_.insert(*it, {{*nextElement, 1}});
                }
                else
                {
                    firstLvlDataMap_.insert(*it, {{endToken_, 1}});
                }
            }
        }
    }

private:
    QString GetMd5Hash(const QList<T>& tokenList)
    {
        return "";
    }

    int     GetRandomIdx(int indexCount)
    {
        return std::lround(std::ceil(random_.bounded(100.) / 100. * indexCount));
    }

private:
    QRandomGenerator                                random_;
    QMap<T, QMap<T, uint>>                          firstLvlDataMap_;
    QVector<QHash<QString, QList<T>>>               multiLvlDataRefMap_;
    QVector<QMap<QString, QMap<T, uint>>>           multiLvlDataMap_;

    const T startToken_;
    const T endToken_;
};
