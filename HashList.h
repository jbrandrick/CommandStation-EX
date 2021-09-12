
#ifndef HashList_h
#define HashList_h


#include "Turnout.h"
#include "Sensor.h"
#include "Output.h"
template <class T>
class HashList {

  class Node;

  public:

    HashList<T> () noexcept {
      pFreeRoot = nullptr;
      pRoot     = nullptr;
      count     = 0;
      seq       = 0;
      seqBase   = 0;
    };

    T*    get (int);
    T*    add (int);
    T*    getOrAdd (int);
    bool  remove (int);
    int   getFirstKey ();
    int   getNextKey (int);

    template <typename FTN>
    void walkList (FTN ftn) {
      for (Node* pNode = getRootNode (); pNode != nullptr; pNode = pNode->pNext) {
        ftn (pNode->key, pNode->data);
      }
    }
    int   size () {
      return count;
    }
    bool  hasChanged () {
      return (seq > seqBase);
    }
    void  resetChanged () {
      seqBase = seq;
    }


  private:

    Node* pFreeRoot;
    Node* pRoot;
    int   count;
    int   seq;
    int   seqBase;


    class Node {
      int   key;
      T*    data;
      Node* pNext;

      friend class HashList;
    };

    void bumpChanged () {
      if (seqBase > 64000)
        seqBase = seq = 0;
      seq++;
    }
    Node* getNode (int key) {
      Node* pNode = getRootNode ();

      while (pNode) {
        if (pNode->key == key)
          return pNode;
        pNode = pNode->pNext;
        }
      return nullptr;
    }
    Node* getRootNode () {
      return pRoot;
    }
    void setRootNode (Node* pNode) {
      pRoot = pNode;
    }
    Node* makeNode (int key) {
      Node* pNode = getRootNode ();
      Node* pNewNode;

      if (pFreeRoot == nullptr) {
        pNewNode        = new Node;
        pNewNode->data  = new T;

      } else {
        pNewNode        = pFreeRoot;
        pFreeRoot       = pNewNode->pNext;
      }

      pNewNode->key     = key;
      pNewNode->pNext   = nullptr;

      if (pNode) {
        while (pNode->pNext != nullptr) {
          pNode         = pNode->pNext;
        }
        pNode->pNext    = pNewNode;

      } else {
        setRootNode (pNewNode);
      }
      count++;
      bumpChanged ();

      return pNewNode;
    }
};

template class HashList<Turnout>;
template class HashList<Sensor>;
template class HashList<Output>;

#endif
