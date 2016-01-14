#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <LockfreeList.h>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;

boost::atomic_int producer_count(0);
boost::atomic_int consumer_count(0);

boost::atomic_int producer_value(0);
boost::atomic_int consumer_value(0);

const int iterations = 10000;
const int producer_thread_count = 10;
const int consumer_thread_count = 10;

list_t *lock_free_list;
vector<lock_t*> nodes(100001);

void producer(void)
{
    for(int i=0; i<iterations; ++i){
        int input = producer_value++;
        
        lock_t *new_lock;
        new_lock = (lock_t *)malloc(sizeof(lock_t));
        new_lock->next = NULL;
        new_lock->record_id = input;        

        list_append(lock_free_list, new_lock);
        
        nodes[input] = new_lock;
    }
}

boost::atomic<bool> done(false);

void consumer(void)
{
    while(!done);

    while(consumer_value < 100000){   
        lock_t* remove_node = nodes[consumer_value++];
        list_remove(lock_free_list, remove_node);
    }
}


int main()
{
    lock_free_list = (list_t *)malloc(sizeof(list_t));
    init_list_t(lock_free_list);

    boost::thread_group producer_threads, consumer_threads;

    for(int i=0; i != producer_thread_count; ++i){
        producer_threads.create_thread(producer);
    }

    for(int i=0; i != consumer_thread_count; ++i){
       consumer_threads.create_thread(consumer);
    }

    producer_threads.join_all();
    done = true;
    cout << "producer finished " << "consumer start!" << endl;
    consumer_threads.join_all();
    cout << "produced " << producer_count << " object" << endl;

    cout << "consumed " << consumer_count << " object" << endl;
    printf("finish");
    print_list(lock_free_list);
    return 0;
}
