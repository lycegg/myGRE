#include <assert.h>
#include <algorithm>
#include <iostream>
#include "Tree.h"
#include "N.cpp"

namespace ART_unsynchronized {

    Tree::Tree(LoadKeyFunction loadKey) : root(new N256(nullptr, 0)), loadKey(loadKey) {
    }

    Tree::~Tree() {
        N::deleteChildren(root);
        N::deleteNode(root);
    }

    TID Tree::lookup(const Key &k) const {
        N *node = nullptr;
        N *nextNode = root;
        uint32_t level = 0;
        bool optimisticPrefixMatch = false;

        while (true) {
            node = nextNode;
            switch (checkPrefix(node, k, level)) { // increases level
                case CheckPrefixResult::NoMatch:
                    return 0;
                case CheckPrefixResult::OptimisticMatch:
                    optimisticPrefixMatch = true;
                    // fallthrough
                case CheckPrefixResult::Match:
                    if (k.getKeyLen() <= level) {
                        return 0;
                    }
                    nextNode = N::getChild(k[level], node);

                    if (nextNode == nullptr) {
                        return 0;
                    }
                    if (N::isLeaf(nextNode)) {
                        TID tid = N::getLeaf(nextNode);
                        if (level < k.getKeyLen() - 1 || optimisticPrefixMatch) {
                            return checkKey(tid, k);
                        }
                        return tid;
                    }
                    level++;
            }
        }
    }

