#pragma once
#include"prepare.h"
#include<cassert>
namespace uuz
{
	template<typename T,typename A>
	class list;

	template<typename T>
	struct list_node
	{
		list_node() = default;
		list_node(const T& p):dat{p}{}
		list_node(T&& p):dat{std::move(p)}{}
		void destory()noexcept
		{
			if (next != nullptr)
				next->destory();
			delete this;
			return;
		}
		list_node* next = nullptr;
		list_node* last = nullptr;
		T dat;
	};

	template<typename T, typename Allocator = uuz::allocator>
	class list_iterator
	{
		using self = list_iterator;
		friend list<T, Allocator>;
	public:
		self& operator++() noexcept
		{
			t= t->next;
			return *this;
		}
		self operator++(int)noexcept
		{
			auto p{ *this };
			t = t->next;
			return p;
		}
		self& operator--()noexcept
		{
			t = t->last;
			return *this;
		}
		self operator--(int) noexcept
		{
			auto p{ *this };
			t = t->last;
			return p;
		}

		T& operator*()noexcept
		{
			return t->dat;
		}
		const T& operator*()const noexcept
		{
			return t->dat;
		}
		T* operator->()noexcept
		{
			return &(t->dat);
		}
		const T* operator->()const noexcept
		{
			return &(t->dat);
		}

		friend bool operator==(const self& a, const self& b)noexcept
		{
			return a.t == b.t;
		}
		friend bool operator!=(const self& a, const self& b)noexcept
		{
			return !(a == b);
		}
		friend bool operator<(const self& a, const self& b)noexcept
		{
			return a.t < b.t;
		}
		friend bool operator>(const self& a, const self& b)noexcept
		{
			return b < a;
		}
		friend bool operator<=(const self& a, const self& b)noexcept
		{
			return a < b || a == b;
		}
		friend bool operator>=(const self& a, const self& b)noexcept
		{
			return a > b || a == b;
		}

	private:
		self(list_node* tt):t(tt){}
		self(const list_node* tt):t(const_cast<list_node*>(tt)){}
		list_node* t = nullptr;
	};

	template<typename T,typename Allocator = uuz::allocator>
	class list
	{
		using iterator = list_iterator<T, Allocator>;
		using self = list;
		using node = list_node<T>;
		using size_t = _Uint32t;
	public:
		list()
		{
			nul->next = &nul;
			nul->last = &nul;
		}
		list(size_t t, const T& p):list()
		{
			if (t != 0)
			{
				auto k = new node(p);
				auto pp = k;
				for (auto i = 1; i < t; ++i)
				{
					auto temp = new node(p);
					pp->next = temp;
					temp->last = p;
					pp = temp;
				}
				charu(&nul, k, pp);
			}
		}
		explicit list(size_t t) :list(t, T{}) {}
		template< typename InputIt,typename = decltype(*(std::declval<InputIt>()))>
		list(const InputIt& _first, const InputIt& _last):list()
		{
			if (_first != _last)
			{
				auto k = new node(*_first);
				auto p = k;
				auto ios = _first;
				++ios;
				for (auto i = ios; i != _last; ++i)
				{
					auto temp = new node(*i);
					p->next = temp;
					temp->last = p;
					p = temp;
				}
				charu(&nul, k, p);
			}
		}
		list(const std::initializer_list<T>& init) :list(init.begin(), init.end()) {}
		list(const self& t):list(t.begin(),t.end()){}
		list(self&& t):self()
		{
			this->swap(t);
		}

		list& operator=(const list& other)noexcept
		{
			if (this == &other)
				return *this;
			auto temp(other);
			this->swap(other);
			return *this;
		}
		list& operator=(list&& other)noexcept
		{
			if (this == &other)
				return *this;
			auto temp(std::move(other));
			this->swap(other);
			return *this;
		}
		list& operator=(const std::initializer_list<T>& ilist)noexcept
		{
			auto temp(ilist);
			this->swap(other);
			return *this;
		}

		void assign(size_t count, const T& value)
		{
			auto temp(count, value);
			this->swap(temp);
		}
		template< typename InputIt ,typename = decltype(*(std::declval<InputIt>()))>
		void assign(const InputIt& _first, const InputIt& _last)
		{
			auto temp(_first, _last);
			this->swap(temp);
		}
		void assign(const std::initializer_list<T>& ilist)
		{
			auto temp{ ilist };
			this->swap(temp);
		}

