#pragma once
#include"container.h"
#include"pair.h"
#include<functional>
namespace uuz
{
	template<typename T, typename Compare, typename A>
	class rb_tree;

	template<typename T>
	struct rb_tree_node
	{
		rb_tree_node() = default;
		rb_tree_node(const T& value):data(value){}
		rb_tree_node(T&& value) :data(std::move(value)) {}
		template<typename...Args>
		rb_tree_node( Args&&... args) : data(std::forward<Args>(args)...) {}
		
		void destroy()noexcept
		{
			data.data.destroy();
			this->~rb_tree_node();
			return;
		}

		rb_tree_node* grandfather()const noexcept
		{
			return father->father;
		}

		rb_tree_node* brother()const noexcept
		{
			if (father->left == this)
				return father->right;
			return father->left;
		}

		rb_tree_node* uncle()const noexcept
		{
			return father->brother();
		}

		T& get()noexcept
		{
			return data.data.get();
		}

		union dat
		{
			dat() = default;
			dat(const T& p):data(p){}
			dat(T&& p):data(std::move(p)){}
			template<typename...Args>
			dat(Args&&...args) : data(std::forward<Args>(args)...) {}
			storage<T> data;
			rb_tree_node* end = nullptr;
		} data;
		
		bool color = false;
		rb_tree_node* father = nullptr;
		rb_tree_node* left = nullptr;
		rb_tree_node* right = nullptr;
	};

	template<typename T, typename Allocator>
	class to_node
	{
	public:
		template<typename A,typename B,typename C>
		friend class rb_tree;
		using allocator_type = Allocator;
		constexpr to_node() noexcept:alloc(Allocator()),t(nullptr){}
		to_node(to_node&& nh) noexcept:to_node()
		{
			this->swap(nh);
		}
		to_node(rb_tree_node<T>* a,const allocator_type& b):t(a),alloc(b){}

		to_node& operator=(to_node&& nh)noexcept(is_nothrow_swap_alloc<Allocator>::value)
		{
			to_node temp(std::move(nh), alloc);
			this->swap(temp);
			return *this;
		}

		bool empty() const noexcept
		{
			return t == nullptr;
		}

		explicit operator bool() const noexcept
		{
			return t != nullptr;
		}

		allocator_type get_allocator() const
		{
			return alloc;
		}

		void swap(to_node& nh) noexcept(is_nothrow_swap_alloc<Allocator>::value)
		{
			using std::swap;
			swap(t, nh.t);
			swap(alloc, nh.alloc);
		}

		~to_node()
		{
			if(t)
			{
				t->destroy();
				alloc.deallocate(t, 1);
				t = nullptr;
			}
		}
	protected:
		allocator_type alloc;
		rb_tree_node<T>* t = nullptr;
	};



	
	template<typename T, typename Compare, typename A>
	class rb_tree_iterator
	{
		using self = rb_tree_iterator;
		friend rb_tree<T, Compare, A>;
	public:
		self& operator++() noexcept
		{
			increase();
			return *this;
		}
		self operator++(int)noexcept
		{
			auto p{ t };
			increase();
			return p;
		}
		self& operator--()noexcept
		{
			decrease();
			return *this;
		}
		self operator--(int) noexcept
		{
			auto p{ t };
			decrease();
			return p;
		}

		T& operator*()noexcept
		{
			return t->get();
		}
		const T& operator*()const noexcept
		{
			return t->get();
		}
		T* operator->()noexcept
		{
			return &(t->get());
		}
		const T* operator->()const noexcept
		{
			return &(t->get());
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

	//private:
		void increase()noexcept
		{
			if (!isnul(t->right))
			{
				t = t->right;
				while (!isnul(t->left))
					t = t->left;
				return;
			}
			auto y = t->father;
			while (!isnul(y) && t == y->right)
			{
				t = y;
				y = y->father;
			}
			t = y;
		}
		void decrease()noexcept
		{
			if (!isnul(t->left))
			{
				t = t->left;
				while (!isnul(t->right))
					t = t->right;
				return;
			}
			auto y = t->father;
			while (!isnul(y) && t == y->left)
			{
				t = y;
				y = y->father;
			}
			t = y;
		}

		bool isnul(rb_tree_node<T>* t)const noexcept
		{
			return t == nullptr || (t->left == t->right && t->left != nullptr);
		}
		self(rb_tree_node<T>* tt)noexcept : t(tt) {}
		self(const rb_tree_node<T>* tt)noexcept : t(const_cast<rb_tree_node<T>*>(tt)) {}
		rb_tree_node<T>* t = nullptr;
	};