    bool Tree::lookupRange(const Key &, const Key &, Key &, TID [],
                                std::size_t , std::size_t &) const {
        return false;
        /*for (uint32_t i = 0; i < std::min(start.getKeyLen(), end.getKeyLen()); ++i) {
            if (start[i] > end[i]) {
                resultsFound = 0;
                return false;
            } else if (start[i] < end[i]) {
                break;
            }
        }
        TID toContinue = 0;
        std::function<void(const N *)> copy = [&result, &resultSize, &resultsFound, &toContinue, &copy](const N *node) {
            if (N::isLeaf(node)) {
                if (resultsFound == resultSize) {
                    toContinue = N::getLeaf(node);
                    return;
                }
                result[resultsFound] = N::getLeaf(node);
                resultsFound++;
            } else {
                std::tuple<uint8_t, N *> children[256];
                uint32_t childrenCount = 0;
                N::getChildren(node, 0u, 255u, children, childrenCount);
                for (uint32_t i = 0; i < childrenCount; ++i) {
                    const N *n = std::get<1>(children[i]);
                    copy(n);
                    if (toContinue != 0) {
                        break;
                    }
                }
            }
        };
        std::function<void(const N *, uint32_t)> findStart = [&copy, &start, &findStart, &toContinue, &restart, this](
                const N *node, uint32_t level) {
            if (N::isLeaf(node)) {
                copy(node);
                return;
            }

            uint64_t v;
            PCCompareResults prefixResult;
            do {
                v = node->startReading();
                prefixResult = checkPrefixCompare(node, start, level, loadKey);
            } while (!node->stopReading(v));
            switch (prefixResult) {
                case PCCompareResults::Bigger:
                    copy(node);
                    break;
                case PCCompareResults::Equal: {
                    uint8_t startLevel = (start.getKeyLen() > level) ? start[level] : 0;
                    std::tuple<uint8_t, N *> children[256];
                    uint32_t childrenCount = 0;
                    N::getChildren(node, startLevel, 255, children, childrenCount);
                    for (uint32_t i = 0; i < childrenCount; ++i) {
                        const uint8_t k = std::get<0>(children[i]);
                        const N *n = std::get<1>(children[i]);
                        if (k == startLevel) {
                            findStart(n, level + 1);
                        } else if (k > startLevel) {
                            copy(n);
                        }
                        if (toContinue != 0 || restart) {
                            break;
                        }
                    }
                    break;
                }
                case PCCompareResults::SkippedLevel:
                    restart = true;
                    break;
                case PCCompareResults::Smaller:
                    break;
            }
        };
        std::function<void(const N *, uint32_t)> findEnd = [&copy, &end, &toContinue, &restart, &findEnd, this](
                const N *node, uint32_t level) {
            if (N::isLeaf(node)) {
                return;
            }
            uint64_t v;
            PCCompareResults prefixResult;
            do {
                v = node->startReading();
                prefixResult = checkPrefixCompare(node, end, level, loadKey);
            } while (!node->stopReading(v));

            switch (prefixResult) {
                case PCCompareResults::Smaller:
                    copy(node);
                    break;
                case PCCompareResults::Equal: {
                    uint8_t endLevel = (end.getKeyLen() > level) ? end[level] : 255;
                    std::tuple<uint8_t, N *> children[256];
                    uint32_t childrenCount = 0;
                    N::getChildren(node, 0, endLevel, children, childrenCount);
                    for (uint32_t i = 0; i < childrenCount; ++i) {
                        const uint8_t k = std::get<0>(children[i]);
                        const N *n = std::get<1>(children[i]);
                        if (k == endLevel) {
                            findEnd(n, level + 1);
                        } else if (k < endLevel) {
                            copy(n);
                        }
                        if (toContinue != 0 || restart) {
                            break;
                        }
                    }
                    break;
                }
                case PCCompareResults::Bigger:
                    break;
                case PCCompareResults::SkippedLevel:
                    restart = true;
                    break;
            }
        };

        restart:
        restart = false;
        resultsFound = 0;

        uint32_t level = 0;
        N *node = nullptr;
        N *nextNode = root;

        while (true) {
            node = nextNode;
            uint64_t v;
            PCEqualsResults prefixResult;
            do {
                v = node->startReading();
                prefixResult = checkPrefixEquals(node, level, start, end, loadKey);
            } while (!node->stopReading(v));
            switch (prefixResult) {
                case PCEqualsResults::SkippedLevel:
                    goto restart;
                case PCEqualsResults::NoMatch: {
                    return false;
                }
                case PCEqualsResults::Contained: {
                    copy(node);
                    break;
                }
                case PCEqualsResults::StartMatch: {
                    uint8_t startLevel = (start.getKeyLen() > level) ? start[level] : 0;
                    std::tuple<uint8_t, N *> children[256];
                    uint32_t childrenCount = 0;
                    N::getChildren(node, startLevel, 255, children, childrenCount);
                    for (uint32_t i = 0; i < childrenCount; ++i) {
                        const uint8_t k = std::get<0>(children[i]);
                        const N *n = std::get<1>(children[i]);
                        if (k == startLevel) {
                            findStart(n, level + 1);
                        } else if (k > startLevel) {
                            copy(n);
                        }
                        if (restart) {
                            goto restart;
                        }
                        if (toContinue) {
                            break;
                        }
                    }
                    break;
                }
                case PCEqualsResults::BothMatch: {
                    uint8_t startLevel = (start.getKeyLen() > level) ? start[level] : 0;
                    uint8_t endLevel = (end.getKeyLen() > level) ? end[level] : 255;
                    if (startLevel != endLevel) {
                        std::tuple<uint8_t, N *> children[256];
                        uint32_t childrenCount = 0;
                        N::getChildren(node, startLevel, endLevel, children, childrenCount);
                        for (uint32_t i = 0; i < childrenCount; ++i) {
                            const uint8_t k = std::get<0>(children[i]);
                            const N *n = std::get<1>(children[i]);
                            if (k == startLevel) {
                                findStart(n, level + 1);
                            } else if (k > startLevel && k < endLevel) {
                                copy(n);
                            } else if (k == endLevel) {
                                findEnd(n, level + 1);
                            }
                            if (restart) {
                                goto restart;
                            }
                            if (toContinue) {
                                break;
                            }
                        }
                    } else {
                        nextNode = N::getChild(startLevel, node);
                        if (!node->stopReading(v)) {
                            goto restart;
                        }
                        level++;
                        continue;
                    }
                    break;
                }
            }
            break;
        }
        if (toContinue != 0) {
            loadKey(toContinue, continueKey);
            return true;
        } else {
            return false;
        }*/
    }


    TID Tree::checkKey(const TID tid, const Key &k) const {
        Key kt;
        this->loadKey(tid, kt);
        if (k == kt) {
            return tid;
        }
        return 0;
    }

