//
// Created by jay on 07/11/25.
//

#ifndef SERVICECONTAINER_H
#define SERVICECONTAINER_H

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cxxabi.h>
#include <functional>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>


template<class Service, class Deleter, class... Deps>
using ServiceFactoryFunction = std::unique_ptr<Service, Deleter> (*)(Deps *...);

template<class Service, class... Deps>
std::unique_ptr<Service> GenericServiceFactory(Deps *...deps) {
    return std::make_unique<Service>(deps...);
}

template<class Type>
std::string GetTypeNameGcc() {
    int status;
    const char *typeName = typeid(Type).name();

    auto realName = abi::__cxa_demangle(typeName, nullptr, nullptr, &status);

    if (status != 0) {
        assert(false && "failed to demangle name");
        return {typeName};
    }

    return {realName};
}

template<class Service>
concept ParameterlessConstructor = requires {
    { std::unique_ptr<Service>(new Service()) } -> std::same_as<std::unique_ptr<Service>>;
};

class AbstractPointerContainer {
public:
    virtual ~AbstractPointerContainer() = default;

    [[nodiscard]] void *Get() const { return m_rawPointer; }

    AbstractPointerContainer(AbstractPointerContainer &&other) noexcept {
        m_rawPointer = std::exchange(other.m_rawPointer, nullptr);
    }

    AbstractPointerContainer &operator=(AbstractPointerContainer &&other) noexcept {
        m_rawPointer = std::exchange(other.m_rawPointer, nullptr);
        return *this;
    }

protected:
    explicit AbstractPointerContainer(void *rawPointer) : m_rawPointer(rawPointer) {}

private:
    void *m_rawPointer{};
};

template<class T, class Deleter>
class UniquePointerContainer : public AbstractPointerContainer {
public:
    ~UniquePointerContainer() override = default;

    explicit UniquePointerContainer(std::unique_ptr<T, Deleter> &&uniquePointer) :
        AbstractPointerContainer(uniquePointer.get()), m_uniquePointer(std::move(uniquePointer)) {}

private:
    std::unique_ptr<T, Deleter> m_uniquePointer;
};

using ServiceInstanceMap = std::unordered_map<std::type_index, AbstractPointerContainer *>;
using ServiceUniquePtrList = std::list<std::unique_ptr<AbstractPointerContainer>>;

class ServiceProvider {
public:
    ServiceProvider(ServiceUniquePtrList &&serviceOwningPointers, ServiceInstanceMap &&serviceInstanceMap) :
        m_serviceOwningPointers(std::move(serviceOwningPointers)), m_services(std::move(serviceInstanceMap)) {}

    template<typename Service>
    Service *GetService() {
        const auto it = m_services.find(std::type_index{typeid(Service)}); // NOLINT(*-identifier-length)
        if (it == m_services.end()) {
            throw std::runtime_error("Failed to find service in container: " + std::string(typeid(Service).name()));
        }
        return static_cast<Service *>(it->second->Get());
    }


private:
    template<typename Service, class Dependant>
    Service *GetServiceInternal() {
        const auto it = m_services.find(std::type_index{typeid(Service)});
        if (it == m_services.end()) {
            throw std::runtime_error(std::string(typeid(Service).name()) + ": unsatisfied dependency of " +
                                     std::string(typeid(Dependant).name()));
        }
        return static_cast<Service *>(it->second->Get());
    }

    /*
     * The pointers should be owned by a list since the order they inhabit the map in is not well-defined, which
     * can introduce destruction order issues.
     */
    ServiceUniquePtrList m_serviceOwningPointers;
    ServiceInstanceMap m_services;
};


