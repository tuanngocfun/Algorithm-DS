#ifndef HASHTABLE_HPP_INCLUDED
#define HASHTABLE_HPP_INCLUDED

#include <iostream>
#include <string>
#include <cmath>

namespace Hashtable
{
template<typename _Tp, std::size_t N>
class Hashing
{
 public:
    using size_type = std::size_t;

    constexpr Hashing() { static_assert(N != 0, "Size can not be zero!"); }
    virtual void clear() = 0;

 protected:
    // generic hash function for all type
    template<typename T> static constexpr size_type Hash_Function(const T& value)
    {
        return static_cast<size_type>(value) % N;
    }

    // specific hash functions for string type
    static constexpr size_type Hash_Function(const std::string& value)
    {
        using std::pow;
        constexpr short unsigned prime_chosen = 263;
        constexpr long unsigned carol_prime = pow(pow(2, 15) - 1, 2) - 2;
        int64_t hash = 0;

        for(auto&& it : value){
            hash += ((prime_chosen * carol_prime) ^ (prime_chosen * hash + it)) % carol_prime;
        }
        return hash % N;
    }

    // specific hash functions for char type
    static constexpr size_type Hash_Function(const char& value)
    {
        unsigned int hash = 0xAAAAAAAA;
        return ((value & 1) == 0) ? (  (hash << 7) ^ (value) * (hash >> 3)) % N
                                  : (~((hash << 11) + (value ^ (hash >> 5)) ))  % N;
    }

    static constexpr size_type Hash_Function(const unsigned char& value)
    {
        unsigned int hash = 0xAAAAAAAA;
        return ((value & 1) == 0) ? (  (hash << 7) ^ (value) * (hash >> 3)) % N
                                  : (~((hash << 11) + (value ^ (hash >> 5)) ))  % N;
    }

    static constexpr size_type Hash_Function(const signed char& value)
    {
        unsigned int hash = 0xAAAAAAAA;
        return ((value & 1) == 0) ? (  (hash << 7) ^ (value) * (hash >> 3)) % N
                                  : (~((hash << 11) + (value ^ (hash >> 5)) ))  % N;
    }
};

template<typename _Tp, std::size_t N = 100>
class Hashtable_Chaining : public Hashing<_Tp, N>
{
 public:
    using value_type = _Tp;
    using reference = _Tp&;
    using const_reference = const _Tp&;
    using size_type = std::size_t;

 private:
    class DoublyLinkedList
    {
    public:
        class Node
        {
        public:
            using Node_pointer = Node*;
            using Const_Node_ptr = const Node*;

        private:
            Node_pointer next;
            Node_pointer prev;
            value_type* key;

        public:
            constexpr Node() : next(nullptr), prev(nullptr), key(nullptr) {}

            constexpr Node(value_type _key) : Node() { key = new value_type(_key); }

            Node(const Node& other) : next(other.next), prev(other.prev), key(nullptr)
            {
                if(other.key) {
                    key = new value_type(*other.key);
                }
            }

            constexpr Node(Node&& other) noexcept : next(std::move(other.next)), prev(std::move(other.prev)), key(std::move(other.key)) {}

            constexpr Node& operator=(const Node& other)
            {
                if(this == &other) {
                    return (*this);
                }

                this->~Node();
                next = other.next;
                prev = other.prev;

                if(other.key){
                    key = new value_type(*other.key);
                } else {
                    key = nullptr;
                }
                return (*this);
            }

            constexpr Node& operator=(Node&& other) noexcept
            {
                this->~Node();

                next = other.next;
                prev = other.prev;
                key = other.key;

                other.next = nullptr;
                other.prev = nullptr;
                other.key = nullptr;
                return (*this);
            }

            virtual ~Node()
            {
                clear();
            }

            constexpr void setKey(value_type _key)
            {
                if(!key){
                    key = new value_type(_key);
                }
                else{
                    *key = _key;
                }
            }

            constexpr reference getKey()
            {
                return *key;
            }

            constexpr const_reference getKey() const
            {
                return *key;
            }

            constexpr void createNext(value_type _key)
            {
                if(next) {
                    return;
                }

                next = new Node(_key);
                next->prev = this;
            }