    void Tree::insert(const Key &k, TID tid) {
        N *node = nullptr;
        N *nextNode = root;
        N *parentNode = nullptr;
        uint8_t parentKey, nodeKey = 0;
        uint32_t level = 0;

        while (true) {
            parentNode = node;
            parentKey = nodeKey;
            node = nextNode;

            uint32_t nextLevel = level;

            uint8_t nonMatchingKey;
            Prefix remainingPrefix;
            switch (checkPrefixPessimistic(node, k, nextLevel, nonMatchingKey, remainingPrefix,
                                                           this->loadKey)) { // increases level
                case CheckPrefixPessimisticResult::NoMatch: {
                    assert(nextLevel < k.getKeyLen()); //prevent duplicate key
                    // 1) Create new node which will be parent of node, Set common prefix, level to this node
                    auto newNode = new N4(node->getPrefix(), nextLevel - level);
 
                    // 2)  add node and (tid, *k) as children
                    newNode->insert(k[nextLevel], N::setLeaf(tid));
                    newNode->insert(nonMatchingKey, node);

                    // 3) update parentNode to point to the new node
                    N::change(parentNode, parentKey, newNode);

                    // 4) update prefix of node
                    node->setPrefix(remainingPrefix,
                                    node->getPrefixLength() - ((nextLevel - level) + 1));

                    return;
                }
                case CheckPrefixPessimisticResult::Match:
                    break;
            }
            assert(nextLevel < k.getKeyLen()); //prevent duplicate key
            level = nextLevel;
            nodeKey = k[level];
            nextNode = N::getChild(nodeKey, node);

            if (nextNode == nullptr) {
                N::insertA(node, parentNode, parentKey, nodeKey, N::setLeaf(tid));
                return;
            }
            if (N::isLeaf(nextNode)) {
                Key key;
                loadKey(N::getLeaf(nextNode), key);

                level++;
                assert(level < key.getKeyLen()); //prevent inserting when prefix of key exists already
                uint32_t prefixLength = 0;
                while (key[level + prefixLength] == k[level + prefixLength]) {
                    prefixLength++;
                }

                auto n4 = new N4(&k[level], prefixLength);
                n4->insert(k[level + prefixLength], N::setLeaf(tid));
                n4->insert(key[level + prefixLength], nextNode);
                N::change(node, k[level - 1], n4);
                return;
            }

            level++;
        }
    }

    bool Tree::update(const Key &k, TID tid) {
        N *node = nullptr;
        N *nextNode = root;
        uint32_t level = 0;
        bool optimisticPrefixMatch = false;

        while (true) {
            node = nextNode;
            switch (checkPrefix(node, k, level)) { // increases level
                case CheckPrefixResult::NoMatch:
                    return false;
                case CheckPrefixResult::OptimisticMatch:
                    optimisticPrefixMatch = true;
                    // fallthrough
                case CheckPrefixResult::Match:
                    if (k.getKeyLen() <= level) {
                        return false;
                    }
                    nextNode = N::getChild(k[level], node);

                    if (nextNode == nullptr) {
                        return false;
                    }
                    if (N::isLeaf(nextNode)) {
                        TID old_tid = N::getLeaf(nextNode);
                        if (level < k.getKeyLen() - 1 || optimisticPrefixMatch) {
                            if (checkKey(old_tid, k) == old_tid) {
                                N::change(node, k[level], N::setLeaf(tid));
                                return true;
                            }
                        }
                        N::change(node, k[level], N::setLeaf(tid));
                        return true;
                    }
                    level++;
            }
        }
    }

