#ifndef _HUFFMAN_UTILS_H_
#define _HUFFMAN_UTILS_H_
#include <bitset>
#include <iostream>
#include <queue>
#include <map>
#include <climits> // for CHAR_BIT
#include <iterator>
#include <algorithm>
#include "sqeazy_common.hpp"

namespace sqeazy {
/* the below is adapted from http://rosettacode.org/wiki/Huffman_codes */

template <typename T>
struct huffman_scheme {

    typedef T raw_type;
    typedef T compressed_type;

    static const std::string name() {
      
      return std::string("huffman");
      
    }
    
    static const bool is_compressor = false;
    static const int num_bits = (CHAR_BIT*sizeof(T));
    static const unsigned long UniqueSymbols = 1 << (num_bits);

    
    typedef std::bitset<num_bits> HuffCode;
    typedef std::map<char, HuffCode> HuffCodeMap;


    static HuffCodeMap global_map;

    class INode
    {
    public:
        const int f;

        virtual ~INode() {}

    protected:
        INode(int f) : f(f) {}
    };

    class InternalNode : public INode
    {
    public:
        INode *const left;
        INode *const right;

        InternalNode(INode* c0, INode* c1) : INode(c0->f + c1->f), left(c0), right(c1) {}

        ~InternalNode()
        {
            delete left;
            delete right;
        }
    };

    class LeafNode : public INode
    {
    public:
        const raw_type c;

        LeafNode(int f, raw_type c) : INode(f), c(c) {}
    };

    struct NodeCmp
    {
        bool operator()(const INode* lhs, const INode* rhs) const {
            return lhs->f > rhs->f;
        }
    };

    typedef std::priority_queue<INode*, std::vector<INode*>, NodeCmp>  HuffTree;

    //  static INode* BuildTree(const int (&frequencies)[UniqueSymbols])
    template <typename fType>
    static const INode* BuildTree(const fType* frequencies)
    {
        HuffTree trees;
        for (unsigned i = 0; i < UniqueSymbols; ++i)
        {
            if(frequencies[i] != 0) {
                trees.push(new LeafNode(frequencies[i], (raw_type)i));//memory leak!

            }
        }
        while (trees.size() > 1)
        {
            INode* childR = trees.top();
            trees.pop();

            INode* childL = trees.top();
            trees.pop();

            INode* parent = new InternalNode(childR, childL);
            trees.push(parent);
        }

        return trees.top();
    }

    static const void GenerateCodes(const INode* node, const HuffCode& prefix, HuffCodeMap& outCodes)
    {
        if (const LeafNode* lf = dynamic_cast<const LeafNode*>(node))
        {
            outCodes[lf->c] = prefix;
        }
        else if (const InternalNode* in = dynamic_cast<const InternalNode*>(node))
        {
            HuffCode leftPrefix = prefix;
            leftPrefix <<= 1;
            leftPrefix.set(0,false);
            GenerateCodes(in->left, leftPrefix, outCodes);

            HuffCode rightPrefix = prefix;
            rightPrefix <<= 1;
            rightPrefix.set(0,true);
            GenerateCodes(in->right, rightPrefix, outCodes);
        }
    }

    template <typename SizeType>
    static const error_code encode(const raw_type* _in, compressed_type* _output,
                                   SizeType& _size,
                                   HuffCodeMap& _out_map = global_map) {

	histogram<raw_type> h_in(_in, _in + _size);
//         int frequencies[UniqueSymbols] = {0};
//         const raw_type* ptr = _in;
//         while (*ptr != '\0')
//             ++frequencies[(int)*ptr++];


        const INode* root = BuildTree(&h_in.bins[0]);

        GenerateCodes(root, HuffCode(), _out_map);

        const raw_type* ptr = _in;
        const raw_type* ptr_e = _in + _size;
	compressed_type* optr = _output;
        for(; ptr!=ptr_e; ++ptr) {
            *optr++ = (compressed_type)_out_map[*ptr].to_ulong();
        }

        delete root;


        return SUCCESS;


    }

    template <typename SizeType>
    static const error_code encode(const raw_type* _in, compressed_type* _output,
                                   std::vector<SizeType>& _size,
                                   HuffCodeMap& _out_map = global_map) {

        unsigned long scalar_size = std::accumulate(_size.begin(), _size.end(), 1, std::multiplies<SizeType>());

        return encode(_in, _output, scalar_size, _out_map);

    }


    template <typename SizeType>
    static const error_code decode(const compressed_type* _in, raw_type* _output,
                                   std::vector<SizeType>& _size,
                                   HuffCodeMap& _in_map = global_map) {

        return NOT_IMPLEMENTED_YET;

    }

    template <typename SizeType>
    static const error_code decode(const compressed_type* _in, raw_type* _output,
                                   SizeType& _size,
                                   HuffCodeMap& _in_map = global_map) {

        return NOT_IMPLEMENTED_YET;

    }

};

template <typename T>
typename huffman_scheme<T>::HuffCodeMap huffman_scheme<T>::global_map = huffman_scheme<T>::HuffCodeMap();

};


#endif /* _HUFFMAN_UTILS_H_ */
