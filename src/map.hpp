/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<class Key, class T, class Compare = std::less<Key>>
class map {
 public:
  typedef pair<const Key, T> value_type;

 private:
  struct Node {
    value_type val;
    Node *left;
    Node *right;
    Node *parent;
    int height;
    Node(const value_type &v, Node *p) : val(v), left(nullptr), right(nullptr), parent(p), height(1) {}
  };

  Node *root = nullptr;
  size_t n = 0;
  Compare comp;

  static int h(Node *x) { return x ? x->height : 0; }
  static void upd(Node *x) { if (x) x->height = (h(x->left) > h(x->right) ? h(x->left) : h(x->right)) + 1; }

  Node *rotateLeft(Node *x) {
    Node *y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->left = x;
    y->parent = x->parent;
    x->parent = y;
    upd(x); upd(y);
    return y;
  }
  Node *rotateRight(Node *x) {
    Node *y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->right = x;
    y->parent = x->parent;
    x->parent = y;
    upd(x); upd(y);
    return y;
  }
  int bf(Node *x) const { return h(x->left) - h(x->right); }

  Node *rebalance(Node *x) {
    upd(x);
    int b = bf(x);
    if (b > 1) {
      if (bf(x->left) < 0) { x->left = rotateLeft(x->left); x->left->parent = x; }
      Node *r = rotateRight(x);
      return r;
    } else if (b < -1) {
      if (bf(x->right) > 0) { x->right = rotateRight(x->right); x->right->parent = x; }
      Node *r = rotateLeft(x);
      return r;
    }
    return x;
  }

  Node *minNode(Node *x) const { while (x && x->left) x = x->left; return x; }
  Node *maxNode(Node *x) const { while (x && x->right) x = x->right; return x; }

  Node *findNode(const Key &key) const {
    Node *cur = root;
    while (cur) {
      if (comp(key, cur->val.first)) cur = cur->left;
      else if (comp(cur->val.first, key)) cur = cur->right;
      else return cur;
    }
    return nullptr;
  }

  void attachParent(Node *child, Node *parent, bool asLeft) {
    if (child) child->parent = parent;
    if (parent) {
      if (asLeft) parent->left = child; else parent->right = child;
    } else {
      root = child;
      if (root) root->parent = nullptr;
    }
  }

  void rebuildUp(Node *from) {
    Node *cur = from;
    while (cur) {
      Node *p = cur->parent;
      Node *newCur = rebalance(cur);
      if (newCur != cur) {
        if (p) {
          newCur->parent = p;
          if (p->left == cur) p->left = newCur; else p->right = newCur;
        } else {
          root = newCur;
          root->parent = nullptr;
        }
      }
      cur = newCur->parent;
    }
  }

  Node *clone(Node *p, Node *other) {
    if (!other) return nullptr;
    Node *x = new Node(other->val, p);
    x->height = other->height;
    x->left = clone(x, other->left);
    x->right = clone(x, other->right);
    return x;
  }

  void clearNode(Node *x) {
    if (!x) return;
    clearNode(x->left);
    clearNode(x->right);
    delete x;
  }

 public:
  class const_iterator;
  class iterator {
    friend class map;
    friend class const_iterator;
    map *owner;
    Node *cur;
    iterator(map *o, Node *c) : owner(o), cur(c) {}
   public:
    iterator() : owner(nullptr), cur(nullptr) {}
    iterator(const iterator &other) : owner(other.owner), cur(other.cur) {}

    iterator operator++(int) { iterator tmp(*this); ++(*this); return tmp; }
    iterator &operator++() {
      if (!owner) throw invalid_iterator();
      if (cur == nullptr) throw invalid_iterator();
      if (cur->right) {
        cur = owner->minNode(cur->right);
      } else {
        Node *p = cur->parent;
        while (p && cur == p->right) cur = p, p = p->parent;
        cur = p;
      }
      return *this;
    }
    iterator operator--(int) { iterator tmp(*this); --(*this); return tmp; }
    iterator &operator--() {
      if (!owner) throw invalid_iterator();
      if (cur == nullptr) {
        if (owner->n == 0) throw invalid_iterator();
        cur = owner->maxNode(owner->root);
        return *this;
      }
      if (cur->left) {
        cur = owner->maxNode(cur->left);
      } else {
        Node *p = cur->parent;
        while (p && cur == p->left) cur = p, p = p->parent;
        if (!p) throw invalid_iterator();
        cur = p;
      }
      return *this;
    }
    value_type &operator*() const {
      if (!owner || cur == nullptr) throw invalid_iterator();
      return cur->val;
    }
    value_type *operator->() const noexcept { return &cur->val; }

    bool operator==(const iterator &rhs) const { return owner == rhs.owner && cur == rhs.cur; }
    bool operator==(const const_iterator &rhs) const;
    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
    bool operator!=(const const_iterator &rhs) const;
  };

  class const_iterator {
    friend class map;
    const map *owner;
    Node *cur;
    const_iterator(const map *o, Node *c) : owner(o), cur(c) {}
   public:
    const_iterator() : owner(nullptr), cur(nullptr) {}
    const_iterator(const const_iterator &other) : owner(other.owner), cur(other.cur) {}
    const_iterator(const iterator &other) : owner(other.owner), cur(other.cur) {}

