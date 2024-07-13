#ifndef SINGLETON_H
#define SINGLETON_H
#include <iostream>

namespace simia
{
template <typename T> class Singleton
{
  public:
    Singleton(Singleton &&) = delete;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
    Singleton &operator=(Singleton &&) = delete;

    static T& instance()
    {

        static T _instance{};
        return _instance;
    };

  protected:
    Singleton() = default;
    ~Singleton() = default;
};

} // namespace simia

#endif