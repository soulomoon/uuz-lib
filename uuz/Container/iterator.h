#pragma once
#include<type_traits>
namespace uuz
{

	struct input_iterator_tag { };
	struct output_iterator_tag { };
	struct forward_iterator_tag : public input_iterator_tag { };
	struct bidirectional_iterator_tag : public forward_iterator_tag { };
	struct random_access_iterator_tag : public bidirectional_iterator_tag { };

	template<typename Iterator> 
	struct iterator_traits 
	{
		using difference_type =  typename Iterator::difference_type;
		using value_type =  typename Iterator::value_type;
		using pointer = typename Iterator::pointer;
		using reference = typename Iterator::reference;
		using iterator_category = typename Iterator::iterator_category;
	};

	template<typename T>
	struct iterator_traits<T*> 
	{
		using difference_type = ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = random_access_iterator_tag;
	};

	template<typename T> 
	struct iterator_traits<const T*>
	{
		using difference_type = ptrdiff_t;
		using value_type = T;
		using pointer = const T*;
		using reference = const T&;
		using iterator_category = random_access_iterator_tag;
		
	};

    template<typename Category, typename T, typename Distance = ptrdiff_t,
			typename Pointer = T*, typename Reference = T&> 
	struct iterator 
	{
		using value_type = T;
		using difference_type = Distance;
		using pointer = Pointer;
		using reference = Reference;
		using iterator_category = Category;
	};
 
