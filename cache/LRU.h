//vec  ：可分配Node的容器
//如果新来的key 存在于hashmap中，则将对应Node更新置链表末端
//如果新来的key不存在于hashmap中，且vec为空，则丢弃链表初始端；若vec不为空，则在链表末端插入对应节点

#include<iostream>
#include<unordered_map>
#include<vector>
#include<string>
using namespace std;
template<typename K, typename V>
struct Node {
	K key;
	V data;
	Node *prev, *next;
};
template<typename K, typename V>
class LRUCache {
public:
	explicit LRUCache(int capacity) {
		entries_ = new Node<K, V>[capacity];
		for (int i = 0; i < capacity; ++i)
			vec.push_back(entries_+i);
		head_ = new Node<K, V>;
		tail_ = new Node<K, V>;
		head_->prev = NULL;
		head_->next = tail_;
		tail_->prev = head_;
		tail_->next = NULL;
	}
	~LRUCache() {
		delete head_;
		delete tail_;
		delete[] entries_;
	}
	V *Put(K *k, V *v) {
		Node<K, V>* node = hashmap_[*k];
		if (node) {
			detach(node);
			node->data = *v;
			attach(node);
		}
		else {
			if (vec.empty()) {
				node = head_->next;
				detach(node);
				hashmap_.erase(node->key);
			}
			else {
				node = vec.back();
				vec.pop_back();
			}
			node->key = *k;
			node->data = *v;
			hashmap_[*k] = node;
			attach(node);
		}
		return NULL;
	}

	V *Get(K *k) {
		Node<K, V> *node = hashmap_[*k];
		if (node) {
			detach(node);
			attach(node);
			return &node->data;
		}
		return NULL;
	}
	void detach(Node<K, V>* node) {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	void attach(Node<K, V>* node) {
		node->prev = tail_->prev;
		tail_->prev->next = node;
		node->next = tail_;
		tail_->prev = node;
	}
private:
	unordered_map<K, Node<K, V>* > hashmap_;
	vector<Node<K, V>* >vec;
	Node<K, V>*head_;
	Node<K, V>*tail_;
	Node<K, V>* entries_;
};