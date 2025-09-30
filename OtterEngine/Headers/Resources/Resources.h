#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <concepts>

namespace OtterEngine {
	template<typename T>
	concept Resource = requires(T t, const std::string& path) {
		{ T::LoadFromFile(path) } -> std::convertible_to<std::shared_ptr<T>>;
		{ t.IsValid() } -> std::convertible_to<bool>;
	};

	template<typename T>
	class ResourceHandle {
	private:
		std::shared_ptr<T> mRes;
		std::string mPath;
	public:
		ResourceHandle() = default;
		ResourceHandle(std::shared_ptr<T> resource, const std::string& path)
			: mRes(std::move(resource)), mPath(path) {
		}
		ResourceHandle(std::shared_ptr<T>&& resource, const std::string& path)
			: mRes(std::move(resource)), mPath(path) {
		}

		T* operator->() const { 
			assert(mRes && "Attempting to dereference null Resource Handle!");
			return mRes.get(); }
		T& operator*() const { 
			assert(mRes && "Attempting to dereference null Resource Handle!"); 
			return *mRes; }
		explicit operator bool() const { return mRes && mRes->IsValid(); }

		const std::string& GetPath() const { return mPath; }
		std::shared_ptr<T> GetSharedPtr() const { return mRes; }
	};
}