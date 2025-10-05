#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <concepts>
#include <typeindex>
#include <filesystem>
#include <functional>
#include <unordered_map>

#include "Core/Logger.h"

namespace OtterEngine {

	// Aliasing std::filesystem for conciseness
	namespace fs = std::filesystem;

	// Establish a contract so that all Resource types must implement
	// a static LoadFromFile method and an IsValid member function
	template<typename T>
	concept Resource = requires(T t, const fs::path& path) {
		{ T::LoadFromFile(path) } -> std::convertible_to<std::shared_ptr<T>>;
		{ t.IsValid() } -> std::convertible_to<bool>;
	};

	/// <summary>
	/// Wrapper around a pointer to a resource and its path
	/// </summary>
	/// <typeparam name="T">The Resource the handle points to</typeparam>
	template<typename T>
	class ResourceHandle {
	private:
		std::shared_ptr<T> mRes;
		fs::path mPath;
	public:
		ResourceHandle() = default;
		ResourceHandle(std::shared_ptr<T> resource, const fs::path& path)
			: mRes(std::move(resource)), mPath(path) {
		}
		ResourceHandle(std::shared_ptr<T>&& resource, const fs::path& path)
			: mRes(std::move(resource)), mPath(path) {
		}

		/// <summary>
		/// Automatically returns the raw pointer to the managed Resource
		/// </summary>
		/// <returns>The raw pointer to the managed Resource</returns>
		T* operator->() const {
			assert(mRes && "Attempting to dereference null Resource Handle!");
			return mRes.get();
		}

		/// <summary>
		/// Automatically returns a reference to the managed Resource
		/// </summary>
		/// <returns>A reference to the managed Resource</returns>
		T& operator*() const {
			assert(mRes && "Attempting to dereference null Resource Handle!");
			return *mRes;
		}

		explicit operator bool() const { return mRes && mRes->IsValid(); }

		/// <summary>
		/// Returns the stored path as a string
		/// </summary>
		/// <returns>A constant reference to the stored path string</returns>
		const fs::path& GetPath() const { return mPath; }

		/// <summary>
		/// Returns the pointer to the managed resource
		/// </summary>
		/// <returns>The smart pointer referencing the managed resource</returns>
		std::shared_ptr<T> GetSharedPtr() const { return mRes; }
	};

	// Base interface for resource caches
	class IResourceCache {
	public:
		virtual ~IResourceCache() = default;
		virtual void Clear() = 0;
	};

	template<Resource T>
	class TypedResourceCache final : public IResourceCache {
	private:
		std::unordered_map<fs::path, std::weak_ptr<T>> mCache;

	public:
		std::shared_ptr<T> Get(const fs::path& path) {
			if (auto iter = mCache.find(path); iter != mCache.end()) {
				if (auto res = iter->second.lock()) {
					return res;
				}
			}
			return nullptr;
		}

		void Store(const fs::path& path, std::shared_ptr<T> res) {
			mCache[path] = std::move(res);
		}

		void Remove(const fs::path& path) {
			mCache.erase(path);
		}

		void Clear() override {
			mCache.clear();
		}
	};

	// Apply Type-Erasure pattern by hiding typed resource caches
	// behind IResourceCache interface via polimorphism
	class ResourceCache final {
	private:
		static inline std::unordered_map<std::type_index, std::unique_ptr<IResourceCache>> mInternalCaches;

		template<Resource T>
		static TypedResourceCache<T>& GetCache() {
			std::type_index typeIndex(typeid(T));

			if (auto iter = mInternalCaches.find(typeIndex); iter != mInternalCaches.end()) {
				return *static_cast<TypedResourceCache<T>*>(iter->second.get());
			}

			// Create new cache for this type
			auto newCache = std::make_unique<TypedResourceCache<T>>();
			auto& cacheRef = *newCache;
			mInternalCaches[typeIndex] = std::move(newCache);
			return cacheRef;
		}

	public:
		template<Resource T>
		static std::shared_ptr<T> Get(const fs::path& path) {
			return GetCache<T>().Get(path);
		}

		template<Resource T>
		static void Store(const fs::path& path, std::shared_ptr<T> res) {
			GetCache<T>().Store(path, std::move(res));
		}

		template<Resource T>
		static void Remove(const fs::path& path) {
			GetCache<T>().Remove(path);
		}

		static void Clear() {
			mInternalCaches.clear();
		}
	};

	class IResourceLoader {
	public:
		virtual ~IResourceLoader() = default;
	};

	template<Resource T>
	class TypedResourceLoader final : public IResourceLoader {
	private:
		std::function<std::shared_ptr<T>(const fs::path&)> mLoader;

	public:
		TypedResourceLoader(std::function<std::shared_ptr<T>(const fs::path&)> loader)
			: mLoader(std::move(loader)) {
		}

		std::shared_ptr<T> Load(const fs::path& path) const {
			return mLoader ? mLoader(path) : nullptr;
		}
	};

	class Resources final {
	private:
		// Default starting point
		static inline fs::path mResPath = "../Resources/";
		static inline std::unordered_map<std::type_index, std::unique_ptr<IResourceLoader>> mResLoaders;

		template<Resource T>
		static TypedResourceLoader<T>* GetLoader() {
			std::type_index typeIndex(typeid(T));
			if (auto iter = mResLoaders.find(typeIndex); iter != mResLoaders.end()) {
				return static_cast<TypedResourceLoader<T>*>(iter->second.get());
			}
			return nullptr;
		}

	public:
		static void SetResourcesPath(const fs::path& newPath) { mResPath = newPath; }

		template<Resource T>
		static ResourceHandle<T> Load(const fs::path& relativePath) {

			fs::path fullPath = mResPath / relativePath;

			// Check cache first
			if (auto cached = ResourceCache::Get<T>(fullPath)) {
				return ResourceHandle<T>(cached, relativePath);
			}

			// Load new resource
			auto* loader = GetLoader<T>();
			if (!loader) {
				OTTER_CORE_ERROR("[RESOURCES] No loader registered for type: {}", typeid(T).name());
				return ResourceHandle<T>();
			}

			auto resource = loader->Load(fullPath);
			if (!resource || !resource->IsValid()) {
				OTTER_CORE_ERROR("[RESOURCES] Failed to load: {}", fullPath);
				return ResourceHandle<T>();
			}

			// Cache new resource and return it
			ResourceCache::Store<T>(fullPath, resource);
			return ResourceHandle<T>(resource, relativePath);
		}

		template<Resource T>
		static void AddLoader() {
			std::type_index typeIndex(typeid(T));
			mResLoaders[typeIndex] = std::make_unique<TypedResourceLoader<T>>(
				[](const fs::path& path) -> std::shared_ptr<T> {
					return T::LoadFromFile(path);
				}
			);
		}

		template<Resource T>
		static void AddCustomLoader(std::function<std::shared_ptr<T>(const fs::path&)> loader) {
			std::type_index typeIndex(typeid(T));
			mResLoaders[typeIndex] = std::make_unique<TypedResourceLoader<T>>(std::move(loader));
		}

		template<Resource T>
		static void RemoveLoader() {
			std::type_index typeIndex(typeid(T));
			mResLoaders.erase(typeIndex);
		}

		static void ClearAll() {
			ResourceCache::Clear();
			mResLoaders.clear();
		}
	};
}