    void Tree::remove(const Key &k) {
        N *node = nullptr;
        N *nextNode = root;
        N *parentNode = nullptr;
        uint8_t parentKey, nodeKey = 0;
        uint32_t level = 0;
        //bool optimisticPrefixMatch = false;

        while (true) {
            parentNode = node;
            parentKey = nodeKey;
            node = nextNode;

            switch (checkPrefix(node, k, level)) { // increases level
                case CheckPrefixResult::NoMatch:
                    return;
                case CheckPrefixResult::OptimisticMatch:
                    // fallthrough
                case CheckPrefixResult::Match: {
                    nodeKey = k[level];
                    nextNode = N::getChild(nodeKey, node);

                    if (nextNode == nullptr) {
                        return;
                    }
                    if (N::isLeaf(nextNode)) {
                        assert(parentNode == nullptr || node->getCount() != 1);
                        if (node->getCount() == 2 && node != root) {
                            // 1. check remaining entries
                            N *secondNodeN;
                            uint8_t secondNodeK;
                            std::tie(secondNodeN, secondNodeK) = N::getSecondChild(node, nodeKey);
                            if (N::isLeaf(secondNodeN)) {

                                //N::remove(node, k[level]); not necessary
                                N::change(parentNode, parentKey, secondNodeN);

                                // delete node;
                            } else {
                                //N::remove(node, k[level]); not necessary
                                N::change(parentNode, parentKey, secondNodeN);
                                secondNodeN->addPrefixBefore(node, secondNodeK);

                                // delete node;
                            }
                        } else {
                            N::removeA(node, k[level], parentNode, parentKey);
                        }
                        return;
                    }
                    level++;
                }
            }
        }
    }


    inline typename Tree::CheckPrefixResult Tree::checkPrefix(N *n, const Key &k, uint32_t &level) {
        if (k.getKeyLen() <= level + n->getPrefixLength()) {
            return CheckPrefixResult::NoMatch;
        }
        if (n->hasPrefix()) {
            for (uint32_t i = 0; i < std::min(n->getPrefixLength(), maxStoredPrefixLength); ++i) {
                if (n->getPrefix()[i] != k[level]) {
                    return CheckPrefixResult::NoMatch;
                }
                ++level;
            }
            if (n->getPrefixLength() > maxStoredPrefixLength) {
                level += n->getPrefixLength() - maxStoredPrefixLength;
                return CheckPrefixResult::OptimisticMatch;
            }
        }
        return CheckPrefixResult::Match;
    }

    typename Tree::CheckPrefixPessimisticResult Tree::checkPrefixPessimistic(N *n, const Key &k, uint32_t &level,
                                                                        uint8_t &nonMatchingKey,
                                                                        Prefix &nonMatchingPrefix,
                                                                        LoadKeyFunction loadKey) {
        if (n->hasPrefix()) {
            uint32_t prevLevel = level;
            Key kt;
            for (uint32_t i = 0; i < n->getPrefixLength(); ++i) {
                if (i == maxStoredPrefixLength) {
                    loadKey(N::getAnyChildTid(n), kt);
                }
                uint8_t curKey = i >= maxStoredPrefixLength ? kt[level] : n->getPrefix()[i];
                if (curKey != k[level]) {
                    nonMatchingKey = curKey;
                    if (n->getPrefixLength() > maxStoredPrefixLength) {
                        if (i < maxStoredPrefixLength) {
                            loadKey(N::getAnyChildTid(n), kt);
                        }
                        for (uint32_t j = 0; j < std::min((n->getPrefixLength() - (level - prevLevel) - 1),
                                                          maxStoredPrefixLength); ++j) {
                            nonMatchingPrefix[j] = kt[level + j + 1];
                        }
                    } else {
                        for (uint32_t j = 0; j < n->getPrefixLength() - i - 1; ++j) {
                            nonMatchingPrefix[j] = n->getPrefix()[i + j + 1];
                        }
                    }
                    return CheckPrefixPessimisticResult::NoMatch;
                }
                ++level;
            }
        }
        return CheckPrefixPessimisticResult::Match;
    }

    typename Tree::PCCompareResults Tree::checkPrefixCompare(N *n, const Key &k, uint32_t &level,
                                                        LoadKeyFunction loadKey) {
        if (n->hasPrefix()) {
            Key kt;
            for (uint32_t i = 0; i < n->getPrefixLength(); ++i) {
                if (i == maxStoredPrefixLength) {
                    loadKey(N::getAnyChildTid(n), kt);
                }
                uint8_t kLevel = (k.getKeyLen() > level) ? k[level] : 0;

                uint8_t curKey = i >= maxStoredPrefixLength ? kt[level] : n->getPrefix()[i];
                if (curKey < kLevel) {
                    return PCCompareResults::Smaller;
                } else if (curKey > kLevel) {
                    return PCCompareResults::Bigger;
                }
                ++level;
            }
        }
        return PCCompareResults::Equal;
    }

