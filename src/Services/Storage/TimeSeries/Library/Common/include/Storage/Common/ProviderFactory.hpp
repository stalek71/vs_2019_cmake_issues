#pragma once
#include <queue>
#include <memory>
#include <semaphore>
#include <chrono>
#include <functional>

namespace mesh::storage::timeseries
{
    template <class ProviderT, int poolSize = 10>
    class ProviderFactory
    {
        using ProvConstrFn = std::function<std::unique_ptr<ProviderT>()>;

    public:
        ProviderFactory(ProvConstrFn providerConstructor);

        std::unique_ptr<ProviderT> Get();
        void Release(std::unique_ptr<ProviderT> &provider);
        void Release();

    private:
        std::queue<std::unique_ptr<ProviderT>> providers_;
        std::counting_semaphore<poolSize> sem_; // Controls the access the to the available providers pool
        std::mutex mut_;                        // Exclusive lock for vector manipulation
        ProvConstrFn providerConstructor_;
    };

    template <class ProviderT, int poolSize>
    ProviderFactory<ProviderT, poolSize>::ProviderFactory(ProvConstrFn providerConstructor) : sem_{poolSize}, providerConstructor_(providerConstructor)
    {
    }

    template <class ProviderT, int poolSize>
    std::unique_ptr<ProviderT> ProviderFactory<ProviderT, poolSize>::Get()
    {
        using namespace std::literals::chrono_literals;

        // Try to obtain the access to the pool of providers
        if (!sem_.try_acquire_for(10s)) // Timeout value should come from env var for docker/kubernetes
            throw std::runtime_error("DBProvider not available - timeout");

        // Try to obtain a provider from the pool of EXISTING providers
        std::scoped_lock lock(mut_);
        if (providers_.size())
        {
            auto elem = std::move(providers_.front());
            providers_.pop();
            return elem;
        }

        // There is no any provider available it the pool - lets construct a one
        return providerConstructor_();
    }

    template <class ProviderT, int poolSize>
    void ProviderFactory<ProviderT, poolSize>::Release(std::unique_ptr<ProviderT> &provider)
    {
        std::scoped_lock lock(mut_);

        // Store the provider
        providers_.push(std::move(provider));

        // Release the lock signaling the new resource is available
        sem_.release();
    }

    template <class ProviderT, int poolSize>
    void ProviderFactory<ProviderT, poolSize>::Release()
    {
        // Release the lock signaling the new resource is available
        sem_.release();
    }
}