#ifndef TASK_SCHEDULER2_HPP
#define TASK_SCHEDULER2_HPP

#include <functional>
#include <future>
#include <map>
#include <type_traits>
#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <list>

enum class execution_status : unsigned int{
    execution_pending,
    execution_underway,
    execution_complete
};


template<typename R, typename ... Args>
class scheduler{

private:

    struct task_node{
        std::function<R(Args...)> func;
        std::tuple<Args...> args;
        unsigned int secs;
    };

    struct task_result{
        execution_status task_status;
        std::future<R> result;
        std::chrono::system_clock::time_point completion_time;
    };

    using task_id_t = unsigned int;
    using task_object_t = std::pair<task_id_t, std::pair<task_node, task_result>>;


    unsigned int num_of_tasks;
    std::list<task_object_t> task_list;
    std::timed_mutex t_mtx;

    struct task_execution{

        template<typename Iter, typename Result = R>
        std::enable_if_t<!std::is_same_v<Result, void>, Result> operator() (Iter& iter, Args ... args){
            R res;
            try{
                res = iter.second.first.func(std::forward<Args>(args)...);
            }
            catch(R r){
                iter.second.second.completion_time = std::chrono::system_clock::now();
                iter.second.second.task_status = execution_status::execution_complete;
                //auto eptr = std::make_exception_ptr(r);
                throw r;
            }
            catch(...){
            }

            iter.second.second.completion_time = std::chrono::system_clock::now();
            iter.second.second.task_status = execution_status::execution_complete;
            return res;
        }

        template<typename Iter, typename Result = R>
        std::enable_if_t<std::is_same_v<Result, void>> operator() (Iter& iter, Args ... args){
            iter.second.first.func(std::forward<Args>(args)...);
            iter.second.second.completion_time = std::chrono::system_clock::now();
            iter.second.second.task_status = execution_status::execution_complete;
        }
    };


    template<class Arg>
    decltype(auto) wrap_ref(Arg&& arg){
        if constexpr (std::is_reference_v<Arg>)
            return std::ref(arg);
        else
            return std::forward<Arg>(arg);
    }

private:

    friend bool operator == (const task_object_t& lhs, const task_object_t& rhs){
        if(lhs.first == rhs.first)
            return true;
        return false;
    }

    friend bool operator == (const task_object_t& task, const task_id_t& id){
        if(task.first == id)
            return true;
        return false;
    }

    template<typename T>
    auto find_task(const T& val){
        return std::find(task_list.begin(), task_list.end(), val);
    }

    /*template<typename F, std::size_t ... Is>
    std::future<R> execute_task(F&& func, std::tuple<Args...> args, std::index_sequence<Is...> seq){
        return std::async(std::move(func), std::move(std::get<Is>(args))...);
    }*/

    template<typename Iter, std::size_t ... Is>
    std::future<R> execute_task(Iter& iter, std::tuple<Args...> args, std::index_sequence<Is...> seq){
        //task_execution te{iter.second.second.completion_time, iter.second.second.task_status};
        task_execution te{};
        return std::async(&task_execution::template operator()<Iter>, te, std::ref(iter), wrap_ref<Args>(std::forward<Args>(std::get<Is>(args)))...);
        //return std::async(std::move(func), std::move(std::get<Is>(args))...);
    }

    template<typename Iter>
    void invoke_task(Iter& iter){
        //iter.second.second.result = execute_task(iter, std::move(iter.second.first.args), std::index_sequence_for<Args...>());
        iter.second.second.result = execute_task(iter, std::move(iter.second.first.args), std::index_sequence_for<Args...>());
        iter.second.second.task_status = execution_status::execution_underway;
    }

    void clean_tasks(){
        using iter_type = typename std::list<task_object_t>::iterator;
        std::list<iter_type> remove_list;
        auto curr_time = std::chrono::system_clock::now();
        std::list<task_node> tn;
        for(auto& task : task_list){
            if(task.second.second.task_status == execution_status::execution_complete && (std::chrono::duration_cast<std::chrono::seconds>(curr_time - task.second.second.completion_time).count() > 60))
                remove_list.push_back(find_task(task));
        }
        for(auto& task : remove_list)
            task_list.erase(task);
    }


public:

    scheduler() : num_of_tasks{0}, task_list{} {}

    template<typename Function>
    int submit(Function func, Args ... args, unsigned int seconds){
        task_node node{std::forward<Function>(func), std::forward<std::tuple<Args...>>(std::tuple<Args...>(std::forward<Args>(args)...)), seconds};
        std::pair<task_node, task_result> p {node, task_result{{execution_status::execution_pending}, std::future<R>()}};
        std::lock_guard lock{t_mtx};
        task_list.push_back(std::move(std::pair{num_of_tasks++, std::move(p)}));
        return num_of_tasks - 1;
    }

    void start(){
        auto start_time = std::chrono::system_clock::now();
        while(true){
            auto curr_time = std::chrono::system_clock::now();
            std::lock_guard lock{t_mtx};
            for(auto& task : task_list){
                if((std::chrono::duration_cast<std::chrono::seconds>(curr_time - start_time)).count() >= task.second.first.secs){
                    if(task.second.second.task_status == execution_status::execution_pending){
                        invoke_task(task);
                    }
                }
            }
            clean_tasks();
        }
    }


    std::pair<R, execution_status> get_result_of(task_id_t id){
        std::lock_guard lock{t_mtx};
        auto iter = find_task(id);
        if(iter != task_list.end()){
            if(iter->second.second.task_status == execution_status::execution_underway){
                return {R(), execution_status::execution_underway};
            }
            else if(iter->second.second.task_status == execution_status::execution_pending){
                return {R(), execution_status::execution_pending};
            }
            else if(iter->second.second.task_status == execution_status::execution_complete){
                try{
                    return {iter->second.second.result.get(), execution_status::execution_complete};
                }
                catch(R r){
                    //auto eptr = std::make_exception_ptr(ex);
                    throw r;
                }
            }
        }
        throw std::invalid_argument("invalid task id");
    }



    execution_status status(task_id_t id){
        std::lock_guard lock{t_mtx};
        auto iter = find_task(id);
        if(iter != task_list.end())
            return iter->second.second.task_status;
        throw std::invalid_argument("invalid task id");
    }
};


#endif // TASK_SCHEDULER2_HPP