    // iterator operations:
    template <typename InputIterator, typename Distance>
    void advance(InputIterator& i, Distance n)
    {
	   if constexpr(std::is_same_v<random_access_iterator_tag,typename iterator_traits<InputIterator>::iterator_category>)
	   {
		   i += n;
	   }
	   else if constexpr(std::is_same_v<bidirectional_iterator_tag, typename iterator_traits<InputIterator>::iterator_category>)
	   {
		   if(n>=0)
			 for (Distance j = 0; j != n; ++j)
				   ++i;
		   else
			   for (Distance j = 0; j != n; --j)
				   --i;
	   }
	   else if constexpr(std::is_same_v<input_iterator_tag, typename iterator_traits<InputIterator>::iterator_category>)
	   {
		   if (n >= 0)
			   for (Distance j = 0; j != n; ++j)
				   ++i;
		   else
#ifdef DEBUG
			   assert(false, "only bidirectional_iterator can sub");
#else
			   throw("");
#endif
	   }
	   else
	   {
		   static_assert(false);
	   }
    }
    template <class InputIterator>
	typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last)
    {
		if constexpr(std::is_same_v<random_access_iterator_tag, typename iterator_traits<InputIterator>::iterator_category>)
		{
			return last - first;
		}
		else if constexpr(std::is_same_v<input_iterator_tag, typename iterator_traits<InputIterator>::iterator_category>)
		{
			typename iterator_traits<InputIterator>::difference_type temp = 0;
			for (auto j = first; j != last; ++j)
				++temp;
			return temp;
		}
		else
		{
			static_assert(false);
		}
    }
    template <class ForwardIterator>
    ForwardIterator next(ForwardIterator x,
		typename std::iterator_traits<ForwardIterator>::difference_type n = 1)
    {
		advacne(x, n);
		return x;
    }
    template <class BidirectionalIterator>
        BidirectionalIterator prev(BidirectionalIterator x,
            typename std::iterator_traits<BidirectionalIterator>::difference_type n = 1)
    {
			std::advance(x, n);
			return x;
    }
 
    // predefined iterators:
		template <typename Iterator>
		class reverse_iterator : public Iterator {
		public:
			using difference_type = typename iterator_traits<Iterator>::difference_type;
			using value_type = typename iterator_traits<Iterator>::value_type;
			using pointer = typename iterator_traits<Iterator>::pointer;
			using reference = typename iterator_traits<Iterator>::reference;
			using iterator_category = typename iterator_traits<Iterator>::iterator_category;
			using  iterator_type = Iterator;

			reverse_iterator() = default;
			explicit reverse_iterator(Iterator x):Iterator(x){}
			template <typename U,typename = std::enable_if_t<std::is_constructible_v<Iterator,U>>>
    		reverse_iterator(const reverse_iterator<U>& u):Iterator(U(u)){}
			
    		template <typename U, typename = std::enable_if_t<std::is_constructible_v<Iterator, U>>>
    		reverse_iterator& operator=(const reverse_iterator<U>& u)
			{
				reverse_iterator temp(u);
				this->swap(temp);
				return *this;
			}

			Iterator base() const noexcept
			{
				return *this;
			}
			reference operator*() const noexcept(noexcept(Iterator::operator*()))
			{
				return Iterator::operator*();
			}
			pointer operator->() const noexcept(noexcept(Iterator::operator->()))
			{
				return Iterator::operator->();
			}

			reverse_iterator& operator++()noexcept(noexcept(Iterator::operator++()))
			{
				Iterator::operator++();
				return *this;
			}
			reverse_iterator  operator++(int)noexcept(noexcept(Iterator::operator++(1)))
			{
				return reverse_iterator(Iterator::operator++(1));
			}
			reverse_iterator& operator--()noexcept(noexcept(Iterator::operator--()))
			{
				Iterator::operator--();
				return *this;
			}
			reverse_iterator  operator--(int)noexcept(noexcept(Iterator::operator--(1)))
			{
				return reverse_iterator(Iterator::operator--(1));
			}

			reverse_iterator  operator+ (difference_type n) const
			{
				return reverse_iterator(Iterator::operator+(n));
			}
			reverse_iterator& operator+=(difference_type n)
			{
				Iterator::operator+=(n);
				return *this;
			}
			reverse_iterator  operator- (difference_type n) const
			{
				return reverse_iterator(Iterator::operator+(n));
			}
			reverse_iterator& operator-=(difference_type n)
			{
				Iterator::operator-=(n);
				return *this;
			}

			reference operator[](difference_type n) const
			{
				return Iterator::operator[](-n);
			}
		};
		template <class Iterator1, class Iterator2>
        bool operator==(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y)
		{
			return x.base() == y.base();
		}
		template <class Iterator1, class Iterator2>
        bool operator<(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y)
		{
			return !(x.base() < y.base());
		}
		template <class Iterator1, class Iterator2>
        bool operator!=(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y)
		{
			return !(x == y);
		}
		template <class Iterator1, class Iterator2>
		bool operator<=(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y)
		{
			return x < y || x==y;
		}
		template <class Iterator1, class Iterator2>
        bool operator>(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y)
		{
			return !(x <= y);
		}
		template <class Iterator1, class Iterator2>
        bool operator>=(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y)
		{
			return !(x < y);
		}
    
 
    template <class Iterator1, class Iterator2>
    auto operator-(const reverse_iterator<Iterator1>& x,const reverse_iterator<Iterator2>& y) ->decltype(y.base() - x.base())
	{
		return y.base() - x.base();
	}
    template <class Iterator>
    reverse_iterator<Iterator> operator+(typename reverse_iterator<Iterator>::difference_type n,const reverse_iterator<Iterator>& x)
	{
		return reverse_iterator<Iterator>(n + x.base());
	}
    template <class Iterator>
    reverse_iterator<Iterator> make_reverse_iterator(Iterator i)
	{
			return reverse_iterator<Iterator>(i);
	}
 
   /* template <class Container> class back_insert_iterator;
    template <class Container>
        back_insert_iterator<Container> back_inserter(Container& x);
 
    template <class Container> class front_insert_iterator;
    template <class Container>
        front_insert_iterator<Container> front_inserter(Container& x);
 
    template <class Container> class insert_iterator;
    template <class Container>
        insert_iterator<Container> inserter(Container& x, typename Container::iterator i);
 
    template <class Iterator> class move_iterator;
    template <class Iterator1, class Iterator2>
        bool operator==(
            const move_iterator<Iterator1>& x, const move_iterator<Iterator2>& y);
    template <class Iterator1, class Iterator2>
        bool operator!=(
            const move_iterator<Iterator1>& x, const move_iterator<Iterator2>& y);
    template <class Iterator1, class Iterator2>
        bool operator<(
            const move_iterator<Iterator1>& x, const move_iterator<Iterator2>& y);
    template <class Iterator1, class Iterator2>
        bool operator<=(
            const move_iterator<Iterator1>& x, const move_iterator<Iterator2>& y);
    template <class Iterator1, class Iterator2>
        bool operator>(
            const move_iterator<Iterator1>& x, const move_iterator<Iterator2>& y);
    template <class Iterator1, class Iterator2>
        bool operator>=(
            const move_iterator<Iterator1>& x, const move_iterator<Iterator2>& y);
 
    template <class Iterator1, class Iterator2>
        auto operator-(
            const move_iterator<Iterator1>& x,
            const move_iterator<Iterator2>& y) -> decltype(x.base() - y.base());
    template <class Iterator>
        move_iterator<Iterator> operator+(
            typename move_iterator<Iterator>::difference_type n, 
            const move_iterator<Iterator>& x);
    template <class Iterator>
        move_iterator<Iterator> make_move_iterator(Iterator i);*/
 
    // stream iterators:
	//����ɾ��,����д
 
    // range access:
	template< class C >
	constexpr auto begin(C& c) -> decltype(c.begin())
	{
		return c.begin();
	}
	template< class C >
	constexpr auto begin(const C& c) -> decltype(c.begin())
	{
		return c.begin();
	}
	template< class T, std::size_t N >
	constexpr T* begin(T(&array)[N]) noexcept
	{
		return static_cast<T*>(array);
	}
	template< class C >
	constexpr auto cbegin(const C& c) noexcept(noexcept(begin(c)))
		-> decltype(begin(c))
	{
		return begin(c);
	}

	template< class C >
	constexpr auto end(C& c) -> decltype(c.end())
	{
		return c.end();
	}
	template< class C >
	constexpr auto end(const C& c) -> decltype(c.end())
	{
		return c.end();
	}
	template< class T, std::size_t N >
	constexpr T* end(T(&array)[N]) noexcept
	{
		return begin(array) + N;
	}
	template< class C >
	constexpr auto cend(const C& c) noexcept(noexcept(end(c)))
		-> decltype(end(c))
	{
		return end(c);
	}

}