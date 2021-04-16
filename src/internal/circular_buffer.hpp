template<class T, size_t TCount>
class CircularBuffer
{
  public:
	CircularBuffer() = default;

	void put(T item)
	{
		buf_[head_] = item;

		if(full_)
		{
			tail_ = (tail_ + 1) % max_size_;
		}

		head_ = (head_ + 1) % max_size_;

		full_ = head_ == tail_;
	}

	T get()
	{
		if(empty())
		{
			return T();
		}

		// Read data and advance the tail (we now have a free space)
		auto val = buf_[tail_];
		full_ = false;
		tail_ = (tail_ + 1) % max_size_;

		return val;
	}

	void reset()
	{
		head_ = tail_;
		full_ = false;
	}

	bool empty() const
	{
		// if head and tail are equal, we are empty
		return (!full_ && (head_ == tail_));
	}

	bool full() const
	{
		// If tail is ahead the head by 1, we are full
		return full_;
	}

	size_t capacity() const
	{
		return max_size_;
	}

	size_t size() const
	{
		size_t size = max_size_;

		if(!full_)
		{
			if(head_ >= tail_)
			{
				size = head_ - tail_;
			}
			else
			{
				size = max_size_ + head_ - tail_;
			}
		}

		return size;
	}

	size_t head()
	{
		return head_;
	}

	size_t tail()
	{
		return tail_;
	}

	const T* storage()
	{
		return &buf_[0];
	}

  private:
	size_t head_ = 0;
	size_t tail_ = 0;
	const size_t max_size_ = TCount;
	bool full_ = 0;
	T buf_[TCount];
};
