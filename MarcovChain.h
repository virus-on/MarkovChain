#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include <memory>
#include <random>
#include <iostream>


namespace MarcovChain
{
    namespace __
    {
        template<typename T>
        struct Node;

        template<typename T>
        using NodePtr = std::unique_ptr<Node<T>>;

        template<typename T>
        struct Node
        {
            Node() = delete;
            Node(const Node<T>&) = delete;

            ~Node() = default;

            Node(const T* value, uint weight)
                : value_(value)
                , weight_(weight)
            {}

        public:
            Node<T>* tryAdd(const T* value)
            {
                ++accumulatedChildWeights_;

                auto childItr = std::find_if(childNodes_.begin(), childNodes_.end(), [value](const auto& rhs )
                {
                    return rhs->value_ == value;
                });
                if ( childItr == childNodes_.end() )
                {
                    childNodes_.emplace_back( std::make_unique<Node<T>>(value, 0) );
                    childItr = std::prev(childNodes_.end());
                }
                (*childItr)->weight_ += 1;

                return (*childItr).get();
            }

            const Node<T>* selectNode(size_t idx)
            {
                return childNodes_.at(idx);
            }

            const Node<T>* selectNodeByWeight(size_t weight)
            {
                size_t idx = 0;
                size_t accumulatedWeight = 0;

                for (auto itr = childNodes_.begin(); itr != childNodes_.end(); ++itr)
                {
                    accumulatedWeight += childNodes_.at(idx)->weight_;
                    if (accumulatedWeight >= weight)
                        break;
                    idx++;
                }

                if (idx >= childNodes_.size())
                    return nullptr;

                return childNodes_.at(idx).get();
            }

#ifdef DEBUG
            void printNode(uint t)
            {
                for (int i = 0; i != t; ++i)
                    std::cout << "  ";
                std::cout << "{\n";
                t++;
                for (int i = 0; i != t; ++i)
                    std::cout << "  ";
                std::cout << "value_: " << (value_ ? value_->toStdString() : "null" ) << " "
                          << "weight_: " << weight_ << " "
                          << "accumulatedChildWeights_: " << accumulatedChildWeights_ << " "
                          << "childNodes_.size(): " << childNodes_.size();
                t--;
                if (childNodes_.size() > 0)
                {
                    std::cout << " childs: \n";
                    for (const auto& c: childNodes_)
                        c->printNode(t + 1);
                }
                else
                {
                    std::cout << '\n';
                }

                for (int i = 0; i != t; ++i)
                    std::cout << "  ";
                std::cout << "}" << std::endl;
            }
#endif

        public:
            const T* value_ = nullptr;
            uint weight_ = 0;

            uint accumulatedChildWeights_ = 0;
            std::vector<NodePtr<T>> childNodes_;
        };
    }

    template <typename T, uint maxDepth_ = 2>
    class Dictionary
    {
    public:
        using output_value_type = const T*;

    private:
        static constexpr uint maxWindowSize_ = maxDepth_ - 1;

        std::set<T> uniqueTokens_;
        std::vector<__::NodePtr<T>> dict_;

        std::random_device rd_;
        std::mt19937 gen_; // seed the generator

    public:
        Dictionary()
            : rd_("/dev/urandom")
            , gen_(rd_())
        {
            static_assert (maxDepth_ > 1, "maxDepth can't be < 2");
        }

    public:
        template <typename iteratorType>
        void addDataToDictionary( iteratorType dataItBegin, iteratorType dataItEnd)
        {
            addDataToDictionary<iteratorType>(dataItBegin, dataItEnd, maxDepth_, false);
        }

        template <typename Container>
        const typename Container::value_type getToken()
        {
            return getFirstToken();
        }

        template <typename Container, uint windowSize = maxWindowSize_>
        output_value_type getToken(typename Container::iterator tokenItBegin, typename Container::iterator tokenItEnd)
        {
            static_assert (maxWindowSize_ >= windowSize, "Depth can't be greater then maximum depth value");
            if ( windowSize == 0 || tokenItBegin == tokenItEnd )
                return getFirstToken<Container>();

            // shift for appropriate window parameters
            if (std::distance(tokenItBegin, tokenItEnd) > windowSize)
                tokenItBegin = std::prev(tokenItEnd, windowSize);

            auto dictNodeItr = std::find_if(dict_.cbegin(), dict_.cend(), [value = *tokenItBegin](const auto& rhs ){
                return rhs->value_ == value;
            });

            // if nothing found, suggest we reached the end
            if (dictNodeItr == dict_.cend())
                return nullptr;

            ++tokenItBegin;
            for (; tokenItBegin != tokenItEnd; ++tokenItBegin)
            {
                const auto& childNodes = cItrToRawPtr(dictNodeItr)->childNodes_;
                auto childNodeItr = std::find_if(childNodes.begin(), childNodes.end(), [&tokenItBegin](const auto& rhs){
                    return rhs->value_ == *tokenItBegin;
                });

                // No such value, end reached
                if (childNodeItr == childNodes.end())
                    return nullptr;

                dictNodeItr = childNodeItr;
            }

            auto node = itrToRawPtr(dictNodeItr)->selectNodeByWeight( getRandomWeight(itrToRawPtr(dictNodeItr)->accumulatedChildWeights_) );
            if (node)
                return node->value_;
            else
                return nullptr;
        }

