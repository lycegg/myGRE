//
// Created by florian on 18.11.15.
//

#ifndef ARTVERSION1_TREE_H
#define ARTVERSION1_TREE_H
#include "N.h"

using namespace ART;

namespace ART_unsynchronized {

    class Tree {
    public:
        using LoadKeyFunction = void (*)(TID tid, Key &key);

    private:

        N *const root;

        TID checkKey(const TID tid, const Key &k) const;

        LoadKeyFunction loadKey;

        enum class CheckPrefixResult : uint8_t {
            Match,
            NoMatch,
            OptimisticMatch
        };

        enum class CheckPrefixPessimisticResult : uint8_t {
            Match,
            NoMatch,
        };

        enum class PCCompareResults : uint8_t {
            Smaller,
            Equal,
            Bigger,
        };
        enum class PCEqualsResults : uint8_t {
            StartMatch,
            BothMatch,
            Contained,
            NoMatch,
        };
        static CheckPrefixResult checkPrefix(N* n, const Key &k, uint32_t &level);

        static CheckPrefixPessimisticResult checkPrefixPessimistic(N *n, const Key &k, uint32_t &level,
                                                                   uint8_t &nonMatchingKey,
                                                                   Prefix &nonMatchingPrefix,
                                                                   LoadKeyFunction loadKey);

        static PCCompareResults checkPrefixCompare(N* n, const Key &k, uint32_t &level, LoadKeyFunction loadKey);

        static PCEqualsResults checkPrefixEquals(N* n, uint32_t &level, const Key &start, const Key &end, LoadKeyFunction loadKey);

    public:

        Tree(LoadKeyFunction loadKey);

        Tree(const Tree &) = delete;

        Tree(Tree &&t) : root(t.root), loadKey(t.loadKey) { }

        ~Tree();

        TID lookup(const Key &k) const;

        bool lookupRange(const Key &start, const Key &end, Key &continueKey, TID result[], std::size_t resultLen,
                         std::size_t &resultCount) const;

        void insert(const Key &k, TID tid);

        bool update(const Key &k, TID tid);

        void remove(const Key &k);

        long size();

        void cenggao();

        void dfs(N* nd,int depth);

        long long num_leaf,len_path;

        template <typename Iterator>
            void bulk_load(Iterator ibegin, Iterator iend);

        

        template <typename Iterator>
            N* lyc_dfs(Prefix pre,Iterator&icur, Iterator &iend,int depth);    
        template <typename Iterator>
            Key getkey(Iterator &icur);
    };
}
#endif //ARTVERSION1_SYNCHRONIZEDTREE_H