		T& front()
		{
			return nul->next->dat;
		}
		const T& front()const
		{
			return nul->next->dat;
		}

		T& back()
		{
			return nul->last->dat;
		}
		const T& back()const
		{
			return nul->last->dat;
		}

		iterator begin()noexcept
		{
			return iterator(nul->next);
		}
		const iterator begin()const noexcept
		{
			return iterator{ nul->next };
		}
		const iterator cbegin()const noexcept
		{
			return iterator{ nul->next };
		}

		iterator end()noexcept
		{
			return iterator{ &nul };
		}
		const iterator end()const noexcept
		{
			return iterator{ &nul };
		}
		const iterator end()const noexcept
		{
			return iterator{ &nul };
		}

		bool empty()const noexcept
		{
			return nul->next == &nul;
		}

		size_t size()const
		{
			size_t t = 0;
			auto k = nul->next;
			while (k != &nul)
			{
				k = k->next;
				++t;
			}
			return t;
		}
		
		size_t max_size()const
		{
			return size();
		}

		void clear()
		{
			nul->last->next = nullptr;
			nul->next->destory();
			nul->last = nullptr;
			nul->next = nullptr;
		}

		iterator insert(const iterator& pos, const T& value)
		{
			return emplace(pos, value);
		}
		iterator insert(const iterator& pos, T&& value)
		{
			return emplace(pos, std::move(value));
		}
		iterator insert(const iterator& pos, size_t count, const T& value)
		{
			auto k = new node(value);
			auto t = k;
			for (auto i = 1; i != count; ++i)
			{
				auto temp = new node(value);
				t->next = temp;
				temp->last = t;
				t = last;
			}
			charu(*pos, k, t);
			return iterator{ k };
		}
		template<typename InputIt ,typename = decltype(*(std::declval<InputIt>()))>
		iterator insert(const iterator& pos, const InputIt& _first, const InputIt& _last)
		{

			auto k = new node(*_first);
			auto p = _first;
			++p;
			auto t = k;
			for (auto i = p; i != _last; ++i)
			{
				auto temp = new node(*p);
				t->next = temp;
				temp->last = t;
				t = last;
			}
			charu(*pos, k, t);
			return iterator{ k };
		}
		iterator insert(const iterator& pos, std::initializer_list<T> ilist)
		{
			return insert(pos, ilist.begin(), ilist.end()));
		}

		template< class... Args >
		iterator emplace(const iterator& pos, Args&&... args)
		{
			T temp(std::forward<Args>(args)...);
			auto k = new node(std::move(temp));
			charu(*pos, k, k);
		}

		iterator erase(const iterator& pos)
		{
			auto temp = pos;
			return erase(pos, ++pos);
		}
		iterator erase(const iterator& first, const iterator& last)
		{
			if (first == begin() && last == end())
			{
				clear();
				return end();
			}
			first.t->last->next = last.t;
			last.t->last = first.t->last;
			last.t->last->next = nullptr;
			first.t->destory();
		}

		void push_back(const T& value)
		{
			emplace_back(value);
		}
		void push_back(T&& value)
		{
			emplace_back(std::move(value));
		}

		template< typename... Args >
		T& emplace_back(Args&&... args)
		{
			return *(emplace(end(), std::forward<Args>(args)...));
		}

		void pop_back()
		{
			erase(--end());
		}

		void push_front(const T& value)
		{
			emplace_front(value);
		}
		void push_front(T&& value)
		{
			emplace_front(std::move(value));
		}

		template< class... Args >
		T& emplace_front(Args&&... args)
		{
			return *(emplace(begin(), std::forward<Args>(args)...));
		}

		void pop_front()
		{
			erase(begin());
		}

		void resize(size_t count)
		{
			return resize(count, T{});
		}
		void resize(size_t count, const T& value)
		{
			auto p = nul->next;
			while (p != &nul && count != 0)
			{
				p = p->next;
				--count;
			}
			if (p == &nul && count != 0)
			{
				auto k = new node(value);
				--count;
				auto l = k;
				while (count--)
				{
					auto temp = new node(value);
					l->next = temp;
					temp->last = l;
					l = temp;
				}
				charu(end(), k, l);
			}
			else if (p != &nul&&count == 0)
				erase(iterator(p), end());
		}

		void swap(list& other)noexcept
		{
			using std::swap;
			swap(nul->next, other.nul->next);
			swap(nul->last, other.nul->last);
		}

