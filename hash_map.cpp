#include <functional>
#include <memory>
#include <iostream>
#include <utility>
#include <map>
#include <iterator>
#include <cmath>
#include <cassert>


template< typename T, typename Alloc = std::allocator<T> >
class List {

    size_t sz = 0;
public:
    struct Node {
        T data = {};

        Node* next = nullptr;
        Node* prev = nullptr;

        uint32_t cached;

        Node() = default;

        Node(const T& x): data(x) {}

        // Node(T&& x) {
        //     data.first = std::move(x.first);
        //     data.second = std::move(x.second);
        // }

        void unlink() {

            if(next != nullptr and prev != nullptr) {
                next->prev = this->prev;
                prev->next = this->next;
            }

            if(next == nullptr and prev != nullptr) {
                prev->next = nullptr;
            }

            if(next != nullptr and prev == nullptr) {
                next->prev = nullptr;
            }

            this->next = nullptr;//Is it really needed?
            this->prev = nullptr;
        }

    };
private:

    struct base_iterator{};

    template<bool IsConst>
    struct common_iterator: public base_iterator{
        private:            
            Node* ptr = nullptr;
        public:
            friend class List;

            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::conditional_t<IsConst, const T, T>;
            using pointer = value_type*;
            using reference = value_type&;

            common_iterator() = default;

            common_iterator(Node* ptr): ptr(ptr) {}

            common_iterator(const common_iterator& another): ptr(another.ptr) {}

            ~common_iterator() = default;

            common_iterator& operator=(const common_iterator& another) {
                ptr = another.ptr;
                return *this;
            }

            value_type& operator*() {
                return ptr->data;
            }

            value_type* operator->() {
                return &ptr->data;
            }

            common_iterator& operator++() {//add postfix++
                ptr = ptr->next;
                return *this;
            }

            common_iterator operator++(int) {
                auto that = *this;
                ++(*this);
                return that;
            }

            common_iterator& operator--() {
                ptr = ptr->prev;
                return *this;
            }

            bool operator==(common_iterator another) {
                return ptr == another.ptr;
            }

            bool operator!=(common_iterator another) {
                return ptr != another.ptr;
            }
            
    };

    Alloc list_alloc;

    using AllocType = typename std::allocator_traits<Alloc>:: template rebind_alloc<Node>;
    AllocType nodealloc;
public:
using base_iterator = base_iterator;
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
    Node* end_ptr;
    Node* begin_ptr;

    Node* rbegin_ptr;
    Node* rend_ptr;

public:
    iterator begin() {
        return iterator(begin_ptr);
    }

    iterator end() {
        return iterator(end_ptr);
    }

    const_iterator cbegin() {
        return const_iterator(begin_ptr);
    }

    const_iterator cend() {
        return const_iterator(end_ptr);
    }

    iterator rbegin() {
        return reverse_iterator(rbegin_ptr);
    }

    iterator rend() {
        return reverse_iterator(rend);
    }

    List() {
        end_ptr = std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        std::allocator_traits<AllocType>::construct(nodealloc, end_ptr);

        begin_ptr = end_ptr;

    }

    List(Alloc& alloc): list_alloc(alloc/*std::allocator_traits<Alloc>::select_on_container_copy_construction(alloc)*/) {
        end_ptr = std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        std::allocator_traits<AllocType>::construct(nodealloc, end_ptr);

        begin_ptr = end_ptr;

    }

    List(Alloc& alloc, iterator end): list_alloc(alloc), end_ptr(end.ptr) {

        begin_ptr = end_ptr;
    }


    //Define copy/move constructors in the future for unordered_map copy/move operations.

    template<typename U>
    void push_front(U&& value) {
        
        Node* newnode = typename std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        std::allocator_traits<AllocType>::construct(nodealloc, newnode, std::move(value));

        insert(begin(), newnode);
    }

    //template<typename U>
    void insert(iterator it, Node* newnode) {//does not invalidate references
        
        if(it == begin()) {
            
            it.ptr->prev = newnode;
            newnode->next = it.ptr;
            begin_ptr = newnode;           
            
        }

        if(begin_ptr != end_ptr) {
            newnode->prev = it.ptr->prev;
            newnode->prev->next = newnode;

            newnode->next = it.ptr;
            newnode->next->prev = newnode;
        }
        ++sz;
    }