	template<typename T, typename Compare=uuz::less<T>, typename A=uuz::allocator<T>>
	class rb_tree
	{
		
	public:
		using node = rb_tree_node<T>;
		
		using Allocator = typename uuz::exchange<A, node>::type;
		using node_type = to_node<T, Allocator>;
		using iterator = rb_tree_iterator<T, Compare, Allocator>;

		Allocator get_allocator() const
		{
			return alloc;
		}

		bool empty()const noexcept
		{
			return ssize == 0;
		}	 

		size_t size()const noexcept
		{
			return ssize;
		}

		void clear()noexcept
		{
			tree_destroy(root);
			root = nul.left = nul.right = nullptr;
			nul.father = &nul;
			ssize = 0;
		}

		int compare(const rb_tree& t)const noexcept(noexcept(cmp(std::declval<T>(), std::declval<T>())))
		{
			auto i = begin();
			auto j = t.begin();
			for (; i != end() && j != t.end(); ++i, (void)++j)
			{
				if (cmp(*i, *j))
					return -1;
				else if (cmp(*j, *i))
					return 1;
			}
			if (i == end() && j != t.end())
				return -1;
			else if (i != end() && j == t.end())
				return 1;
			return 0;
		}

		pair<iterator, bool> insert(const T& value)
		{
			auto k = ifind(value);
			if (!k|| cmp(value, k->get()) || cmp(k->get(), value))
			{
				auto l = make(value);
				return pair<iterator, bool>(iterator(Insert(l, k)), true);
			}	
			return pair<iterator, bool>(iterator(k), false);

		}
		pair<iterator, bool> insert(T&& value)
		{
			auto k = ifind(value);
			if (!k || cmp(value, k->get()) || cmp(k->get(), value))
			{
				auto l = make(std::move(value));
				return pair<iterator, bool>(iterator(Insert(l, k)), true);
			}
			return pair<iterator, bool>(iterator(k), false);
		}
		iterator insert(const iterator hint, const T& value)
		{
			auto k = check(hint, value);
			if (k)
			{
				auto l = make(value);
				return iterator(l, k);
			}
			return end();
		}
		iterator insert(const iterator hint, T&& value)
		{
			auto k = check(hint, value);
			if (k)
			{
				auto l = make(std::move(value));
				return iterator(l, k);
			}
			return end();
		}
		template< typename InputIt,typename = is_input<T,InputIt>>
		void insert(const InputIt& first,const InputIt& last)
		{
			try
			{
				auto i = first;
				for (; i != last; ++i)
				{
					auto k = make(*i);
					Insert(k, ifind(k));
				}
			}
			catch (...)
			{
				for (auto j = first; j != i; ++j)
					dele(ifind(*j));
				throw;
			}
		}
		void insert(std::initializer_list<T> ilist)
		{
			insert(ilist.begin(), ilist.end());
		}

		iterator insert(const iterator& hint, node_type&& nh)
		{
			auto k = check(hint, nh.t->get());
			if (!k &&(cmp(k->get(),nh.t->get())||cmp(nh.t->get(),k->get())))
			{
				if (get_allocator() == nh.get_allocator())
				{
					auto x = iterator(Insert(nh.t, k));
					nh.t = nullptr;
					return x;
				}
				else
				{
#ifdef DEBUG
					assert(false, "It's undefined that allocator is not equeal");
#else
					auto x = insert(hint, std::move(nh.t->get()));
					nh->~to_node();
					return x;
#endif 
				}
			}
			return end();
		}



		iterator erase(const iterator pos)
		{
			auto k = pos;
			++k;
			dele(pos.t);
			return k;
		}
		iterator erase(const iterator& first, const iterator& last)
		{
			if (first == begin() && last == end())
			{
				clear();
				return end();
			}
			auto k = first;
			while (k != last)
			{
				auto d = k;
				++d;
				dele(k.t);
				k = d;
			}
			return last;
		}


	
		
		Compare key_comp() const noexcept
		{
			return cmp;
		}

		template<typename...Args>
		pair<iterator, bool> emplace(Args&&...args)
		{
			auto k = make(std::forward<Args>(args)...);
			auto d = ifind(k->get());
			auto l = Insert(k, d);
			return pair<iterator, bool>(iterator(l), k == l);
		}