		void merge(list& other)
		{
			return merge(other, pre_less<T, nil>());
		}
		void merge(list&& other)
		{
			return merge(std::move(other), pre_less<T, nil>());
		}
		template <typename Compare>
		void merge(list& other, Compare comp)
		{
			if (other.empty() || *this == other)
				return;
			if (this->empty())
				return this->swap(other);
			auto ob = other.nul->next;
			auto oe = other.nul->last;
			auto q = nul.next;
			while (q!=&nul && ob != &(other.nul))
			{
				if (comp(ob->dat, q->dat))
				{
					ob->last = q->last;
					q->last->next = ob;
					q->last = ob;
					auto ok = ob->next;
					ob->next = q;
					ob = ok;
				}
				else
					q = q->next;
			}
			if (q == &nul && ob != &(other.nul))
			{
				q = q->last;
				q->next = ob;
				ob->last = q;
				oe->next = nul;
				nul.last = oe;
			}
			other.nul->next = other.nul->last = &nul;
		}
		template <typename Compare>
		void merge(list&& other, Compare comp)
		{
			auto t{ std::move(other) };
			return merge(t, comp);
		}

		void splice(const_iterator pos, list& other)
		{

		}
		void splice(const iterator pos, list&& other);
		void splice(const_iterator pos, list& other, const_iterator it);
		void splice(const_iterator pos, list&& other, const_iterator it);
		void splice(const_iterator pos, list& other,
			const_iterator first, const_iterator last);
		void splice(const_iterator pos, list&& other,
			const_iterator first, const_iterator last);

		void remove(const T& value)
		{
			return remove_if([&](const T& i) {return i == value; });
		}
		template< typename UnaryPredicate >
		void remove_if(const UnaryPredicate& p)
		{
			auto l = nul->next;
			while (l != &nul)
			{
				if (p(l->dat))
				{
					l->last->next = l->next;
					l->next->last = l->last;
					auto w = l->next;
					delete l;
					l = w;
				}
			}
		}

		void reverse()noexcept
		{
			auto k = &nul->next;
			while (k != &nul)
			{
				using std:swap;
				std::swap(k->next, k->last);
				k = k->last;
			}
			std::swap(nul->last, nul->next);
		}

		void unique()
		{
			return unique([](const T& a, const T& b) {return a == b; });
		}
		template< typename BinaryPredicate >
		void unique(const BinaryPredicate& p)
		{
			auto k = nul->next;
			while (k->next != &nul)
			{
				if (p(k->dat, k->next->dat))
					auto t = k->next;
				{
					k->next = k->next->next;
					k->next->last = k;
					delete t;
				}
				else
					k = k->next;
			}
		}

		void sort()
		{
			return sort([](const T& a, const T&b) {return a < b; });
		}
		template<typename Compare >
		void sort(const Compare& comp)
		{
			if (empty() || nul->next == nul->last)
				return;

		}

		template< class T, class Alloc >
		bool operator==(const list<T, Alloc>& lhs,
			const list<T, Alloc>& rhs);
	
			template< class T, class Alloc >
		bool operator!=(const list<T, Alloc>& lhs,
			const list<T, Alloc>& rhs);
		
			template< class T, class Alloc >
		bool operator<(const list<T, Alloc>& lhs,
			const list<T, Alloc>& rhs);
		
			template< class T, class Alloc >
		bool operator<=(const list<T, Alloc>& lhs,
			const list<T, Alloc>& rhs);
		
			template< class T, class Alloc >
		bool operator>(const list<T, Alloc>& lhs,
			const list<T, Alloc>& rhs);
		
			template< class T, class Alloc >
		bool operator>=(const list<T, Alloc>& lhs,
			const list<T, Alloc>& rhs);

		template< typename T, typename Alloc >
		void swap(list<T, Alloc>& lhs, list<T, Alloc>& rhs)
		{
			lhs.swap(rhs);
		}

		~list()
		{
			clear();
		}
	private:
		void charu(node* d,node* b, node* e)
		{
			if (empty())
			{
				nul->next = b;
				b->last = &nul;
				nul->last = e;
				e->next = &nul;
			}
			else if (*end() == d)
			{
				nul->last->next = b;
				b->last = nul->last;
				nul->last = e;
				e->next = &nul;
			}
			else if (d == *begin())
			{
				nul->next->last = e;
				e->next = nul->next;
				nul->next = b;
				b->last = &nul;
			}
			else
			{
				auto kk = d->last;
				kk->next =b;
				b->last = kk;
				e->next = d;
				d->last = e;
			}
		}
		node nul;
	};
}