            constexpr void setNext(const Node_pointer& newNext)
            {
                if(next && next->prev == this){
                    next->prev = nullptr;
                }
                next = newNext;

                if(newNext){
                    if(newNext->prev){
                        newNext->prev->next = nullptr;
                    }
                    newNext->prev = this;
                }
            }

            constexpr Node_pointer getNext()
            {
                return next;
            }

            constexpr Const_Node_ptr getNext() const
            {
                return next;
            }

            constexpr void createPrevious(value_type _key)
            {
                if(prev) {
                    return;
                }

                prev = new Node(_key);
                prev->next = this;
            }

            constexpr void setPrevious(const Node_pointer& NewPrevious)
            {
                if(prev && prev->next == this){
                    prev->next = nullptr;
                }
                prev = NewPrevious;

                if(NewPrevious){
                    if(NewPrevious->next){
                        NewPrevious->next->prev = nullptr;
                    }
                    NewPrevious->next = this;
                }
            }

            constexpr Node_pointer getPrevious()
            {
                return prev;
            }

            constexpr Const_Node_ptr getPrevious() const
            {
                return prev;
            }

            void clear()
            {
                if(key){
                    delete key;
                    key = nullptr;
                }
                if(next && next->prev == this){
                    next->prev = nullptr;
                }
                if(prev && prev->next == this){
                    prev->next = nullptr;
                }
                prev = next = nullptr;
            }
        };

        using Node_ptr = typename Node::Node_pointer;
        using ConstNPtr = typename Node::Const_Node_ptr;

        constexpr static Node_ptr MakeNode(value_type key)
        {
            return new Node(key);
        }

    protected:
        Node_ptr head;
        Node_ptr tail;
        size_type length;

    public:
        constexpr DoublyLinkedList() : head(nullptr), tail(nullptr), length(0) {}

        constexpr DoublyLinkedList(std::initializer_list<value_type> initList) : DoublyLinkedList()
        {
            for(auto&& i : initList) {
                insert(i);
            }
        }

        constexpr DoublyLinkedList(const DoublyLinkedList& other) : DoublyLinkedList()
        {
            if(!other.head){
                head = tail = nullptr; length = 0;
                return;
            }
            else{
                head = MakeNode(other.head->getKey());

                Node_ptr current = head;
                Node_ptr other_current = other.head->getNext();
                while(other_current)
                {
                    current->createNext(other_current->getKey());
                    current = current->getNext();
                    other_current = other_current->getNext();
                }
                tail = current;
                length = other.length;
            }
        }

        constexpr DoublyLinkedList(DoublyLinkedList&& other) noexcept
            : head(std::move(other.head)), tail(std::move(other.tail)), length(std::move(other.length)) {}

        constexpr DoublyLinkedList& operator=(const DoublyLinkedList& other)
        {
            if(this == &other) {
                    return (*this);
            }

            clear();

            if(!other.head){
                return (*this);
            }
            else{
                head = MakeNode(other.head->getKey());

                Node_ptr current = head;
                Node_ptr other_current = other.head->getNext();
                while(other_current)
                {
                    current->createNext(other_current->getKey());
                    current = current->getNext();
                    other_current = other_current->getNext();
                }
                tail = current;
                length = other.length;
                return (*this);
            }
        }

        constexpr DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept
        {
            clear();
            head = other.head;
            tail = other.tail;
            other.head = nullptr;
            other.tail = nullptr;
            return (*this);
        }

        virtual ~DoublyLinkedList()
        {
            clear();
        }

        void clear()
        {
            if(length)
            {
                Node_ptr current = head;
                while(current)
                {
                    Node_ptr delTmp = current;
                    current = current->getNext();
                    delete delTmp;
                }
                head = tail = nullptr; length = 0;
            }
        }

        constexpr void push_front(const_reference value)
        {
            if(!length){
                head = MakeNode(value);
                length++;
                tail = head;
            } else {
                head->createPrevious(value);
                length++;
                head = head->getPrevious();
            }
        }