		template<typename...Args>
		iterator emplace_hint(const iterator& t, Args&&...args)
		{
			auto k = make(std::forward<Args>(args)...);
			auto d = check(t, k->get());
			if (!d)
				return end();
			return iterator(Insert(k, d));
		}

		iterator begin()noexcept
		{
			return iterator(nul.father);
		}
		const iterator begin()const noexcept
		{
			return iterator(nul.father);
		}
		const iterator cbegin()const noexcept
		{
			return iterator(nul.father);
		}

		const iterator end()const noexcept
		{
			return iterator(&nul);
		}
		iterator end()noexcept
		{
			return iterator(&nul);
		}
		const iterator cend()const noexcept
		{
			return iterator(&nul);
		}

		void swap(rb_tree& t)noexcept
		{
			if(t.alloc == alloc)
			{
				using std::swap;
				swap(root, t.root);
				swap(nul.father, t.nul.father);
				swap(nul.left, t.nul.left);
				swap(nul.right, t.nul.right);
				swap(nul.data.end, t.nul.data.end);

				if (empty() && !t.empty())
				{
					t.nul.father = &t.nul;
					nul.father->left = &nul;
					nul.left->father = &nul;
					nul.data.end->right = &nul;
				}
				else if (!empty() && t.empty())
				{
					nul.father = &nul;
					t.nul.father->left = &t.nul;
					t.nul.left->father = &t.nul;
					t.nul.data.end->right = &t.nul;
				}
				else if (!empty() && !t.empty())
				{
					swap(nul.father, t.nul.father);
					swap(t.nul.father->left, nul.father->left);
					swap(t.nul.left->father, nul.left->father);
					swap(t.nul.data.end->right, nul.data.end->right);
				}
				swap(ssize, t.ssize);
			}
			else
			{
#ifdef DEBUG
				assert(false, "It's undefined that alloc is not equal.");
#else
				rb_tree a(t, alloc);
				rb_tree b(*this, t.alloc);
				t.swap(b);
				swap(a);
#endif
			}
		}

		~rb_tree()noexcept
		{
			clear();
		}
	protected:	
		rb_tree()noexcept(std::is_nothrow_default_constructible_v<Allocator>) = default;
		rb_tree(const A& a):alloc(a){}
		rb_tree(const Compare& cmp,const A& a):cmp(cmp),alloc(a){}
		rb_tree(rb_tree&& t)noexcept:rb_tree(A())
		{
			if(t.alloc == alloc)
				this->swap(t);
			else
			{
#ifdef DEBUG
				assert(false, "It's undefined that alloc is not equal.");
#else
				if (!t.empty())
				{
					root = multimove(t.root, &t.nul);
					nul.left = nul.right = root;
					root->father = &nul;
				}
				ssize = t.ssize;
				t.clear();
#endif
			}
		}
		rb_tree(rb_tree&& t,const A& a)noexcept:rb_tree(a)
		{
			if(alloc==t.alloc)
				this->swap(t);
			else
			{
#ifdef DEBUG
				assert(false, "It's undefined that alloc is not equal.");
#else
				if (!t.empty())
				{
					root = multimove(t.root, &t.nul);
					nul.left = nul.right = root;
					root->father = &nul;
				}
				ssize = t.ssize;
				t.clear();
#endif
			}
		}
		rb_tree(const rb_tree& t):rb_tree(t.cmp,Allocator())
		{
			if(!t.empty())
			{
				root = copy(t.root, &t.nul);
				nul.left = nul.right = root;
				root->father = &nul;
			}
			ssize = t.ssize;
		}
		rb_tree(const rb_tree& t, const A& a) :rb_tree(a)
		{
			try
			{
				if (!t.empty())
					root = copy(t.root, &t.nul);
			}
			catch (...)
			{
				clear();
				throw;
			}
			nul.left = nul.right = root;
			root->father = &nul;
			ssize = t.ssize;
		}

		template< typename InputIt, typename = is_input<T, InputIt>>
		void inid(const InputIt& a, const InputIt& b)
		{
			try
			{
				for (auto i = a; i != b; ++i)
					insert(*i);
			}
			catch (...)
			{
				clear();
				throw;
			}
		}

