#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

template<typename T>
struct Node;

template<typename T>
using NodePtr = std::unique_ptr<Node<T>>;

template<typename T>
struct Node
{
    Node(const T* value, uint weight, uint accumulatedChildWeights, const std::vector<NodePtr<T>>& childNodes)
        : value_(value)
        , weight_(weight)
        , accumulatedChildWeights_(accumulatedChildWeights)
        , childNodes_(childNodes)
    {

    }

public:
    Node<T>* tryAdd(const T* value)
    {
        ++accumulatedChildWeights_;

        auto childItr = std::find_if(childNodes_.begin(), childNodes_.end(), [value](const auto& rhs )
        {
            return rhs->value_ == value;
        });
        if ( childItr != childNodes_.end() )
            (*childItr)->weight_ += 1;
        else
        {
            childNodes_.emplace_back( new Node<T>(value, 1, 0, std::vector<NodePtr<T>>{}) );
            childItr = std::prev(childNodes_.end());
        }

        return *childItr;
    }



    Node<T>* selectNode(size_t idx) const
    {
        return childNodes_.at(idx);
    }

    Node<T>* selectNodeByWeight(size_t weight) const
    {
        size_t idx = 0;
        size_t accumulatedWeight = 0;

        std::cout << __func__ << " weight: " << weight << " accumulatedChildWeights_: " << accumulatedChildWeights_ << " childNodes_.size(): " << childNodes_.size() << std::endl;

        for (auto itr = childNodes_.begin(); itr != childNodes_.end(); ++itr)
        {
            std::cout << "\t" << " idx: " << idx << " accumulatedWeight: " << accumulatedWeight << std::endl;
            accumulatedWeight += childNodes_.at(idx)->weight_;
            if (accumulatedWeight >= weight)
                break;
            idx++;
        }

        std::cout << __func__ << " idx: " << idx << " childNodes_.size(): " << childNodes_.size() << std::endl;
        if (idx >= childNodes_.size())
            return nullptr;

        return childNodes_.at(idx);
    }

    void printNode(uint t)
    {
        for (int i = 0; i != t; ++i)
            std::cout << "  ";
        std::cout << "{\n";
        t++;
        for (int i = 0; i != t; ++i)
            std::cout << "  ";
        std::cout << "value_: " <<value_ << ":" << (value_ ? std::to_string(*(value_)) : "null" ) << " "
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

public:
    const T* value_;
    uint weight_ = 1;

    uint accumulatedChildWeights_ = 0;
    std::vector<NodePtr<T>> childNodes_;
};