    void push_back(Node* newnode) {
        insert(end(), newnode);
    }

    template<typename IteratorType>//wrong: iterator is pointing to std::pair, not to Node
    void push_back(IteratorType it) {
        if constexpr(std::is_same_v<IteratorType, iterator> || std::is_same_v<IteratorType, const_iterator>) {
            insert(end(), it.ptr);
        } else {
            Node* newnode = std::allocator_traits<AllocType>::allocate(nodealloc, 1);
            std::allocator_traits<AllocType>::construct(nodealloc, newnode, *it);

            insert(end(), newnode);
        }
    }

    // template<typename IteratorType>
    // void push_back(IteratorType it) {
    //     Node* newnode = typename std::allocator_traits<AllocType>::allocate(nodealloc, 1);
    //     std::allocator_traits<AllocType>::construct(nodealloc, newnode, *it);

    //     insert(end(), newnode);
    // }





    void erase(iterator it) {

        if(it.ptr == begin_ptr) {
            begin_ptr = it.ptr->next;
        }

        it.ptr->unlink();

        std::allocator_traits<AllocType>::destroy(nodealloc, it.ptr);
        std::allocator_traits<AllocType>::deallocate(nodealloc, it.ptr, 1);
        --sz;
    }

    size_t size() {
        return sz;
    }


};

template<
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator< std::pair<const Key, T> > >
class hash_map {
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;  

    double load_factor = 0;     /* size/number of buckets */
    double max_load_factor_ = 1;

    size_type element_count = 0;
    size_type bucket_count = 0;

    using storage_type = std::vector<List<value_type, Allocator>*>;
    storage_type storage;
    size_t storage_size = 10;
    Hash hash;

public:
    using iterator = typename List<value_type, Allocator>::iterator;
    using const_iterator = typename List<value_type, Allocator>::const_iterator;
    using base_iterator = typename List<value_type, Allocator>::base_iterator;
private:
    iterator global_begin;
    iterator global_end;

    const_iterator global_cbegin;
    const_iterator global_cend;


    allocator_type alloc;
    using ListAllocType = typename std::allocator_traits<allocator_type>:: template rebind_alloc<List<value_type, Allocator>>;
    using NodeType = typename List<value_type,Allocator>::Node;


    using NodeAllocType = typename std::allocator_traits<allocator_type>:: template rebind_alloc<NodeType>;
    NodeAllocType node_alloc;

public:
    hash_map(): storage(10) {
    }

    auto begin() {
        return global_begin;
    }

    auto end() {
        return global_end;
    }

    auto cbegin() {
        return global_cbegin;
    }

    auto cend() {
        return global_cend;
    }

    mapped_type& operator[](const key_type& key) {

        auto [it, second] = insert({key, mapped_type()});
        
        return it->second;
    }

    //TODO: change iterator type
    //template<typename IteratorType> is not needed...?
    std::pair<base_iterator, bool> insert(value_type&& value) { 
        NodeType* newnode = std::allocator_traits<NodeAllocType>::allocate(node_alloc, 1);
        std::allocator_traits<NodeAllocType>::construct(node_alloc, newnode, std::forward<value_type>(value));

        return insert_node(iterator(newnode));
    }

    template<typename IteratorType>
    void insert(IteratorType first, IteratorType last) {
    
        auto cur = first;//old global begin
        auto next = std::next(cur);
        
        auto end_it = last;

        while(cur != end_it) {
            insert_node(cur);
            cur = next;
            if(next != end_it) {
                ++next;
            }
        }

    }

    template<typename... Args>
    std::pair<base_iterator, bool> emplace(Args&&... args) {
        NodeType* newnode = std::allocator_traits<NodeAllocType>::allocate(node_alloc, 1);
        std::allocator_traits<NodeAllocType>::construct(node_alloc, newnode, std::make_pair(std::forward<Args>(args)...));

        return insert_node(iterator(newnode));
    }

