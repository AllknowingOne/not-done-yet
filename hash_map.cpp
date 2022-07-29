#include <functional>
#include <memory>
#include <iostream>
#include <utility>



template< typename T, typename Alloc = std::allocator<T> >
class List {

    struct Node {
        Node* next = nullptr;
        Node* prev = nullptr;

        T data;

        uint32_t cached;

        Node() = default;

        Node(const T& x): data(x) {
        
        }



        Node(T&& x): data(x) {}

        void unlink() {

            if(next != nullptr and prev != nullptr) {
                next->prev = prev;
                prev->next = next;
            }

            if(next == nullptr and prev != nullptr) {
                prev->next = nullptr;
            }

            if(next != nullptr and prev == nullptr) {
                next->prev = prev;
                prev->next = next;
            }

            next = nullptr;
            prev = nullptr;

        }
    };

    template<bool IsConst>
    struct common_iterator {
        private:
            Node* ptr;
        public:
            friend class List;
            using iterator_type = std::forward_iterator_tag;
            using Pointer = Node*;

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

    Alloc listalloc;

    using AllocType = typename std::allocator_traits<Alloc>:: template rebind_alloc<Node>;
    AllocType nodealloc;

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

    List(Alloc& alloc): listalloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(alloc)) {
        end_ptr = typename std::allocator_traits<AllocType>::allocate(nodealloc, 1);
        typename std::allocator_traits<AllocType>::construct(nodealloc, end_ptr);

        begin_ptr = end_ptr;

    }

    template<typename U>
    void push_front(U&& value) {
        Node* begin_ptr;

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
    }

    void erase() {

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

    allocator_type alloc;

    





};





int main() {

    List<int> mylist;

    mylist.insert(mylist.begin(), 3);
    mylist.insert(mylist.end(), 9);
    //mylist.insert(mylist.end(), 10);

    mylist.insert(mylist.begin(), 33);


    for(auto it: mylist) {
        std::cout << it << std::endl;
    }


    //std::cout << *(mylist.begin());


}