        constexpr void push_back(const_reference value)
        {
            if(!length){
                head = MakeNode(value);
                length++;
                tail = head;
            }
            else {
                tail->createNext(value);
                length++;
                tail = tail->getNext();
            }
        }

        constexpr void pop_front()
        {
            if(!length){
                return;
            }
            else if(length == 1){
                delete head;
                length--;
                head = tail = nullptr;
            }
            else{
                Node_ptr tmp = head;
                head = head->getNext();
                delete tmp;
                length--;
            }
        }

        constexpr void pop_back()
        {
            if(!length){
                return;
            }
            else if(length == 1){
                delete head;
                length--;
                head = tail = nullptr;
            }
            else{
                Node_ptr tmp = tail;
                tail = tail->getPrevious();
                delete tmp;
                length--;
            }
        }

        constexpr Node_ptr search(const_reference value) const
        {
            Node_ptr current = head;
            while(current)
            {
                if(current->getKey() == value){
                    return current;
                }
                current = current->getNext();
            }
            return nullptr;
        }

        constexpr size_type erase(const_reference value)
        {
            size_type count = 0;
            Node_ptr current = head;
            while(current)
            {
                if(current->getKey() == value)
                {
                    Node_ptr temp = current;
                    if(temp == head){
                        current = current->getNext();
                        pop_front();
                    }
                    else if(temp == tail){
                        current = current->getNext();
                        pop_back();
                    }
                    else{
                        Node_ptr tmp_prev = temp->getPrevious();
                        current =  temp->getNext();
                        current->setPrevious(tmp_prev);
                        tmp_prev->setNext(current);

                        delete temp;
                        length--;
                    }
                    count++;
                }
                else{
                    current = current->getNext();
                }
            }
            return count;
        }

        constexpr void display(std::ostream& out) const
        {
            if(empty())
                return;

            Node_ptr current = head;
            while(current->getNext()){
                out << current->getKey() << " -> ";
                current = current->getNext();
            }
            out << current->getKey();
            out << '\n';
        }

        constexpr void display() const
        {
            display(std::cout);
        }

        constexpr bool empty() const
        {
            return (!length) ? true : false;
        }

        constexpr size_type size() const
        {
            return length;
        }
    };

 public:
    using DLL_ptr = DoublyLinkedList*;
    using Node_ptr = typename DoublyLinkedList::Node_ptr;
    using Const_Node_ptr = typename DoublyLinkedList::ConstNPtr;

 protected:
    DLL_ptr arr;
    size_type counter;

 public:
    constexpr Hashtable_Chaining() : arr(new DoublyLinkedList[N]), counter(0) {}

    constexpr Hashtable_Chaining(std::initializer_list<value_type> initList) : Hashtable_Chaining()
    {
        for(auto&& value : initList){
            insert(value);
        }
    }

    constexpr Hashtable_Chaining(const Hashtable_Chaining& other) noexcept : Hashtable_Chaining()
    {
        for(size_type i = 0; i < N; ++i) {
            arr[i] = other.arr[i];
        }
        counter = other.counter;
    }

    constexpr Hashtable_Chaining(Hashtable_Chaining&& other) noexcept : arr(std::move(other.arr)), counter(std::move(other.arr_cout)) {}

    constexpr Hashtable_Chaining& operator=(const Hashtable_Chaining& other)
    {
        if(this == &other) { return (*this); }

        for(size_type i = 0; i < N; ++i){
            arr[i] = other.arr[i];
        }
        counter = other.counter;

        return (*this);
    }

    constexpr Hashtable_Chaining& operator=(Hashtable_Chaining&& other) noexcept
    {
        this->~Hashtable_Chaining();
        arr = other.arr;
        counter = other.counter;
        other.arr = nullptr;
        other.counter = 0;
        return (*this);
    }

    virtual ~Hashtable_Chaining()
    {
        if(arr){
            delete [] arr;
            arr = nullptr;
            counter = 0;
        }
    }

    constexpr size_type size()
    {
        return counter;
    }

    constexpr bool empty()
    {
        return (!counter) ? true : false;
    }