class ServiceCollection {
public:
    template<class Service, class Deleter, class... Deps>
    void RegisterService(const ServiceFactoryFunction<Service, Deleter, Deps...> factory) {
        auto typeId = std::type_index{typeid(Service)};

        auto creationFunction = [factory](const ServiceInstanceMap &map) {
            std::unique_ptr<Service, Deleter> uniquePtr = factory(ResolveDependencyFromMap<Deps, Service>(map)...);
            if (uniquePtr == nullptr) {
                throw std::runtime_error{"Failed to construct service " + GetTypeNameGcc<Service>() +
                                         ": factory method returned nullptr"};
            }

            auto container = std::make_unique<UniquePointerContainer<Service, Deleter>>(std::move(uniquePtr));
            return container;
        };

        ([&] { m_serviceDependants.insert(std::make_pair(std::type_index{typeid(Deps)}, typeId)); }(), ...);

        GraphNode node{
                .creationFunction = creationFunction,
                .typeIndex = typeId,
                .typeName = GetTypeNameGcc<Service>(),
                .dependencies = {std::type_index{typeid(Deps)}...},
        };

        auto [it, inserted] = m_nodes.insert(std::make_pair(typeId, node));

        if (!inserted) {
            // We could continue here with no problems, but this was almost certainly unintentional. Stop and let the
            // caller know.
            throw std::logic_error{"Service " + std::string(typeid(Service).name()) +
                                   " has already been added to the container"};
        }

        if (sizeof...(Deps) == 0) {
            m_nodesWithNoDependencies.push_back(it);
        }
    }

    template<ParameterlessConstructor Service>
    void RegisterService() {
        RegisterService(GenericServiceFactory<Service>);
    }

    ServiceProvider BuildServiceProvider() {
        // Perform a topological sort of the graph nodes (Kahn's algorithm)
        std::vector<GraphNode> sortedNodes;
        sortedNodes.reserve(m_nodes.size());

        while (!m_nodesWithNoDependencies.empty()) {
            const auto nodeIt = m_nodesWithNoDependencies.back();
            m_nodesWithNoDependencies.pop_back();

            const GraphNode &node = sortedNodes.emplace_back(std::move(nodeIt->second));

            auto [begin, end] = m_serviceDependants.equal_range(node.typeIndex);
            for (auto dependantIt = begin; dependantIt != end; ++dependantIt) {
                auto dependantNodeIt = m_nodes.find(dependantIt->second);
                assert(dependantNodeIt != m_nodes.end());

                GraphNode &dependantNode = dependantNodeIt->second;
                const std::size_t erased = dependantNode.dependencies.erase(node.typeIndex);
                assert(erased != 0);

                if (dependantNode.dependencies.empty()) {
                    m_nodesWithNoDependencies.push_back(dependantNodeIt);
                }
            }
        }

        for (const auto &[typeId, node]: m_nodes) {
            if (!node.dependencies.empty()) {
                throw std::runtime_error{
                        "Failed to build service provider: topological sort error. Check "
                        "the dependencies of the service '" +
                        node.typeName +
                        "'. Ensure there are no circular dependencies that and all dependencies have been registered."};
            }
        }

        ServiceUniquePtrList ptrList;
        ServiceInstanceMap instanceMap;

        for (const GraphNode &node: sortedNodes) {
            std::unique_ptr<AbstractPointerContainer> container = node.creationFunction(instanceMap);
            instanceMap.insert(std::make_pair(node.typeIndex, container.get()));

            /*
             * When we topologically sort the service graph, we get a list of services in the order they should be
             * constructed. Ideally services should then be destroyed in the reverse of this order, so that they can
             * access all of their dependencies in their destructors.
             *
             * Adding to the front of the list for each element of the sorted node list achieves this ordering.
             */
            ptrList.push_front(std::move(container));
        }

        return ServiceProvider{std::move(ptrList), std::move(instanceMap)};
    }

private:
    using ServiceCreationFunction =
            std::function<std::unique_ptr<AbstractPointerContainer>(const ServiceInstanceMap &)>;

    template<class Service, class Dependant>
    static Service *ResolveDependencyFromMap(const ServiceInstanceMap &map) {
        const auto it = map.find(std::type_index{typeid(Service)});
        if (it == map.end()) {
            throw std::runtime_error(GetTypeNameGcc<Service>() + ": unsatisfied dependency of " +
                                     GetTypeNameGcc<Dependant>());
        }
        return static_cast<Service *>(it->second->Get());
    }

    struct GraphNode {
        ServiceCreationFunction creationFunction;
        std::type_index typeIndex;
        std::string typeName;
        std::set<std::type_index> dependencies; ///< 'Incoming edges', in graph speak
    };

    std::unordered_map<std::type_index, GraphNode> m_nodes; ///< Lookup for graph nodes
    std::unordered_multimap<std::type_index, std::type_index> m_serviceDependants; ///< Lookup for 'outgoing edges'
    std::vector<decltype(m_nodes)::iterator> m_nodesWithNoDependencies; ///< Starting set of nodes with no dependencies
};


#endif // SERVICECONTAINER_H