    template<typename IteratorType>
    std::pair<base_iterator, bool> insert_node(IteratorType node) {
        int bucket_index = hash(node->first) % storage_size;

        if(storage[bucket_index] == nullptr) {//if there are no such bucket yet
            
            using ListAllocType = typename std::allocator_traits<allocator_type>:: template rebind_alloc<List<value_type>>;
            ListAllocType listalloc;
            storage[bucket_index] = std::allocator_traits<ListAllocType>::allocate(listalloc, 1);//created List(bucket)
            
            if(bucket_count == 0) {//if bucket_count = 0
                std::allocator_traits<ListAllocType>::construct(listalloc, storage[bucket_index], alloc);
                (storage[bucket_index])->push_back(node);//error
                global_end = (storage[bucket_index])->end();
                global_cend = (storage[bucket_index])->cend();

            } else {
                std::allocator_traits<ListAllocType>::construct(listalloc, storage[bucket_index], alloc, global_begin);
                (storage[bucket_index])->push_back(node);
            }
            global_begin = (storage[bucket_index])->begin();
            global_cbegin = (storage[bucket_index])->cbegin();

            ++bucket_count;
            ++element_count;
            return std::make_pair((storage[bucket_index])->begin(), true);
        } else {//if there is such bucket already
            for(iterator it = storage[bucket_index]->begin(); it != storage[bucket_index]->end(); ++it) {
                if(it->first == node->first) {
                    return std::make_pair(it,false);
                }
            }
            storage[bucket_index]->push_back(node);
            ++element_count;
            return std::make_pair(--(storage[bucket_index]->end()), true);//KEKW
        }
    }

    //TODO: change iterator
    iterator find(const Key& key) {
        size_t bucket_index = hash(key) % storage_size;

        for(auto it = storage[bucket_index]->begin(); it != storage[bucket_index]->end(); ++it) {
            //std::cout << it->first;
            if(it->first == key) {
                return it;
            }
        }
        return this->end();
    }

    const_iterator find(const Key& key) const{
        size_t bucket_index = hash(key) % storage_size;

        for(auto it = storage[bucket_index]->cbegin(); it != storage[bucket_index]->cend(); ++it) {
            //std::cout << it->first;
            if(it->first == key) {
                return it;
            }
        }
        return this->cend();
    }



    T& at(const Key& key) {
        auto it = find(key);

        if(it == global_end) {
            throw(std::out_of_range{"AAAAA"});
        }

        return it->second;
    }   

    //template<typename IteratorType>
    void erase(const key_type& key) {//TODO:erase by iterator and range of iterators
        int bucket_index = hash(key) % storage_size;

        auto it = find(key);

        if(it != this->end()) {
            if(key == global_begin->first) {
                ++global_begin;
            }
            storage[bucket_index]->erase(it);
            --element_count;
        }
    }
    template<typename IteratorType>
    void erase(IteratorType it) {//TODO:erase by iterator and range of iterators
        int bucket_index = hash(it->first) % storage_size;

        auto iter = find(it->first);

        if(iter != this->end()) {
            if(it->first == global_begin->first) {
                ++global_begin;
            }
            storage[bucket_index]->erase(it);
            --element_count;
        }
    }

    template<typename IteratorType>
    void erase(IteratorType first, IteratorType last) {
        auto cur = first;
        auto next = std::next(first);

        auto end_it = last;

        while(cur != end_it) {
            size_t bucket_index = hash(cur->first) % storage_size;
            if(cur->first == global_begin->first) {//if constexpr std::is_same_v(IteratorType, iterator/const_iterator)
                ++global_begin;
            }
            storage[bucket_index]->erase(cur);
            cur = next;
            --element_count;
            if(next != end_it) {
                ++next;
            }
        }
    }

public:

    void rehash(size_t count) {

        if(count <= storage_size) {
           return;
        }
        std::cout << "|rehash called|" << std::endl;

        auto cur = this->begin();//old global begin
        auto next = std::next(cur);
        
        auto end_it = this->end();

        storage.clear();
        bucket_count = 0;
        element_count = 0;
        storage.reserve(count);
        storage_size = count;

        while(cur != end_it) {
            insert_node(cur);
            cur = next;
            if(next != end_it) {
                ++next;
            }
        }

    }

    void reserve(size_t count) {
        rehash(std::ceil(count / max_load_factor()));
    }



    size_t buckets() {
        return bucket_count;
    }

    size_t size() {
        return element_count;
        //return std::distance(begin(), end());
    }

    double max_load_factor() {
        return max_load_factor_;
    }

};
