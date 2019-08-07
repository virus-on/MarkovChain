#pragma once
#include <QtContainerFwd>
#include <QString>
#include <QRandomGenerator>
#include <QCryptographicHash>
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
        if (not firstLvlDataMap_.contains(currentToken))
        {
            throw std::runtime_error("No such token in DataMap");
        }

        int totalIdxCount = 0;
        for (const uint& newToken: firstLvlDataMap_[currentToken].values())
        {
            totalIdxCount += newToken;
        }
        int idx = GetRandomIdx(totalIdxCount);

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

    T GetNextToken(const QList<T>& currentTokenList)
    {
        int windowSize = currentTokenList.length();
        if (windowSize == 0 || windowSize > maxWindowSize)
            throw std::runtime_error("Requested window size is greater then maximum!");
        if (windowSize == 1)
            return GetNextToken(currentTokenList[0]);
        if (not multiLvlDataMap_[windowSize - 2].contains(currentTokenList))
            throw std::runtime_error("No such token in DataMap");

        int totalIdxCount = 0;
        for (const uint& newToken: multiLvlDataMap_[windowSize - 2][currentTokenList].values())
        {
            totalIdxCount += newToken;
        }
        int idx = GetRandomIdx(totalIdxCount);
        totalIdxCount = 0;
        for (auto it = multiLvlDataMap_[windowSize - 2][currentTokenList].begin(); it != multiLvlDataMap_[windowSize - 2][currentTokenList].end(); it++)
        {
            totalIdxCount += it.value();

            if (idx < totalIdxCount || (it + 1) == multiLvlDataMap_[windowSize - 2][currentTokenList].end())
            {
                return it.key();
            }
        }
    }

    T GetFirstToken()
    {
        return GetNextToken(startToken_);
    }

    void    AddData(const QList<T>& inputData)
    {
        // Build First lvl dict
        if ( firstLvlDataMap_.contains(startToken_) )
            firstLvlDataMap_[startToken_].insert(*(inputData.begin()), 1);
        else
            firstLvlDataMap_.insert(startToken_, {{*(inputData.begin()), 1}});

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

        // Build second lvl of dict
        if (maxWindowSize > 1)
        {
            // Get all keys from lvl1 dict
            const QList<T>& FirstLvlKeys = firstLvlDataMap_.keys();

            // if we have no lvl2 dict data - add it
            if (multiLvlDataMap_.length() == 0)
            {
                QMap<QList<T>, QMap<T, uint>> dataMap;
                multiLvlDataMap_.append(dataMap);
            }

            // use existing dict data by reference
            QMap<QList<T>, QMap<T, uint>>& dataMapRef = multiLvlDataMap_[0];

            // go around all keys...
            for (const auto& key: FirstLvlKeys)
            {
                if (key == startToken_ || key == endToken_)
                    continue;
                // ... and all of it's possible extensions
                QList<T> allPossiableExtensions = firstLvlDataMap_[key].keys();
                for (const auto& extension: allPossiableExtensions)
                {
                    // create new data cortage consisting of existing data and it's extension
                    // if it is unique add it's md5 hash to reference table.
                    QList<T> dataCortege = {key, extension};

                    // go across all data and search for a new tokens to add to lvl2 dict
                    for (auto iter = inputData.begin(); iter != inputData.end(); iter++)
                    {
                        if ((iter + 1) == inputData.end()) // no more data
                            break;

                        if (*iter == key && *(iter + 1) == extension) // GOTCHA!
                        {
                            T nextToken;

                            if ((iter + 2) == inputData.end() && extension != endToken_)  // End of tokens reached
                                nextToken = endToken_;
                            else if ((iter + 2) == inputData.end() && extension == endToken_)
                                continue;
                            else                                // Or we have real token?
                                nextToken = *(iter + 2);

                            if (dataMapRef.contains(dataCortege))    // dataMap already contains record with such id
                            {
                                // just add data referencing it
                                if (dataMapRef[dataCortege].contains(nextToken))
                                    dataMapRef[dataCortege][nextToken] += 1;         // update token count
                                else
                                    dataMapRef[dataCortege].insert(nextToken, 1);    // add new record for such token
                            }
                            else                                            // new unique id found
                            {
                                // insert first record for id reference to this data map
                                dataMapRef.insert(dataCortege, {{nextToken, 1}});
                            }
                        }
                    }
                }
            }
        }

        // Dict gen for all windows greater then lvl2
        for (int windowSize = 3; windowSize <= maxWindowSize; windowSize++)
        {
            const QList<QList<T>>& keyList = multiLvlDataMap_[windowSize - 3].keys();
            if (multiLvlDataMap_.length() <= windowSize - 2)
            {
                QMap<QList<T>, QMap<T, uint>> dataMap;
                multiLvlDataMap_.append(dataMap);
            }

            // use existing dict data by reference
            QMap<QList<T>, QMap<T, uint>>& dataMapRef = multiLvlDataMap_[windowSize - 2];
            for (const auto& key: keyList)
            {
                QList<T> allPossiableExtensions = multiLvlDataMap_[windowSize - 3][key].keys();
                for (const auto& extension: allPossiableExtensions)
                {
                    if (extension == endToken_)
                        continue;
                    QList<T> newTokenDataCortege = key;
                    newTokenDataCortege.append(extension);

                    // go across all data and search for a new tokens to add to lvl2 dict
                    for (auto iter = inputData.begin(); iter != inputData.end(); iter++)
                    {
                        auto testDataChains = [windowSize, &newTokenDataCortege](auto itID)
                        {
                            for (int i = 0; i < newTokenDataCortege.length(); i++)
                            {
                                if (newTokenDataCortege[i] != *(itID + i))
                                    return false;
                            }
                            return true;
                        };

                        if ((iter + (windowSize - 1)) == inputData.end()) // no more data
                            break;
                        if (testDataChains(iter)) // GOTCHA!
                        {
                            T nextToken;
                            if ((iter + windowSize) == inputData.end()) // End of tokens reached
                                nextToken = endToken_;
                            else                                        // Or we have real token?
                                nextToken = *(iter + windowSize);

                            if (dataMapRef.contains(newTokenDataCortege))    // dataMap already contains record with such id
                            {
                                // just add data referencing it
                                if (dataMapRef[newTokenDataCortege].contains(nextToken))
                                    dataMapRef[newTokenDataCortege][nextToken] += 1;         // update token count
                                else
                                    dataMapRef[newTokenDataCortege].insert(nextToken, 1);    // add new record for such token
                            }
                            else                                            // new unique id found
                            {
                                // insert first record for id reference to this data map
                                dataMapRef.insert(newTokenDataCortege, {{nextToken, 1}});
                            }
                        }
                    }
                }
            }
        }
    }

private:
    int     GetRandomIdx(int indexCount)
    {
        return std::lround(std::ceil(random_.bounded(100.) / 100. * indexCount));
    }

private:
    QRandomGenerator                                random_;
    QMap<T, QMap<T, uint>>                          firstLvlDataMap_;
    QVector<QMap<QList<T>, QMap<T, uint>>>          multiLvlDataMap_;

    const T startToken_;
    const T endToken_;
};
