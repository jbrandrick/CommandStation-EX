#include "HashList.h"

template <class T>
T* HashList<T>::get (int key) {
  Node* pNode = getNode (key);
  if (pNode == nullptr)
    return nullptr;
  else
    return pNode->data;
}

template <class T>
T* HashList<T>::add (int key) {
  Node* pT = makeNode (key);
  return pT->data;
}

template <class T>
T* HashList<T>::getOrAdd (int key) {
  T* pT = get (key);

  if (pT == nullptr)
    pT = add (key);
  return pT;
}

template <class T>
bool HashList<T>::remove (int key) {
  Node* pNode       = getRootNode ();
  Node* pPriorNode  = nullptr;

  while (pNode != nullptr  &&  pNode->key != key) {
    pPriorNode  = pNode;
    pNode       = pNode->pNext;
  }
  if (pNode == nullptr) return false;

  if (pPriorNode == nullptr) {
    setRootNode (pNode->pNext);
  } else {
    pPriorNode->pNext = pNode->pNext;
  }

  pNode->pNext  = pFreeRoot;
  pFreeRoot     = pNode;

  count--;
  bumpChanged ();
  return true;
}


template <class T>
int HashList<T>::getFirstKey () {
  if (getRootNode () == nullptr)
    return -1;
  return pRoot->key;
}

template <class T>
int HashList<T>::getNextKey (int key) {
  Node* pNode = getNode (key);
  if (pNode == nullptr)
    return getFirstKey ();

  if (pNode->pNext == nullptr)
    return getFirstKey ();
  else
    return pNode->pNext->key;
}