		template< typename InputIt, typename = is_input<T, InputIt>>
		void mutilinid(const InputIt& a, const InputIt& b)
		{
			try
			{
				for (auto i = a; i != b; ++i)
				{
					auto t = make(*i);
					auto k = ifind(t->get());
					if (k&&!cmp(*i, k->get()) && !cmp(k->get(), *i))
					{
						k = nextnode(k);
						Insert(t, k, false);
					}
					else
						Insert(t, k);
				}
			}
			catch (...)
			{
				clear();
				throw;
			}
		}

		void multimove(node* a,const node* n)
		{
			auto b = make(std::move(a->get()));
			b->color = a->color;
			if (a->left == n)
			{
				b->left = &nul;
				nul.father = b;
			}
			else if (a->right == n)
			{
				b->right = &nul;
				nul.data.end = b;
			}
			try
			{
				if (a->left && a->left != n)
				{
					b->left = copy(a->left, n);
					b->left->father = b;
				}

				if (a->right && a->right != n)
				{
					b->right = copy(a->right, n);
					b->right->father = b;
				}

			}
			catch (...)
			{
				if (!isnul(b->left))
					tree_destroy(b->left);
				if (!isnul(b->right))
					tree_destroy(b->right);
				if (b->father->left == b)
					b->father->left = nullptr;
				else
					b->father->right = nullptr;
				destroy(b);
				throw;
			}
			return b;
		}


		rb_tree& operator=(const rb_tree& t)
		{
			rb_tree temp(t,alloc);
			this->swap(temp);
			return *this;
		}
		rb_tree& operator=(rb_tree&& t)
		{
			rb_tree temp(std::move(t), alloc);
			this->swap(temp);
			return *this;
		}

		template<typename U>
		node* truefind(const U& a)const noexcept 
		{
			auto b = root;
			while(!isnul(b))
			{
				if (cmp(b->get(), a))
					b = b->right;
				else if (cmp(a, b->get()))
					b = b->left;
				else
					return b;
			}
			return const_cast<node*>(&nul);
		}	
		template<typename U>
		node* low_bound(const U& a)const noexcept
		{
			auto k = &nul;
			auto b = root;
			while (!isnul(b))
			{
				if (cmp(b->get(), a))
					b = b->right;
				else
				{
					k = b;
					b = b->left;
				}
			}
			return k;
		}
		template<typename U>
		node* up_bound(const U& a)const noexcept
		{
			auto k = &nul;
			auto b = root;
			while (!isnul(b))
			{
				if (cmp(a, b->get()))
				{
					k = b;
					b = b->right;
				}
				else
					b = b->left;
			}
			return k;
		}
		template<typename U>
		pair<node*, node*> eqrange(const U& a)const noexcept
		{
			auto p = root;
			auto l = &nul;
			auto r = &nul;
			while (!isnul(p))
			{
				if (cmp(p->get(), a))
					p = p->right;
				else 
				{
					if (isnul(r) && cmp(a, p->get()))//??
						r = p;
					l = p;
					p = p->left;
				}
			}
			p = isnul(r) ? root : r->left;
			while (isnul(p))
			{
				if (cmp(a, p->get()))
				{
					r = p;
					p = p->left;
				}
				else
					p = p->right;
			}
			return make_pair(l, r);
		}

		node* nextnode(node* a)const noexcept
		{
			if (!isnul(a->left))
			{
				auto k = a->left;
				while (!isnul(k->right))
					k = k->right;
				return k;
			}
			return a;
		}

		node* Insert(node* k,node*b,bool flag = true)noexcept
		{
			if (!b)
			{
				root = k;
				k->father=k->left=k->right = &nul;
				nul.data.end=nul.father = nul.left = nul.right = k;
			}
			else
			{
				if (flag && !cmp(k->get(), b->get()) && !cmp(b->get(), k->get()))
				{
					destroy(k);
					return b;
				}
				else 
				{
					k->color = true;
					k->father = b;
					if (cmp(k->get(), b->get()))
					{
						if (b->left == &nul)
						{
							k->left = &nul;
							nul.father = k;
						}
						b->left = k;
					}
					else
					{
						if (b->right == &nul)
						{
							k->right = &nul;
							nul.data.end = k;
						}
						b->right = k;
					}
						
					fixinsert(k);
				}
			}
			++ssize;
			return k;
		}

