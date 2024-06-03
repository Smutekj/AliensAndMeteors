#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <cassert>

template <class ResourceType, class Identifier>
class ResourceHolder
{
public:
	
	void load(Identifier id, const std::string &filename);
	
	template <class ...Args>
	void load(Identifier id, const Args... names);
	
	ResourceType &get(Identifier id);

private:
	void insertResource(Identifier id, std::unique_ptr<ResourceType> resource);

private:
	std::unordered_map<Identifier, std::unique_ptr<ResourceType>> m_id2resource;
};

template <typename ResourceType, typename Identifier>
template <class ...Args>
void ResourceHolder<ResourceType, Identifier>::load(Identifier id, const Args... names)
{
	std::unique_ptr<ResourceType> resource(new ResourceType());
	if (!resource->loadFromFile(names...))
	{
		throw std::runtime_error("ResourceHolder::load - Failed to load!");
	}
	insertResource(id, std::move(resource));
}

template <typename ResourceType, typename Identifier>
void ResourceHolder<ResourceType, Identifier>::load(Identifier id, const std::string &filename)
{
	std::unique_ptr<ResourceType> resource(new ResourceType());
	if (!resource->loadFromFile(filename))
	{
		throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);
	}
	insertResource(id, std::move(resource));
}


template <typename ResourceType, typename Identifier>
ResourceType &ResourceHolder<ResourceType, Identifier>::get(Identifier id)
{
	assert(m_id2resource.count(id) > 0);
	return *m_id2resource.at(id);
}

template <typename Resource, typename Identifier>
void ResourceHolder<Resource, Identifier>::insertResource(Identifier id, std::unique_ptr<Resource> resource)
{
	// Insert and check success
	auto inserted = m_id2resource.insert(std::make_pair(id, std::move(resource)));
	assert(inserted.second);
}