    typename Tree::PCEqualsResults Tree::checkPrefixEquals(N *n, uint32_t &level, const Key &start, const Key &end,
                                                      LoadKeyFunction loadKey) {
        if (n->hasPrefix()) {
            bool endMatches = true;
            Key kt;
            for (uint32_t i = 0; i < n->getPrefixLength(); ++i) {
                if (i == maxStoredPrefixLength) {
                    loadKey(N::getAnyChildTid(n), kt);
                }
                uint8_t startLevel = (start.getKeyLen() > level) ? start[level] : 0;
                uint8_t endLevel = (end.getKeyLen() > level) ? end[level] : 0;

                uint8_t curKey = i >= maxStoredPrefixLength ? kt[level] : n->getPrefix()[i];
                if (curKey > startLevel && curKey < endLevel) {
                    return PCEqualsResults::Contained;
                } else if (curKey < startLevel || curKey > endLevel) {
                    return PCEqualsResults::NoMatch;
                } else if (curKey != endLevel) {
                    endMatches = false;
                }
                ++level;
            }
            if (!endMatches) {
                return PCEqualsResults::StartMatch;
            }
        }
        return PCEqualsResults::BothMatch;
    }

    long Tree::size() {
        auto size = N::size(root);
        return size + sizeof(root);
    }

    void Tree::dfs(N*nd,int depth){
        if(nd==nullptr)return;
        if(N::isLeaf(nd)){
            num_leaf++;
            len_path+=depth;
            return;
        }
        switch(nd->getType()){
            case NTypes::N4:{
                auto n = static_cast<N4 *>(nd);
                for(int i=0;i<256;i++)
                    dfs(n->getChild(i),depth+1);}
                        
                break;
            case NTypes::N16:{
                auto n = static_cast<N16 *>(nd);
                for(int i=0;i<256;i++)
                    dfs(n->getChild(i),depth+1);}
                break;
            case NTypes::N48:{
                auto n = static_cast<N48 *>(nd);
                for(int i=0;i<256;i++)
                    dfs(n->getChild(i),depth+1);}
                break;
            case NTypes::N256:{
                auto n = static_cast<N256 *>(nd);
                for(int i=0;i<256;i++)
                    dfs(n->getChild(i),depth+1);}
                break;
        }
    }
    void Tree::cenggao(){
        num_leaf=0;
        len_path=0;
        dfs(root,0);
        if(num_leaf==0){
            printf("cenggao cannot be counted\n");
        }
        else{
            printf("cenggao:%lld/%lld=%lf\n",len_path,num_leaf,len_path/(double)num_leaf);
        }
    }

    inline static uint32_t lycswap_endian(uint32_t i) {
        return __builtin_bswap32(i);
    }
    inline static uint64_t lycswap_endian(uint64_t i) {
        return __builtin_bswap64(i);
    }