		node* copy(node* a,const node* n)
		{
			auto b = make(a->get());
			b->color = a->color;
			if (a->left == n )
			{
				b->left = &nul;
				nul.father = b;
			}
			else if (a->right==n)
			{
				b->right = &nul;
				nul.data.end = b;
			}
			try
			{
				if (a->left && a->left !=n)
				{
					b->left = copy(a->left, n);
					b->left->father = b;
				}
					
				if (a->right && a->right !=n)
				{
					b->right = copy(a->right, n);
					b->right->father = b;
				}
					
			}
			catch (...)
			{
				if (!isnul(b->left))
					tree_destroy(b->left);
				if (!isnul(b->right))
					tree_destroy(b->right);
				if (b->father->left == b)
					b->father->left = nullptr;
				else
					b->father->right = nullptr;
				destroy(b);
				throw;
			}		
			return b;
		}

		void dele(node* x)
		{
			help_dele(x);
			destroy(x);
		}

		void help_dele(node* x)noexcept
		{
			node* child = nullptr;
			node* p = nullptr;
			--ssize;
			if (!isnul(x->left) && !isnul(x->right))
			{
				auto replace = successor(x);
				if (x != root)
				{
					if (x->father->left == x)
						x->father->left = replace;
					else
						x->father->right = replace;
				}
				else
				{
					root = replace;
					nul.left = nul.right = replace;
				}
				child = replace->right;
				p = replace->father;
				auto c = replace->color;
				if (p == x)
					p = replace;
				else
				{
					if (!isnul(child))
						child->father = p;
					p->left = child;
					replace->right = x->right;
					x->right->father = replace;
				}
				replace->father = x->father;
				replace->color = x->color;
				replace->left = x->left;
				x->left->father = replace;
				if (!c)
					fixdele(child, p);
				return;
			}
			if (!isnul(x->left))
				child = x->left;
			else if (!isnul(x->right))
				child = x->right;
			p = x->father;
			if (!isnul(child))
				child->father = p;
			if (!isnul(p))
			{
				if (p->left == x)
					p->left = child;
				else
					p->right = child;
			}
			else
			{
				root = child;
				nul.left = nul.right = child;
			}
			if (!x->color)
				fixdele(child, p);
			if (x->left == &nul && x->right == &nul)
			{
				nul.left = nul.right = nul.data.end = root = nullptr;
				nul.father = &nul;
			}
			else if (x->left == &nul)
			{
				auto k = minimum(p);
				k->left = &nul;
				nul.father = k;
			}
			else if (x->right == &nul)
			{
				auto k = maximum(p);
				k->right = &nul;
				nul.data.end = k;
			}
		}
		
		node* extract(const iterator p)noexcept
		{
			help_dele(p.t);
			p.t->father = p.t->left = p.t->right = nullptr;
			p.t->color = false;
			return p.t;
		}

		void print()const noexcept
		{
			auto k = nul.father;
			while (!isnul(k))
			{
				uuz::print(k->get());
				k = successor(k);
			}
			
		}

		template<typename... Args>
		node* make(Args&&...args)
		{
			node *t = nullptr;
			try
			{
				t = alloc.allocate();
				new(t) node(std::forward<Args>(args)...);
				return t;
			}
			catch (const bad_alloc&)
			{
				throw;
			}
			catch (...)
			{
				alloc.deallocate(t, 1);
				throw;
			}
		}
		
		template<typename U>
		node* check(const iterator& t, const U& k)const noexcept
		{
			if (cmp(t.t->get(), k) && isnul(t.t->right))
			{
				auto y = t;
				++y;
				if (isnul(y.t) || !cmp(y.t->get(),k))
					return t.t;
			}
			else if(isnul(t.t->left))
			{
				auto y = t;
				--y;
				if (isnul(y.t) || !cmp(k, y.t->get()))
					return t.t;
			}
			return nullptr;
		}

		void destroy(node* t)noexcept
		{
			if (!t)
				return;
			t->destroy();
			alloc.deallocate(t, 1);
		}
			
		template<typename U>
		node* ifind(const U& a)const noexcept
		{
			if (empty())
				return nullptr;
			for(auto b = root;;)
			{
				if (cmp(a, b->get()))
				{
					if (isnul(b->left))
						return b;
					b = b->left;
				}
				else if(cmp(b->get(), a))
				{
					if (isnul(b->right))
						return b;
					b = b->right;
				}
				else 
					return b;
			}
			
		}

		bool isnul(node* p)const noexcept
		{
			return !p || p == &nul;
		}

