/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author David Mórász
 *  @date 2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Mcro/FunctionTraits.h"
#include "Mcro/Construct.h"

/** @brief Utilities for TSharedPtr/Ref and related */
namespace Mcro::SharedObjects
{
	using namespace Mcro::Concepts;
	using namespace Mcro::FunctionTraits;

	/** @brief Copy thread-safety from other shared object type */
	template <CSharedOrWeak T>
	using TSharedPtrFrom = TSharedPtr<typename T::ElementType, T::Mode>;

	/** @brief Copy thread-safety from other shared object type */
	template <CSharedOrWeak T>
	using TSharedRefFrom = TSharedRef<typename T::ElementType, T::Mode>;

	/** @brief Copy thread-safety from other shared object type */
	template <CSharedOrWeak T>
	using TWeakPtrFrom = TWeakPtr<typename T::ElementType, T::Mode>;

	/**
	 *	@brief  Concept describing an object which provides a deferred initializer for shared objects.
	 *	
	 *	This works around the annoyance of TSharedFromThis objects cannot use their shared pointers in their
	 *	constructor, braking RAII in some cases. Of course this is only achievable if the object cooperates and
	 *	implements an Initialize method.
	 *	It's important that `Initialize` should not be virtual so derived classes can hide them with their own
	 *	overload.
	 */
	template <typename T, typename... Args>
	concept CSharedInitializeable = requires(T& object, Args&&... args)
	{
		object.Initialize(FWD(args)...);
	};

	/**
	 *	@brief
	 *	A wrapper around MakeShareable that automatically calls an initializer method Initialize on the
	 *	instantiated object.
	 *	
	 *	This works around the annoyance of TSharedFromThis objects cannot use their shared pointers in their
	 *	constructor, braking RAII in some cases. Of course this is only achievable if the object cooperates and
	 *	implements an Initialize method.
	 *	It's important that Initialize should not be virtual so derived classes can hide them with their own
	 *	overload.
	 */
	template <typename T, ESPMode Mode = ESPMode::ThreadSafe, typename... Args>
	requires CSharedInitializeable<T, Args...>
	TSharedRef<T, Mode> MakeShareableInit(T* newObject, Args&&... args)
	{
		TSharedRef<T, Mode> result = MakeShareable(newObject);
		result->Initialize(FWD(args)...);
		return result;
	}

	/**
	 *	@brief  A combination of MakeShareable and Mcro::Construct::ConstructNew
	 *	
	 *	Usage:
	 *	@code
	 *	using namespace Mcro::SharedObjects;
	 *	
	 *	auto myObject = ConstructShared([](MyObject& _)
	 *	{
	 *		_.Foo = 42;
	 *		_.Initialize();
	 *		// etc...
	 *	});
	 *	static_assert(std::is_same_v<decltype(myObject), TSharedRef<MyObject>>);
	 *	@endcode
	 *
	 *	@param    init  A lambda function with a single l-value reference parameter of the object type to initialize.
	 *	@param    args  Arguments of the object constructor
	 *	@return   A pointer to the object instance on heap.
	 *	@remarks  The C++ 20 designated initializers with named arguments has annoying limitations, therefore this exists
	 */
	template <
		CFunctorObject Initializer,
		typename... Args,
		ESPMode Mode = ESPMode::ThreadSafe,
		typename ResultArg = TFunction_Arg<Initializer, 0>,
		typename Result = std::decay_t<ResultArg>
	>
	requires std::is_lvalue_reference_v<ResultArg>
	TSharedRef<Result, Mode> ConstructShared(Initializer&& init, Args&&... args)
	{
		using namespace Mcro::Construct;
		return MakeShareable(
			ConstructNew(FWD(init), FWD(args)...)
		);
	}

	/**
	 *	@brief  Create a shared pointer which takes in an object with in-place refcounting.
	 *	
	 *	Refcounted TSharedPtr/Ref doesn't take ownership of the object and when the last reference goes out of scope it
	 *	simply decreases the refcount instead of deleting the object.
	 */
	template <CRefCounted T, ESPMode Mode = ESPMode::ThreadSafe>
	TSharedRef<T, Mode> ShareRefCounted(T* object)
	{
		check(object);
		object->AddRef();
		return MakeShareable(object, [](T* object) { object->Release(); });
	}

	/**
	 *	@brief
	 *	Same as `SharedThis(this)` in `TSharedFromThis` but returning a weak pointer instead.
	 *	
	 *	Indeed, `TSharedFromThis` already has `AsWeak()` but that can only return the type which was originally set by
	 *	`TSharedFromThis` making its usage slightly less convenient in derived classes.
	 */
	template <
		CSharedFromThis T,
		ESPMode Mode = decltype(DeclVal<T const>().AsShared())::Mode
	>
	auto WeakSelf(const T* self) -> TWeakPtr<T const, Mode>
	{
		return StaticCastSharedRef<std::decay_t<T> const>(self->AsShared());
	}

	/**
	 *	@brief
	 *	Same as `SharedThis(this)` in `TSharedFromThis` but returning a weak pointer instead.
	 *	
	 *	Indeed, `TSharedFromThis` already has `AsWeak()` but that can only return the type which was originally set by
	 *	`TSharedFromThis` making its usage slightly less convenient in derived classes.
	*/
	template <
		CSharedFromThis T,
		ESPMode Mode = decltype(DeclVal<T>().AsShared())::Mode
	>
	auto WeakSelf(T* self) -> TWeakPtr<T, Mode>
	{
		return StaticCastSharedRef<std::decay_t<T>>(self->AsShared());
	}

	/** @brief Same as `SharedThis(this)` in `TSharedFromThis`. */
	template <
		CSharedFromThis T,
		ESPMode Mode = decltype(DeclVal<T const>().AsShared())::Mode
	>
	auto SharedSelf(const T* self) -> TSharedRef<T const, Mode>
	{
		return StaticCastSharedRef<std::decay_t<T> const>(self->AsShared());
	}

	/** @brief Same as `SharedThis(this)` in `TSharedFromThis`. */
	template <
		CSharedFromThis T,
		ESPMode Mode = decltype(DeclVal<T>().AsShared())::Mode
	>
	auto SharedSelf(T* self) -> TSharedRef<T, Mode>
	{
		return StaticCastSharedRef<std::decay_t<T>>(self->AsShared());
	}

	/**
	 *	@brief
	 *	A simple mutable shared storage of any value.
	 *
	 *	One particular use case for this may be delegates which may remove themselves once invoked. For example:
	 *	@code
	 *	auto once = MakeSharedDelegateHandle();
	 *	once->Value = MyEvent.AddLambda([this, once]
	 *	{
	 *		// execute something only once.
	 *		MyEvent.Remove(once->Value);
	 *	});
	 *	@endcode
	 */
	template <CDefaultInitializable T>
	struct TSharedStorage
	{
		T Value;
	};
	
	template <CDefaultInitializable T>
	using TSharedStoragePtr = TSharedPtr<TSharedStorage<T>>;
	
	template <CDefaultInitializable T>
	using TSharedStorageRef = TSharedRef<TSharedStorage<T>>;
	
	template <CDefaultInitializable T>
	using TSharedStorageWeakPtr = TWeakPtr<TSharedStorage<T>>;

	/** @brief A simple convenience wrapper around making FDelegateHandles shared */
	FORCEINLINE TSharedStorageRef<FDelegateHandle> MakeSharedDelegateHandle()
	{
		return MakeShared<TSharedStorage<FDelegateHandle>>();
	}
}
