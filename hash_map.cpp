#include <functional>
#include <memory>
#include <iostream>
#include <utility>
#include <map>



template< typename T, typename Alloc = std::allocator<T> >
class List {
    size_t sz = 0;
public:
    struct Node {
        Node* next = nullptr;
        Node* prev = nullptr;

        T data;

        uint32_t cached;

        Node() = default;

        Node(const T& x): data(x) {}

        Node(T&& x): data(std::move(x)) {}

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
    template<bool IsConst>
    struct common_iterator {
        private:
            Node* ptr = nullptr;
        public:
            friend class List;
            using iterator_type = std::bidirectional_iterator_tag;
            using Pointer = Node*;

            common_iterator() = default;

            common_iterator(Node* ptr): ptr(ptr) {}

            common_iterator(const common_iterator& another): ptr(another.ptr) {}

            ~common_iterator() = default;

            common_iterator& operator=(const common_iterator& another) {
                ptr = another.ptr;
                return *this;
            }

            T& operator*() {
                return ptr->data;
            }

            T* operator->() {
                return ptr;
            }

            common_iterator& operator++() {
                ptr = ptr->next;
                return *this;
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
    //Define copy/move constructors in the future for unordered_map copy/move operations.

    template<typename U>
    void push_front(U&& value) {
        
        Node* newnode = typename std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        std::allocator_traits<AllocType>::construct(nodealloc, newnode, std::move(value));

        insert(begin(), newnode);
    }

    template<typename U>
    void insert(iterator it, U&& value) {
        
        Node* newnode = std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        std::allocator_traits<AllocType>::construct(nodealloc, newnode, std::move(value));

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

    template<typename U>
    void push_back(U&& value) {
        Node* newnode = std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        std::allocator_traits<AllocType>::construct(nodealloc, newnode, std::move(value));

        insert(end(), *newnode);
    }

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

//being() == NodeList::iterator(storage[hash(key)%size]]);
//std::vector<NodeList*> storage;


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
    double max_load_factor = 0.8;

    size_type element_count = 0;
    size_type bucket_count = 0;

    std::vector<List<value_type, Allocator>*> storage;
    Hash hash;

    // typename List<mapped_type, Allocator>::Node* global_begin;
    // typename List<mapped_type, Allocator>::Node* global_end;
    typename List<value_type, Allocator>::iterator global_begin;
    typename List<value_type, Allocator>::iterator global_end;


    allocator_type alloc;
    // using ListAllocType = std::allocator_traits<allocator_type>::



public:
    hash_map(): storage(10) {
        
    }

    auto begin() {
        return global_begin;
    }


    mapped_type operator[](key_type key) {
        //is half wrong
        //should return a reference
        size_t bucket_index = hash(key) % storage.size();

        for(auto it: *(storage[bucket_index])) {
            if(it.first == key) {
                return it.second;
            }
        }
        return 0;
    }

    void insert(const key_type key, T value) {
        int hashed = hash(key) % storage.size();

        if(storage[hashed] == nullptr) {
            ++bucket_count;
            
            using ListAllocType = typename std::allocator_traits<allocator_type>:: template rebind_alloc<List<value_type>>;
            ListAllocType listalloc;
            storage[hashed] = std::allocator_traits<ListAllocType>::allocate(listalloc, 1);
            std::allocator_traits<ListAllocType>::construct(listalloc, storage[hashed], alloc);

            (storage[hashed])->push_back(std::make_pair(key, value));

            //global_begin = *(storage[hashed]).begin_ptr;
            global_begin = (storage[hashed])->begin();

        } else {
            storage[hashed]->push_back(std::make_pair(key, value));
        }
    }
};





int main() {

    hash_map<int, int> hm;

    hm.insert(5, 9);
    hm.insert(15, 3);
    auto [key, value] = *hm.begin();
    std::cout << value;








}