    constexpr bool insert(const_reference value)
    {
        size_type index = this->Hash_Function(value);
        if(!arr[index].search(value))
        {
            arr[index].push_back(value);
            counter++;
            return true;
        }
        else{
            return false;
        }
    }

    constexpr bool search(const_reference value)
    {
        size_type index = this->Hash_Function(value);
        return (arr[index].search(value) != nullptr) ? true : false;
    }

    constexpr size_type erase(const_reference value)
    {
        size_type index = this->Hash_Function(value);
        size_type erase_count = arr[index].erase(value);
        counter -= erase_count;
        return erase_count;
    }

    void clear() override
    {
        if(counter){
            counter = 0;
            delete arr;
            arr = new DoublyLinkedList[N];
        }
    }

    constexpr void display(std::ostream& out) const
    {
        for(size_type i = 0; i < N; ++i)
        {
            if(!arr[i].empty())
            {
                out << "List #" << i + 1 << ": ";
                arr[i].display(out);
            }
        }
    }

    constexpr void display() const
    {
        display(std::cout);
    }
};

template<typename _Tp, std::size_t N = 100>
class Hashtable_Probing : public Hashing<_Tp, N>
{
 public:
    using value_type = _Tp;
    using reference = _Tp&;
    using const_reference = const _Tp&;
    using size_type = std::size_t;
    static const size_type npos = -1;

 private:
    class data_wrapper
    {
     private:
        value_type* data;
        bool flag;

     public:
        data_wrapper() : data(nullptr), flag(false) {}

        data_wrapper(const_reference value) : data(new value_type(value)), flag(true) {}

        virtual ~data_wrapper() { delete_data(); flag = false; }

        data_wrapper(const data_wrapper& other) : data(nullptr), flag(false)
        {
            if(other.data != nullptr)
            {
                data = new value_type(*other.data);
                flag = true;
            }
        }

        data_wrapper(data_wrapper&& other) noexcept : data(other.data), flag(other.flag) { other.data = nullptr; other.flag = false; }

        data_wrapper& operator=(const data_wrapper& other)
        {
            if(this == &other)
                return *this;

            delete_data();

            if(other.data != nullptr)
            {
                data = new value_type(*other.data);
                flag = true;
            }
            else
            {
                data = nullptr;
                flag = false;
            }

            return *this;
        }

        data_wrapper& operator=(data_wrapper&& other) noexcept
        {
            delete_data();
            data = other.data;
            flag = other.flag;
            other.data = nullptr;
            other.flag = false;
            return *this;
        }

        void delete_data()
        {
            if(data != nullptr)
            {
                delete data;
                data = nullptr;
            }
        }

        constexpr void set_data(const_reference value)
        {
            if(!data)
            {
                data = new value_type(value);
                flag = true;
            }
            else
            {
                *data = value;
            }
        }

        constexpr reference get_data()
        {
            return *data;
        }

        constexpr const_reference get_data() const
        {
            return *data;
        }

        constexpr void to_blank()
        {
            if(data != nullptr)
            {
                delete data;
                data = nullptr;
            }
            flag = false;
        }

        constexpr bool is_tombstone() const
        {
            return (data == nullptr && flag == true) ? true : false;
        }

        constexpr bool is_blank() const
        {
            return (data == nullptr && flag == false) ? true : false;
        }

        constexpr bool is_full() const
        {
            return (data != nullptr) ? true : false;
        }
    };

 protected:
    data_wrapper* arr;
    size_type counter;

 public:
    Hashtable_Probing() : arr(new data_wrapper[N]), counter(0) { static_assert(N != 0, "Size of the table cannot be 0"); }

    Hashtable_Probing(std::initializer_list<value_type> value_list) : Hashtable_Probing()
    {
        for(auto&& value : value_list)
            insert(value);
    }

    virtual ~Hashtable_Probing()
    {
        if(arr != nullptr)
        {
            delete[] arr;
            arr = nullptr;
            counter = 0;
        }
    }

    Hashtable_Probing(const Hashtable_Probing& other) : Hashtable_Probing()
    {
        for(size_type i = 0; i < N; i++)
            arr[i] = other.arr[i];

        counter = other.counter;
    }