        template <typename Container>
        const typename Container::value_type getFirstToken()
        {
            auto dictBeginNodeItr = std::find_if(dict_.cbegin(), dict_.cend(), [](const auto& rhs ){
                return rhs->value_ == nullptr;
            });
            if (dictBeginNodeItr == dict_.cend())
                return nullptr;

            auto beginNode = itrToRawPtr(dictBeginNodeItr);
            return beginNode->selectNodeByWeight(
                        getRandomWeight(beginNode->accumulatedChildWeights_)
                        )->value_;
        }

#ifdef DEBUG
        void printInternals()
        {
            std::cout << "uniqueTokens_: ";
            for (const auto& v: uniqueTokens_)
                std::cout << v.toStdString() << " ";
            std::cout << '\n';

            std::cout << "dict_:";
            for (const auto& v: dict_)
            {
                v->printNode(0);
            }
        }
#endif

    private:
        template <typename iteratorType>
        void addDataToDictionary(iteratorType dataItBegin, iteratorType dataItEnd, uint maxDepth, bool inRecursiveCall)
        {
            // if not in recursive call, add or update BEGIN token node
            if (not inRecursiveCall)
            {
                auto dictBeginNodeItr = std::find_if(dict_.begin(), dict_.end(), [](const auto& rhs ){
                    return (*rhs).value_ == nullptr;
                });

                if (dictBeginNodeItr == dict_.end())
                {
                    dict_.emplace_back( std::make_unique<__::Node<T>>( nullptr, 1) );
                    dictBeginNodeItr = std::prev(dict_.end());
                }
                else
                {
                    itrToRawPtr(dictBeginNodeItr)->weight_ += 1;
                }

                buildNodes<iteratorType>(dictBeginNodeItr, dataItBegin, dataItEnd, maxDepth - 1);
            }

            // iterate through input values
            for (; dataItBegin != dataItEnd; ++dataItBegin)
            {
                auto result = uniqueTokens_.insert( *dataItBegin );

                // if insert took place
                // add value to dict
                typename decltype(dict_)::const_iterator dictItr;
                if (result.second)
                {
                    // get token ptr from uniqueTokens_
                    dict_.push_back( std::make_unique<__::Node<T>>( cItrValToRawPtr(result.first), inRecursiveCall ? 0 : 1) );
                    dictItr = std::prev(dict_.cend());
                }
                // else find dict value for current token
                else
                {
                    dictItr = std::find_if(dict_.cbegin(), dict_.cend(), [token = result.first, this](const auto& rhs ){
                        return (*rhs).value_ == cItrValToRawPtr(token);
                    });
                    if (not inRecursiveCall)
                    {
                        itrToRawPtr(dictItr)->weight_ += 1;
                    }
                }

                if (not inRecursiveCall)
                    buildNodes<iteratorType>(dictItr, dataItBegin + 1, dataItEnd, maxDepth - 1);
            } // end building dict
        }

        template <typename iteratorType>
        void buildNodes(typename decltype(dict_)::const_iterator dictItr, iteratorType dataItBegin, iteratorType dataItEnd, uint maxDepth = maxDepth_)
        {
            // increment weight
            __::Node<T>* internalNodeP = itrToRawPtr(dictItr);
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
                        addDataToDictionary<iteratorType>(nextTokenIt, nextTokenIt + 1, maxDepth_, true);
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
            return randNum;
        }

        template <typename U>
        constexpr __::Node<T>* itrToRawPtr(U& itr)
        {
            return (*itr).get();
        }

        template <typename U>
        constexpr const __::Node<T>* cItrToRawPtr(U& itr)
        {
            return (*itr).get();
        }

        template <typename U>
        constexpr const T* cItrValToRawPtr(U& itr)
        {
            return &(*itr);
        }
    };
}