    template <typename Iterator>
    Key Tree::getkey(Iterator &icur){

        Key k;
        auto reversed=icur->first;
        reversed = lycswap_endian(icur->first);
        k.set(reinterpret_cast<char *>(&reversed), sizeof(icur->first));
        return k;
    }
    template <typename T1,typename T2>
    bool lyccmp(T1 &a,T2 &b,int c){
        for(int i=0;i<c;i++)
            if(a[i]!=b[i])return true;
        return false;
    }
    using lyc_Prefix = uint8_t[66];
    template <typename Iterator>
        N* Tree::lyc_dfs(lyc_Prefix pre,Iterator &icur, Iterator &iend,int depth){
            std::pair<int,N*>sons[256];
            int cnt=0;



            lyc_Prefix nw;
            memcpy(nw,pre,depth);
            while(1){
                auto keycur=getkey(icur);//memcmp(pre,keycur.data,depth)
                if(icur==iend||lyccmp(pre,keycur,depth)){//可能效率不高。
                    if(cnt==0){
                        assert(false);
                        printf("ERR\n");
                    }
                    if(depth==0){
                        N256*ret=static_cast<N256 *>(root);
                        for(int i=0;i<cnt;i++)
                            ret->insert(sons[i].first,sons[i].second);
                        return ret;
                    }
                    if(cnt==1){
                        //if(depth-1==)
                        if(!N::isLeaf(sons[0].second)){
                            memcpy(nw+1,sons[0].second->getPrefix(),std::min(sons[0].second->getPrefixLength(),maxStoredPrefixLength-1));
                            auto bef=icur-1;
                            auto keybef=getkey(bef);
                            nw[0]=keybef[depth];
                            sons[0].second->setPrefix(nw,sons[0].second->getPrefixLength()+1);
                        }
                        /*if(depth==0){
                            N4*ret;
                            ret=static_cast<N4 *>(root);
                            ret->insert(sons[0].first,sons[0].second);
                        }*/
                        return sons[0].second;//maxStoredPrefixLength
                    }
                    
                    if(cnt<=4){
                        N4*ret=new N4(nullptr,0);
                        for(int i=0;i<cnt;i++)
                            ret->insert(sons[i].first,sons[i].second);
                        return ret;
                    }
                    if(cnt<=16){
                        N16*ret=new N16(nullptr,0);
                        for(int i=0;i<cnt;i++)
                            ret->insert(sons[i].first,sons[i].second);
                        return ret;
                    }
                    if(cnt<=48){
                        N48*ret=new N48(nullptr,0);
                        for(int i=0;i<cnt;i++)
                            ret->insert(sons[i].first,sons[i].second);
                        return ret;
                    }
                    if(cnt<=256){
                        N256*ret=new N256(nullptr,0);
                        for(int i=0;i<cnt;i++)
                            ret->insert(sons[i].first,sons[i].second);
                        return ret;
                    }
                }
                nw[depth]=keycur[depth];
                auto nxt=icur+1;
                auto keynxt=getkey(nxt);//memcmp(keynxt.data,keycur.data,depth+1)
                if(nxt==iend||lyccmp(keynxt,keycur,depth+1)){
                    sons[cnt++]=std::make_pair(nw[depth],N::setLeaf(reinterpret_cast<TID>(icur)));
                    icur++;
                }
                else{
                    /*if(depth==7){
                        printf("keylen=%d\n",keycur.getKeyLen());
                        for(int i=0;i<8;i++){
                            printf("%d=%d\n",keycur[i],keynxt[i]);
                        }

                    }*/
                    sons[cnt++]=std::make_pair(nw[depth],lyc_dfs(nw,icur,iend,depth+1));
                }
            }

        }
    template <typename Iterator>
        void Tree::bulk_load(Iterator ibegin, Iterator iend){
            if(ibegin==iend)return;
            sort(ibegin,iend);
            lyc_Prefix zro;

            lyc_dfs(zro,ibegin,iend,0);
        }
}

/*
N *node = nullptr;
        N *nextNode = root;
        N *parentNode = nullptr;
        uint8_t parentKey, nodeKey = 0;
        uint32_t level = 0;

        while (true) {
            parentNode = node;
            parentKey = nodeKey;
            node = nextNode;

            uint32_t nextLevel = level;

            uint8_t nonMatchingKey;
            Prefix remainingPrefix;
            switch (checkPrefixPessimistic(node, k, nextLevel, nonMatchingKey, remainingPrefix,
                                                           this->loadKey)) { // increases level
                case CheckPrefixPessimisticResult::NoMatch: {
                    assert(nextLevel < k.getKeyLen()); //prevent duplicate key
                    // 1) Create new node which will be parent of node, Set common prefix, level to this node
                    auto newNode = new N4(node->getPrefix(), nextLevel - level);

                    // 2)  add node and (tid, *k) as children
                    newNode->insert(k[nextLevel], N::setLeaf(tid));
                    newNode->insert(nonMatchingKey, node);

                    // 3) update parentNode to point to the new node
                    N::change(parentNode, parentKey, newNode);

                    // 4) update prefix of node
                    node->setPrefix(remainingPrefix,
                                    node->getPrefixLength() - ((nextLevel - level) + 1));

                    return;

*/