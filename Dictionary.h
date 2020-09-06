#pragma once

#include "Node.h"

#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <iostream>

template <typename T, uint maxDepth_ = 1>
class Dictionary
{
private:
    std::set<T> uniqueTokens_;
    std::vector<NodePtr<T>> dict_;

    std::random_device rd_;
    std::mt19937 gen_; // seed the generator

public:
    Dictionary()
        : rd_("/dev/urandom")
        , gen_(rd_())
    {

    }

public:
    template <typename Container>
    void addDataToDictionary( typename Container::iterator dataItBegin, typename Container::iterator dataItEnd, uint maxDepth = maxDepth_)
    {
        if (maxDepth > maxDepth_)
            throw std::out_of_range("Depth can't be greater then maximum value");
        addDataToDictionary<Container>(dataItBegin, dataItEnd, maxDepth, false);
    }

    template <typename Container>
    const typename Container::value_type getToken()
    {
        return getFirstToken();
    }

    template <typename Container>
    const typename Container::value_type getToken(typename Container::iterator tokenItBegin, typename Container::iterator tokenItEnd, uint maxDepth = maxDepth_)
    {
        std::cout << __func__ << std::endl;
        if (maxDepth > maxDepth_)
            throw std::out_of_range("Depth can't be greater then maximum depth value");

        // if empty range, return first token
        if (tokenItBegin == tokenItEnd)
            return getFirstToken<Container>();

        // shift for appropriate window parameters
        if (std::distance(tokenItBegin, tokenItEnd) > maxDepth)
            tokenItBegin = std::prev(tokenItEnd, maxDepth);

        auto dictNodeItr = std::find_if(dict_.cbegin(), dict_.cend(), [value = *tokenItBegin](const auto& rhs ){
            return rhs->value_ == value;
        });

        // if nothing found, suggest we reached the end
        if (dictNodeItr == dict_.cend())
            return nullptr;

        ++tokenItBegin;

        for (tokenItBegin; tokenItBegin != tokenItEnd; ++tokenItBegin)
        {
            const auto& childNodes = (*dictNodeItr)->childNodes_;
            auto childNodeItr = std::find_if(childNodes.begin(), childNodes.end(), [&tokenItBegin](const auto& rhs){
                return rhs->value_ == *tokenItBegin;
            });

            // No such value, end reached
            if (childNodeItr == childNodes.end())
                return nullptr;

            dictNodeItr = childNodeItr;
        }

        auto node = cItrToRawPtr(dictNodeItr)->selectNodeByWeight( getRandomWeight(cItrToRawPtr(dictNodeItr)->accumulatedChildWeights_) );
        if (node)
            return node->value_;
        else
            return nullptr;
    }

    void printNodes()
    {
        std::cout << "data_: ";
        for (const auto& v: uniqueTokens_)
            std::cout << v << " ";
        std::cout << '\n';

        std::cout << "dict_:";
        for (const auto& v: dict_)
        {
            v->printNode(0);
        }
    }

private:
    template <typename Container>
    const typename Container::value_type getFirstToken()
    {
        std::cout << __func__ << std::endl;
        auto dictBeginNodeItr = std::find_if(dict_.begin(), dict_.end(), [](const auto rhs ){
            return rhs->value_ == nullptr;
        });

        auto beginNode = *dictBeginNodeItr;

        return beginNode->selectNodeByWeight(
                    getRandomWeight(beginNode->accumulatedChildWeights_)
                    )->value_;
    }

    template <typename Container>
    void addDataToDictionary(typename Container::iterator dataItBegin, typename Container::iterator dataItEnd, uint maxDepth, bool inRecursiveCall)
    {
        // if not in recursive call, add or update BEGIN token node
        if (not inRecursiveCall)
        {
            auto dictBeginNodeItr = std::find_if(dict_.begin(), dict_.end(), [](const auto& rhs ){
                return (*rhs).value_ == nullptr;
            });

            if (dictBeginNodeItr == dict_.end())
            {
                dict_.emplace_back( new Node<T>( nullptr, 0, 0, std::vector<NodePtr<T>>{}) );
                dictBeginNodeItr = std::prev(dict_.end());
            }
            (*dictBeginNodeItr)->weight_ += 1;

            buildNodes<Container>(dictBeginNodeItr, dataItBegin, dataItEnd, maxDepth - 1);
        }

        // iterate through input values
        for (dataItBegin; dataItBegin != dataItEnd; ++dataItBegin)
        {
            auto result = uniqueTokens_.insert( *dataItBegin );

            // if insert took place
            // add value to dict
            typename decltype(dict_)::iterator dictItr;
            if (result.second)
            {
                // get value ptr from data_
                dict_.emplace_back( new Node<T>( &(*result.first), 0, 0, std::vector<NodePtr<T>>{}) );
                dictItr = std::prev(dict_.end());
            }

            // else find dict value for current token
            else
                dictItr = std::find_if(dict_.begin(), dict_.end(), [token = result.first](const auto& rhs ){
                    return (*rhs).value_ == &(*token);
                });

            buildNodes<Container>(dictItr, dataItBegin + 1, dataItEnd, maxDepth - 1);

        } // end building dict
    }

    template <typename Container>
    void buildNodes(typename decltype(dict_)::iterator dictItr, typename Container::iterator dataItBegin, typename Container::iterator dataItEnd, uint maxDepth = maxDepth_)
    {
        // increment weight
        Node<T>* internalNodeP = itrToRawPtr(dictItr);
        internalNodeP->weight_ += 1;
        for (uint i = 0; i < maxDepth; ++i)
        {
            auto nextTokenIt = dataItBegin + i;

            // if not at the end of added data, add this token to dict node
            if (nextTokenIt != dataItEnd)
            {
                auto tokenDataIt = std::find(uniqueTokens_.cbegin(), uniqueTokens_.cend(), *nextTokenIt);
                if (tokenDataIt != uniqueTokens_.cend())
                {
                    internalNodeP = internalNodeP->tryAdd( cItrValToRawPtr(tokenDataIt) );
                }
                else
                {
                    // recursive call to add token's which is not in uniqueTokens_ already
                    addDataToDictionary<Container>(nextTokenIt, dataItEnd, maxDepth, true);
                    tokenDataIt = std::find(uniqueTokens_.begin(), uniqueTokens_.end(), *nextTokenIt);
                    internalNodeP = internalNodeP->tryAdd( cItrValToRawPtr(tokenDataIt) );
                }
            }
            else
            {
                // add END token
                internalNodeP->tryAdd(nullptr);
                break;
            }
        } // end iterative add child node
    }

    size_t getRandomWeight(uint totalWeight)
    {
        if (totalWeight < 1)
            return 1;
        std::uniform_int_distribution<> distr(1, totalWeight); // define the range
        auto randNum = distr(gen_);
        std::cout << __func__  << " randNum: " << randNum << " totalWeight: " << totalWeight << std::endl;
        return randNum;
    }

    template <typename U>
    constexpr NodePtr<T>& itrToRawPtr(U& itr)
    {
        return *itr;
    }

    template <typename U>
    constexpr const NodePtr<T>& cItrToRawPtr(U& itr)
    {
        return *itr;
    }

    template <typename U>
    constexpr const T* cItrValToRawPtr(U& itr)
    {
        return &(*itr);
    }
};