    const_iterator operator++(int) { const_iterator tmp(*this); ++(*this); return tmp; }
    const_iterator &operator++() {
      if (!owner) throw invalid_iterator();
      if (cur == nullptr) throw invalid_iterator();
      if (cur->right) {
        cur = owner->minNode(cur->right);
      } else {
        Node *p = cur->parent;
        Node *c = cur;
        while (p && c == p->right) c = p, p = p->parent;
        cur = p;
      }
      return *this;
    }
    const_iterator operator--(int) { const_iterator tmp(*this); --(*this); return tmp; }
    const_iterator &operator--() {
      if (!owner) throw invalid_iterator();
      if (cur == nullptr) {
        if (owner->n == 0) throw invalid_iterator();
        cur = owner->maxNode(owner->root);
        return *this;
      }
      if (cur->left) {
        cur = owner->maxNode(cur->left);
      } else {
        Node *p = cur->parent;
        Node *c = cur;
        while (p && c == p->left) c = p, p = p->parent;
        if (!p) throw invalid_iterator();
        cur = p;
      }
      return *this;
    }
    const value_type &operator*() const {
      if (!owner || cur == nullptr) throw invalid_iterator();
      return cur->val;
    }
    const value_type *operator->() const noexcept { return &cur->val; }

    bool operator==(const const_iterator &rhs) const { return owner == rhs.owner && cur == rhs.cur; }
    bool operator==(const iterator &rhs) const { return owner == rhs.owner && cur == rhs.cur; }
    bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
    bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
  };

  bool key_eq(const Key &a, const Key &b) const { return !comp(a, b) && !comp(b, a); }

  map() : root(nullptr), n(0), comp(Compare()) {}

  map(const map &other) : n(other.n), comp(other.comp) {
    root = clone(nullptr, other.root);
  }

  map &operator=(const map &other) {
    if (this == &other) return *this;
    clear();
    comp = other.comp;
    n = other.n;
    root = clone(nullptr, other.root);
    return *this;
  }

  ~map() { clear(); }

  T &at(const Key &key) {
    Node *x = findNode(key);
    if (!x) throw index_out_of_bound();
    return x->val.second;
  }
  const T &at(const Key &key) const {
    Node *x = findNode(key);
    if (!x) throw index_out_of_bound();
    return x->val.second;
  }

  T &operator[](const Key &key) {
    Node *cur = root, *parent = nullptr;
    bool isLeft = false;
    while (cur) {
      parent = cur;
      if (comp(key, cur->val.first)) cur = cur->left, isLeft = true;
      else if (comp(cur->val.first, key)) cur = cur->right, isLeft = false;
      else return cur->val.second;
    }
    value_type v(key, T());
    Node *node = new Node(v, parent);
    if (!parent) root = node;
    else attachParent(node, parent, isLeft);
    ++n;
    rebuildUp(parent ? parent : node);
    return node->val.second;
  }

  const T &operator[](const Key &key) const {
    Node *x = findNode(key);
    if (!x) throw index_out_of_bound();
    return x->val.second;
  }

  iterator begin() { return iterator(this, minNode(root)); }
  const_iterator cbegin() const { return const_iterator(this, minNode(root)); }

  iterator end() { return iterator(this, nullptr); }
  const_iterator cend() const { return const_iterator(this, nullptr); }

  bool empty() const { return n == 0; }
  size_t size() const { return n; }

  void clear() {
    clearNode(root);
    root = nullptr;
    n = 0;
  }

  pair<iterator, bool> insert(const value_type &value) {
    Node *cur = root, *parent = nullptr;
    bool isLeft = false;
    while (cur) {
      parent = cur;
      if (comp(value.first, cur->val.first)) cur = cur->left, isLeft = true;
      else if (comp(cur->val.first, value.first)) cur = cur->right, isLeft = false;
      else return pair<iterator, bool>(iterator(this, cur), false);
    }
    Node *node = new Node(value, parent);
    if (!parent) root = node;
    else attachParent(node, parent, isLeft);
    ++n;
    rebuildUp(parent ? parent : node);
    return pair<iterator, bool>(iterator(this, node), true);
  }

  void erase(iterator pos) {
    if (pos.owner != this || pos.cur == nullptr) throw invalid_iterator();
    Node *z = pos.cur;
    Node *rebFrom = nullptr;
    if (!z->left || !z->right) {
      Node *child = z->left ? z->left : z->right;
      Node *p = z->parent;
      bool asLeft = p && p->left == z;
      attachParent(child, p, asLeft);
      rebFrom = p ? p : child;
      delete z;
      --n;
      rebuildUp(rebFrom);
    } else {
      // successor with no left child
      Node *s = minNode(z->right);
      Node *sParent = s->parent;
      Node *sRight = s->right;

      // Detach s from its position
      if (sParent->left == s) {
        sParent->left = sRight;
      } else {
        sParent->right = sRight; // when z->right == s
      }
      if (sRight) sRight->parent = sParent;

      // Replace z with s
      Node *p = z->parent;
      bool zIsLeft = p && p->left == z;
      s->parent = p;
      if (p) {
        if (zIsLeft) p->left = s; else p->right = s;
      } else {
        root = s;
      }
      s->left = z->left; if (s->left) s->left->parent = s;
      s->right = (z->right == s ? sRight : z->right); if (s->right) s->right->parent = s;

      // Preserve height hints (will be fixed in rebuild)
      delete z;
      --n;

      // Rebuild from where structural changes occurred
      if (sParent == z) rebFrom = s; else rebFrom = sParent;
      rebuildUp(rebFrom);
    }
  }

  size_t count(const Key &key) const { return findNode(key) ? 1 : 0; }

  iterator find(const Key &key) { return iterator(this, findNode(key)); }
  const_iterator find(const Key &key) const { return const_iterator(this, findNode(key)); }
};

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::iterator::operator==(const const_iterator &rhs) const {
  return owner == rhs.owner && cur == rhs.cur;
}

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::iterator::operator!=(const const_iterator &rhs) const {
  return !(*this == rhs);
}

}

#endif