    Hashtable_Probing(Hashtable_Probing&& other) noexcept : arr(other.arr), counter(other.counter) { other.arr = nullptr; counter = 0; }

    Hashtable_Probing& operator=(const Hashtable_Probing& other)
    {
        if(this == &other)
            return *this;

        for(size_type i = 0; i < N; i++)
            arr[i] = other.arr[i];

        counter = other.counter;
        return *this;
    }

    Hashtable_Probing& operator=(Hashtable_Probing&& other) noexcept
    {
        if(arr != nullptr)
            delete[] arr;

        arr = other.arr;
        counter = other.counter;
        other.arr = nullptr;
        other.counter = 0;
        return *this;
    }

    void clear()
    {
        for(size_type i = 0; i < N; i++)
            arr[i].to_blank();

        counter = 0;
    }

    size_type count() const
    {
        return counter;
    }

    size_type size() const
    {
        return N;
    }

    bool empty() const
    {
        return (counter == 0) ? true : false;
    }

    bool full() const
    {
        return (counter == N) ? true : false;
    }

    bool insert(const_reference value)
    {
        // If the table is already fulfilled, do nothing
        if(this->full())
            return false;

        size_type tomb_note = npos;
        size_type search_counter = 0;
        size_type index = this->Hash_Function(value);

        while(!arr[index].is_blank() && search_counter != N)
        {
            // If an entry with this value is already exist, don't attempt to insert anymore
            if(arr[index].is_full() && arr[index].get_data() == value)
            {
                break;
            }
            // If a tombstone is found, store this tombstone entry
            else if(arr[index].is_tombstone() && tomb_note == npos)
            {
                tomb_note = index;
            }

            search_counter++;
            index = (index + 1) % N;
        }

        if(!arr[index].is_blank() && search_counter != N)
        {
            return false;
        }
        else if(arr[index].is_blank() && tomb_note != npos && search_counter != N)
        {
            arr[tomb_note].set_data(value);
            counter++;
            return true;
        }
        else if(arr[index].is_blank() && tomb_note == npos && search_counter != N)
        {
            arr[index].set_data(value);
            counter++;
            return true;
        }
        else
        {
            arr[tomb_note].set_data(value);
            counter++;
            return true;
        }
    }

    bool search(const_reference value) const
    {
        // search_counter is used to avoid infinite loop
        size_type search_counter = 0;

        size_type index = this->Hash_Function(value);
        while(!arr[index].is_blank() && search_counter != N)
        {
            // If an entry with this value is found, stop traversing
            if(arr[index].is_full() && arr[index].get_data() == value)
                break;

            index = (index + 1) % N;
            search_counter++;
        }

        // false when the fulfilled table does not contain the value
        if(search_counter == N)
        {
            return false;
        }
        // false when stop at blank wrapper
        else if(search_counter != N && arr[index].is_blank())
        {
            return false;
        }
        // otherwise, if the current index is a full entry, the value is found
        else
        {
            return true;
        }
    }

    size_type erase(const_reference value)
    {
        // search_counter is used to avoid infinite loop
        size_type search_counter = 0;

        size_type index = this->Hash_Function(value);
        while(!arr[index].is_blank() && search_counter != N)
        {
            // If an entry with this value is found, stop traversing
            if(arr[index].is_full() && arr[index].get_data() == value)
                break;

            index = (index + 1) % N;
            search_counter++;
        }

        // do nothing when the fulfilled table does not contain the value
        if(search_counter == N)
        {
            return 0;
        }
        // do nothing when stop at blank wrapper
        else if(search_counter != N && arr[index].is_blank())
        {
            return 0;
        }
        // otherwise, this is the entry to be deleted
        else
        {
            arr[index].delete_data();
            counter--;
            return 1;
        }
    }

    void display(std::ostream& out) const
    {
        for(size_type i = 0; i < N; i++)
        {
            if(arr[i].is_full())
                out << "Entry #" << i + 1 << ":  " << arr[i].get_data() << "\n";
        }
    }

    void display() const
    {
        return display(std::cout);
    }
};
}

#endif // HASHTABLE_HPP_INCLUDED
