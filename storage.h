#ifndef __STORAGE_H_INCLUDED
#define __STORAGE_H_INCLUDED


typedef int payload_t;
#define NULL_PAYLOAD (~ (payload_t) 0)

enum estatus_t
{
	s__min,
	
	s_none = s__min,
	s_success,
	s_fauilue,

	s__max,
	
	s_invalid = s__max,
};

struct storage_record_t
{
	storage_record_t() {}
	storage_record_t(void * /*nullptr*/): _status(s_invalid), _next_record(nullptr) {}

	void assign(estatus_t status, payload_t payload)
	{
		_status = status;
		_payload = payload;
	}	

	void assign(estatus_t status, payload_t payload, storage_record_t *next_record)
	{
		assign(status, payload);
		_next_record = next_record;
	}

	estatus_t				_status;
	payload_t				_payload;
	storage_record_t		*_next_record;
};

struct queue_item_t
{
	queue_item_t() {}
	queue_item_t(estatus_t status, payload_t payload): _status(_status), _payload(payload) {}

	void assign(estatus_t status, payload_t payload)
	{
		_status = status;
		_payload = payload;
	}

	estatus_t				_status;
	payload_t				_payload;
};


class storage_t
{
public:
	storage_t():
		_storage_top(0),
		_records(),
		_records_lock(),
		_p_queue_first(nullptr),
		_pp_queue_last(&_p_queue_first),
		_queue_size(0),
		_storage_size(0)
	{}

private:
	// todo: allocate via index_pool_t
	bool allocate_index(size_t *out_index) { return *out_index = _storage_top++, true; }
	bool deallocate_index(size_t index) { assert(false); return false; }

public:
	/************************************************************************/
	/* Inserts element into storage                                         */
	/************************************************************************/
	bool allocate_record(size_t *out_index)
	{	
		bool result;
		
		_records_lock.lock();
		
		size_t new_index;
		if (allocate_index(&new_index))
		{
			ASSERT(IN_RAGNE(new_index, 0, STORAGE_SIZE));
			_records[new_index].assign(s_none, NULL_PAYLOAD, nullptr);
			*out_index = new_index;
			++_storage_size;

			result = true;
		}
		else
		{
			result = false;
		}
		
		_records_lock.unlock();

		return result;
	}

	bool unsafe_locate_record(size_t index, storage_record_t **record)
	{
		bool result;

		if (index < _storage_top && _records[index]._status != s_invalid)
		{
			*record = &_records[index];
			result = true;
		}
		else
		{
			result = false;
		}

		return result;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool change_record_status(size_t index, estatus_t status, payload_t payload)
	{
		bool result;

		_records_lock.lock();

		storage_record_t *this_record;
		if (unsafe_locate_record(index, &this_record))
		{
			ASSERT(this_record != NULL);
			
			if (this_record->_status != s_none)
			{
				// looks like it was already placed into queue, verify it is chained 				
				this_record->assign(status, payload); // update while keeping it in queue 
			}
			else
			{
				// link the record to the end of the queue
				ASSERT(_pp_queue_last != NULL);
				this_record->assign(status, payload);

				*_pp_queue_last = this_record;
				_pp_queue_last = &this_record->_next_record;
				
				++_queue_size;
			}

			result = true;
		}
		else
		{
			result = false;
		}

		_records_lock.unlock();

		return result;
	}

	/************************************************************************/
	/* Makes a snapshot of current queue                                    */
	/************************************************************************/
	void grab_queue(queue_item_t **out_queuedata, size_t *out_queuesize)
	{
		assert(out_queuedata != nullptr);
		assert(out_queuesize != nullptr);

		_records_lock.lock();

		if (_queue_size > 0)
		{
			*out_queuedata = new queue_item_t[_queue_size];
			*out_queuesize = _queue_size;

			queue_item_t *current_queue_item = *out_queuedata;

			for (storage_record_t *p_record = _p_queue_first; p_record != *_pp_queue_last; )
			{
				current_queue_item->assign(p_record->_status, p_record->_payload);
				p_record->_status = s_none;
				
				auto *temp = p_record;
				p_record = p_record->_next_record;
				temp->_next_record = nullptr;

				++current_queue_item;
			}
			ASSERT( ((ptrdiff_t)(current_queue_item - *out_queuedata)) == _queue_size );
			_p_queue_first = nullptr;
			_pp_queue_last = &_p_queue_first;
			_queue_size = 0;
		}
		else
		{
			*out_queuesize = 0;
			*out_queuedata = nullptr;
		}

		_records_lock.unlock();
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool remove_record(size_t index)
	{
		bool result;

		_records_lock.lock();

		storage_record_t *this_record;
		if (unsafe_locate_record(index, &this_record))
		{
			ASSERT(this_record != NULL);

			if (this_record->_status != s_none)
			{
				unsafe_remove_queued_record(this_record);
			}
			else
			{
				// is not queued, 
				unsafe_remove_unqueed_record(this_record);
			}
			
			result = true;
		}
		else
		{
			result = false;
		}

		_records_lock.unlock();

		return result;
	}
	
	void unsafe_remove_unqueed_record(storage_record_t *record_to_remove)
	{
		ASSERT(record_to_remove->_status == s_none);
		ASSERT(record_to_remove->_next_record == nullptr);
		
		#pragma message("warning: not implemented yet")
		
		record_to_remove->assign(s_invalid, NULL_PAYLOAD, nullptr);
		// ...
		--_storage_size;
	}		

	void unsafe_remove_queued_record(storage_record_t *record_to_remove)
	{
		ASSERT(record_to_remove->_status != s_none);

		if (record_to_remove == _p_queue_first)
		{
			if (_pp_queue_last == &_p_queue_first->_next_record)
			{
				// last == first case 
				_p_queue_first = nullptr;
				_pp_queue_last = &_p_queue_first;
			}
			else
			{
				// first is removed, but it has at least two elements
				_p_queue_first = _p_queue_first->_next_record;
			}						
		}
		else
		{
			// locate element preceding record to remove
			storage_record_t *pcurrent = _p_queue_first;
			ASSERT(pcurrent->_next_record != nullptr); // must be at least to elements in list
			for (; pcurrent->_next_record != record_to_remove; pcurrent = pcurrent->_next_record)
			{				
				ASSERT(pcurrent->_next_record != nullptr); // debug check of queue consistency
			}
			ASSERT(pcurrent->_next_record == record_to_remove);

			// relink records
			if (_pp_queue_last == &record_to_remove->_next_record)
			{
				_pp_queue_last = &pcurrent->_next_record;
			}

			pcurrent->_next_record = record_to_remove->_next_record;
			
			--_queue_size;

			record_to_remove->_next_record = nullptr;
			record_to_remove->_status = s_none;
			unsafe_remove_unqueed_record(record_to_remove);
		}		
	}


	size_t storage_size() const { return _storage_size; }
	size_t queue_size() const { return _queue_size; }

private:
	size_t					_storage_top;
	storage_record_t		_records[STORAGE_SIZE];
	std::mutex				_records_lock;

	storage_record_t		*_p_queue_first;
	storage_record_t		**_pp_queue_last;
	size_t					_queue_size;
	size_t					_storage_size;
};

#endif // __STORAGE_H_INCLUDED
