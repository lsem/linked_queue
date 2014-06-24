#include "global.h"
#include "storage.h"


#define CHECKED(Condition) do { if (!(Condition)) { fprintf(stderr, "checked operation failed: %s (%s: %d)\n", #Condition, __FILE__, __LINE__); abort(); } } while (false)


bool linqqueue_test()
{
	shared_ptr<storage_t> storage = std::make_shared<storage_t>();

	size_t index_a, index_b, index_c;
	//////////////////////////////////////////////////////////////////////////
	// test 0.
	//////////////////////////////////////////////////////////////////////////	
	{
		CHECKED(storage->allocate_record(&index_a) == true);
		CHECKED(storage->allocate_record(&index_b) == true);
		CHECKED(storage->allocate_record(&index_c) == true);

		CHECKED(storage->storage_size() == 3);
		CHECKED(storage->queue_size() == 0);

		storage->change_record_status(index_a, s_success, 10);
		storage->change_record_status(index_b, s_fauilue, 20);

		CHECKED(storage->storage_size() == 3);
		CHECKED(storage->queue_size() == 2);
	}

	//////////////////////////////////////////////////////////////////////////
	// test 1.
	//////////////////////////////////////////////////////////////////////////
	queue_item_t *queue_to_process;
	size_t queue_to_process_size;
	{		
		storage->grab_queue(&queue_to_process, &queue_to_process_size);

		CHECKED(queue_to_process_size == 2);
		CHECKED(storage->queue_size() == 0);
		CHECKED(storage->storage_size() == 3);

		CHECKED(queue_to_process[0]._status == s_success && queue_to_process[0]._payload == 10);
		CHECKED(queue_to_process[1]._status == s_fauilue && queue_to_process[1]._payload == 20);

		delete[] queue_to_process;
	}


	//////////////////////////////////////////////////////////////////////////
	// test 2.
	//////////////////////////////////////////////////////////////////////////
	{
		storage->grab_queue(&queue_to_process, &queue_to_process_size);

		CHECKED(queue_to_process_size == 0);
		CHECKED(storage->storage_size() == 3);
	}

	//////////////////////////////////////////////////////////////////////////
	// test 3.
	//////////////////////////////////////////////////////////////////////////
	{
		storage->change_record_status(index_a, s_success, 10);
		storage->change_record_status(index_c, s_fauilue, 20);

		CHECKED(storage->queue_size() == 2);
		CHECKED(storage->storage_size() == 3);

		storage->grab_queue(&queue_to_process, &queue_to_process_size);

		CHECKED(queue_to_process_size == 2);
		CHECKED(storage->queue_size() == 0);

		delete[] queue_to_process;
	}

	{
		storage->change_record_status(index_a, s_success, 10);
		storage->change_record_status(index_c, s_fauilue, 20);

		CHECKED(storage->queue_size() == 2);
		CHECKED(storage->storage_size() == 3);


		// first, remove enqueued record
		storage->remove_record(index_b);

		CHECKED(storage->storage_size() == 2);
		CHECKED(storage->queue_size() == 2);

		storage->grab_queue(&queue_to_process, &queue_to_process_size);

		CHECKED(storage->queue_size() == 0);
		CHECKED(storage->storage_size() == 2);
		CHECKED(queue_to_process_size == 2);

		delete[] queue_to_process;

		// now, remove queued record (and first one is removed case)
		CHECKED(true == storage->remove_record(index_a));
		CHECKED(storage->storage_size() == 1);

		storage->grab_queue(&queue_to_process, &queue_to_process_size);
		CHECKED(storage->queue_size() == 0);
		CHECKED(storage->storage_size() == 1);
		delete[] queue_to_process;

		CHECKED(false == storage->remove_record(index_b));
		CHECKED(storage->queue_size() == 0);
		CHECKED(storage->storage_size() == 1);		

		CHECKED(true == storage->remove_record(index_c));
		CHECKED(storage->storage_size() == 0);
		CHECKED(storage->queue_size() == 0);

		// grab is idempotent operation
		storage->grab_queue(&queue_to_process, &queue_to_process_size);
		CHECKED(storage->queue_size() == 0);
		CHECKED(storage->storage_size() == 0);

		delete[] queue_to_process;
	}
	
	return true;
}


int main(int argc, char *argv[])
{
	if (linqqueue_test())
	{
		int break_point = 0;
	}
}