		void fixinsert(node* b)noexcept
		{
			if (!b->father->color || b->father == &nul)
				return;
			else if(b->father->color &&(!isnul(b->uncle())&&b->uncle()->color))
			{
				b->father->color = b->uncle()->color = false;
				b->grandfather()->color = true;
				return fixinsert(b->grandfather());
			}
			if (b == b->father->right && b->father == b->grandfather()->left)
			{
				lrotate(b->father);
				b = b->left;
			}
			else if(b == b->father->left && b->father == b->grandfather()->right)
			{
				rrotate(b->father);
				b = b->right;
			}
			b->father->color = false;
			b->grandfather()->color = true;
			if (b == b->father->left && b->father == b->grandfather()->left)
				rrotate(b->grandfather());
			else
				lrotate(b->grandfather());
		}

		node* minimum(node* x)const noexcept
		{
			while (!isnul(x->left))
				x = x->left;
			return x;
		}
		node* maximum(node* x)const noexcept
		{
			while (!isnul(x->right))
				x = x->right;
			return x;
		}

		node* successor(node* x)const noexcept
		{
			if (!isnul(x->right))
				return minimum(x->right);
			auto y = x->father;
			while (!isnul(y) && x == y->right)
			{
				x = y;
				y = y->father;
			}
			return isnul(y) ? nullptr : y;
		}
		node* predecessor(node* x)const noexcept
		{
			if (!isnul(x->left))
				return maximum(x->left);
			auto y = x->father;
			while (!isnul(y) && x == y->left)
			{
				x = y;
				y = y->father;
			}
			return isnul(y) ? nullptr : y;
		}

		void fixdele(node* x, node* p)noexcept
		{
			while ((isnul(x)|| !x->color) && x != root)
			{
				if (x == p->left)
				{
					auto s = p->right;
					if (s->color == true)
					{
						s->color = false;
						p->color = true;
						lrotate(p);
						s = p->right;
					}
					if ((isnul(s->left) || !s->left->color) && (isnul(s->right) || !s->right->color))
					{
						s->color = true;
						x = p;
						p = p->father;
					}
					else
					{
						if (isnul(s->right) || !s->right->color)
						{
							s->left->color = false;
							s->color = true;
							rrotate(s);
							s = p->right;
						}
						s->color = p->color;
						p->color = false;
						s->right->color = false;
						lrotate(p);
						x = root;
						break;
					}
				}
				else
				{
					auto s = p->left;
					if (s->color == true)
					{
						s->color = false;
						p->color = true;
						rrotate(p);
						s = p->left;
					}
					if ((isnul(s->left) || !s->left->color) && (isnul(s->right) || !s->right->color))
					{
						s->color = true;
						x = p;
						p = p->father;
					}
					else
					{
						if (isnul(s->left) || !s->left->color)
						{
							s->right->color = false;
							s->color = true;
							lrotate(s);
							s = p->left;
						}
						s->color = p->color;
						p->color = false;
						s->left->color = false;
						rrotate(p);
						x = root;
						break;
					}
				}
			}
			if (x)
				x->color = false;
		}

		void tree_destroy(node* k)noexcept
		{
			if (!isnul(k))
			{
				tree_destroy(k->left);
				tree_destroy(k->right);
				destroy(k);
			}
		}

		int blackhigh(node* k)
		{
			if (isnul(k))
				return 1;
			auto l = blackhigh(k->left);
			auto r = blackhigh(k->right);
			if (l != r)
				std::cout<<l<<"  "<<r<<std::endl;
			return k->color ? l : l + 1;
		}

		void rrotate(node* p)noexcept
		{
			auto x = p->left;
			auto c = x->right;
			p->left = c;
			if(c)
				c->father = p;
			x->right = p;
			x->father = p->father;
			p->father = x;
			if (x->father->left == p)
				x->father->left = x;
			if (x->father->right == p)
				x->father->right = x;
			if (x->father == &nul)
				root = x;
		}
		void lrotate(node* p)noexcept
		{
			auto x = p->right;
			auto b = x->left;
			p->right = b;
			if (b)
				b->father = p;
			x->left = p;
			x->father = p->father;
			p->father = x;
			if (x->father->left == p)
				x->father->left = x;
			if (x->father->right == p)
				x->father->right = x;
			if (x->father == &nul)
				root = x;

		}	
		
		mutable Compare cmp;
		size_t ssize = 0;
		Allocator alloc;
		node* root=nullptr;
		node nul;
	};
}