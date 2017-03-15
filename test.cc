#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <list>
#include <vector>
#include <memory>
#include <unistd.h>
#include <fstream>
#include "sqlitedb.h"

template<typename T>
class SynQueue
{
    public:
        SynQueue(int MaxSize):
            m_maxsize(MaxSize) { }

        void Put(const T&x)
        {
            std::lock_guard<std::mutex>locker(m_mutex);
            while(isFull())
            {
                m_notFull.wait(m_mutex);
            }
            m_queue.push_back(x);
            m_notEmpty.notify_one();
        }

        void Take(T&x)
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            while(isEmpty())
            {
                std::cout << "no resource... please wait" << std::endl;
                m_notEmpty.wait(m_mutex);
            }
            x = m_queue.front();
            m_queue.pop_front();
            std::cout << "list size:" << m_queue.size() << std::endl;
            m_notFull.notify_one();
        }

        bool Empty()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.empty();
        }

        bool Full()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.size() == m_maxsize;
        }

        size_t Size()
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.size();
        }

    private:
        bool isFull() const
        {
            return m_queue.size() == m_maxsize;
        }
        bool isEmpty() const
        {
            return m_queue.empty();
        }

    private:
        std::list<T>m_queue;
        std::mutex m_mutex;
        std::condition_variable_any m_notEmpty;
        std::condition_variable_any m_notFull;
        int m_maxsize;
};

void func(SynQueue<int> *sq)
{
    int ret;
    char *db = (char*)"../sqlitedb/test.db";
    std::string sql = "select * from student;";


    dbhelper cDb(db);
    

    while (1){
        sleep(2);
        sq->Take(ret);
    std::cout << ret << std::endl;
    char *zSQL = sqlite3_mprintf("insert into student values('%s','%d')","jianqi001",ret);

    cDb.insert(zSQL);
    }
    
}


void collection(SynQueue<int> *sq){
    for(int i = 0; i < 10; i++)
    {
        sq->Put(i);
    }
    std::ofstream examplefile ("/home/jianqi/example.txt");  
    if (examplefile.is_open()) {  
        examplefile << "This is a line.\n";  
        examplefile << "This is another line.\n";  
        examplefile.close();  
    }  
}

int main(int argc, char *argv[])
{
    SynQueue<int>syn(20);
    std::vector<std::shared_ptr<std::thread>> tvec;
    tvec.push_back(std::make_shared<std::thread>(collection, &syn));
    tvec.push_back(std::make_shared<std::thread>(func, &syn));
    tvec[0]->detach();
    tvec[1]->detach();
    sleep(10);
    int j = 1;
    while(j!=12){
        syn.Put(j+10);
        sleep(1);
        j++;
    } 
    std::ofstream examplefile ("/home/jianqi/example.txt");  
    if (examplefile.is_open()) {  
        examplefile << "This is a line.\n";  
        examplefile << "This is another line.\n";  
        examplefile.close();  
    }  

    return EXIT_SUCCESS;
}
