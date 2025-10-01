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
#include "Widgets/SWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Layout/Visibility.h"
#include "SlotBase.h"
#include "Mcro/FunctionTraits.h"
#include "Mcro/Range/Iterators.h"
#include "Mcro/Range/Views.h"

/**
 *	@brief
 *	Extra functionalities for general Slate programming chores, including enhancements of the Slate declarative syntax
 */
namespace Mcro::Slate
{
	using namespace Mcro::FunctionTraits;

	/** @brief Constraining given type to a Slate widget */
	template <typename T>
	concept CWidget = CDerivedFrom<T, SWidget>;

	/** @brief Constraining given type to a slot of a widget */
	template <typename T>
	concept CSlot = CDerivedFrom<T, FSlotBase>;

	/** @brief Constraining given type to either a slot or a widget */
	template <typename T>
	concept CWidgetOrSlot = CWidget<T> || CSlot<T>;

	/** @brief Constraining given type to the arguments of either a widget or a slot */
	template <typename T>
	concept CWidgetOrSlotArguments = CSameAsDecayed<T, typename std::decay_t<T>::WidgetArgsType>;

	/** @brief Constraining given type to the arguments of a widget  */
	template <typename T>
	concept CWidgetArguments = requires(typename std::decay_t<T>::WidgetType& t) { t; };

	/** @brief Constraining given type to the arguments of a slot  */
	template <typename T>
	concept CSlotArguments = CDerivedFrom<T, FSlotBase::FSlotArguments>
		&& requires(T& args)
		{
			args.GetSlot();
		}
	;

	/** @brief Constraining given type to a widget which can receive slots */
	template <typename T>
	concept CWidgetWithSlots = requires(typename T::FSlot& t) { t; };

	template <typename T>
	concept CBoxPanelWidget = CWidgetWithSlots<T>
		&& CDerivedFrom<T, SBoxPanel>
		&& requires(T&& t, typename T::FScopedWidgetSlotArguments& slotArgs)
		{
			{ t.AddSlot() } -> CSameAsDecayed<typename T::FScopedWidgetSlotArguments>;
		}
	;

	template <typename T>
	struct TArgumentsOf_Struct {};
	
	template <CWidget T>
	struct TArgumentsOf_Struct<T>
	{
		using Type = typename T::FArguments;
	};
	
	template <CSlot T>
	struct TArgumentsOf_Struct<T>
	{
		using Type = typename T::FSlotArguments;
	};

	/** @brief Get the type of arguments from either a widget or a slot type (FArguments or FSlotArguments) */
	template <typename T>
	using TArgumentsOf = typename TArgumentsOf_Struct<T>::Type;

	/**
	 *	@brief
	 *	An attribute block functor which takes in reference of FArguments or FSlotArguments and returns the
	 *	same reference but presumably setting some Slate attributes before that. This is useful for modularizing the
	 *	Slate declarative syntax.
	 *
	 *	Use `TAttributeBlock` alias for your functions for better convenience
	 */
	template <CWidgetOrSlotArguments T>
	struct TAttributeBlockFunctor
	{
		using Function = TFunction<T&(T&)>;

		template <CConvertibleToDecayed<Function> Arg>
		TAttributeBlockFunctor(Arg&& function) : Storage(FWD(function)) {}

		Function Storage;

		T& operator () (T& args) const
		{
			return Storage(args);
		}
		
		/**
		 *	@brief
		 *	The "append attribute block" operator which allows pre-defined "blocks of slate attributes" naturally fit
		 *	inside the Slate declarative syntax. Traditionally repeated structures in Slate were expressed as either
		 *	explicit mutations on widgets after they were created or as entirely separate compound widgets. Either way
		 *	breaks the flow of the declarative syntax and makes using Slate sometimes pretty clunky. This operator aims
		 *	to make widget composition more comfortable.
		 *	
		 *	@tparam   Arguments  Right hand side FArguments or FSlotArguments
		 *	@param         args  r-value reference right hand side FArguments or FSlotArguments
		 *	@param   attributes  An attribute block function
		 *	@return  The same reference as args or a new slot if that has been added inside the attribute block
		 */
		friend T& operator / (TIdentity_T<T>&& args, TAttributeBlockFunctor const& attributes)
		{
			return attributes(args);
		}
		
		/**
		 *	@brief
		 *	The "append attribute block" operator which allows pre-defined "blocks of slate attributes" naturally fit
		 *	inside the Slate declarative syntax. Traditionally repeated structures in Slate were expressed as either
		 *	explicit mutations on widgets after they were created or as entirely separate compound widgets. Either way
		 *	breaks the flow of the declarative syntax and makes using Slate sometimes pretty clunky. This operator aims
		 *	to make widget composition more comfortable.
		 *	
		 *	@tparam   Arguments  Right hand side FArguments or FSlotArguments
		 *	@param         args  l-value reference right hand side FArguments or FSlotArguments
		 *	@param   attributes  An attribute block function
		 *	@return  The same reference as args or a new slot if that has been added inside the attribute block
		 */
		friend T& operator / (T& args, TAttributeBlockFunctor const& attributes)
		{
			return attributes(args);
		}
	};

	/**
	 *	@brief
	 *	An attribute block functor which takes in reference of FArguments or FSlotArguments and returns the
	 *	same reference but presumably setting some Slate attributes before that. This is useful for modularizing the
	 *	Slate declarative syntax.
	 */
	template <CWidgetOrSlot T>
	using TAttributeBlock = TAttributeBlockFunctor<TArgumentsOf<T>>;

	/** @brief An attribute block which does nothing */
	template <CWidgetOrSlot T>
	TAttributeBlock<T> InertAttributeBlock = [](TArgumentsOf<T>& args) -> auto& { return args; };

	/**
	 *	@brief  Add multiple slots at the same time with the declarative syntax derived from an input data array.
	 *	
	 *	@code
	 *	void SMyWidget::Construct(const FArguments& args)
	 *	{
	 *		using namespace Mcro::Slate;
	 *		
	 *		ChildSlot
	 *		[
	 *			SNew(SVerticalBox)
	 *			+ TSlots(args._DataArray, [](const FMyData& data)
	 *			{
	 *				FText dataText = FText::FromString(data.ToString());
	 *				return MoveTemp(SVerticalBox::Slot()
	 *					. HAlign(HAlign_Fill)
	 *					. AutoHeight()
	 *					[ SNew(STextBlock).Text(dataText) ];
	 *				);
	 *			})
	 *			+ SVerticalBox::Slot()
	 *			. HAlign(HAlign_Fill)
	 *			. AutoHeight()
	 *			[
	 *				SNew(STextBlock)
	 *				. Text(INVTEXT_"Footer after the list of data")
	 *			]
	 *		];
	 *	}
	 *	@endcode
	 */
	template <
		CRangeMember Range,
		CFunctionLike Transform,
		CFunctionLike OnEmpty = TUniqueFunction<TFunction_Return<Transform>()>,
		CSlotArguments = TFunction_Return<Transform>
	>
	requires (TFunction_ArgCount<Transform> == 1)
	struct TSlots
	{
		TSlots(Range const& range, Transform&& transform, TOptional<OnEmpty>&& onEmpty = {})
			: RangeRef(const_cast<Range&>(range))
			, TransformStorage(MoveTemp(transform))
			, OnEmptyStorage(MoveTemp(onEmpty))
		{}
		TSlots(Range& range, Transform&& transform, TOptional<OnEmpty>&& onEmpty = {})
			: RangeRef(range)
			, TransformStorage(MoveTemp(transform))
			, OnEmptyStorage(MoveTemp(onEmpty))
		{}

		TSlots(TSlots const&) = delete;
		TSlots(TSlots&& o) noexcept
			: RangeRef(o.RangeRef)
			, TransformStorage(MoveTemp(o.TransformStorage))
			, OnEmptyStorage(MoveTemp(o.OnEmptyStorage))
		{
		}

		TSlots& operator=(TSlots const&) = delete;
		TSlots& operator=(TSlots&& o) noexcept
		{
			if (this == &o)
				return *this;
			RangeRef = o.RangeRef;
			TransformStorage = MoveTemp(o.TransformStorage);
			OnEmptyStorage = MoveTemp(o.OnEmptyStorage);
			return *this;
		}

		template <CWidgetArguments Arguments>
		void Append(Arguments& args)
		{
			using namespace Mcro::Range;
			if (IteratorEquals(RangeRef.begin(), RangeRef.end()) && OnEmptyStorage.IsSet())
			{
				args + OnEmptyStorage.GetValue()();
				return;
			}

			for (auto it = RangeRef.begin(); !IteratorEquals(it, RangeRef.end()); ++it)
				args + TransformStorage(*it);
		}

		/** @copydoc TSlots */
		template <CWidgetArguments Arguments>
		friend Arguments& operator + (Arguments&& args, TSlots&& slots)
		{
			slots.Append(args);
			return args;
		}

	private:
		Range& RangeRef;
		Transform TransformStorage;
		TOptional<OnEmpty> OnEmptyStorage;
	};

	/**
	 *	@brief  Create widget slots from ranges of tuples via a transformation with structured binding arguments.
	 *	@see    Mcro::Range::TransformTuple
	 *	
	 *	For example:
	 *	@code
	 *	void SMyWidget::Construct(const FArguments& args)
	 *	{
	 *		using namespace Mcro::Slate;
	 *		
	 *		ChildSlot
	 *		[
	 *			SNew(SVerticalBox)
	 *			+ SlotsFromTuples(args._DataMap, [](const FString& key, int32 value)
	 *			{
	 *				return MoveTemp(SVerticalBox::Slot()
	 *					. HAlign(HAlign_Fill)
	 *					. AutoHeight()
	 *					[
	 *						SNew(SHorizontalBox)
	 *						+ SHorizontalBox::Slot() [ SNew(STextBlock).Text(AsText(key)) ]
	 *						+ SHorizontalBox::Slot() [ SNew(STextBlock).Text(AsText(value)) ]
	 *					];
	 *				);
	 *			})
	 *			+ SVerticalBox::Slot()
	 *			. HAlign(HAlign_Fill)
	 *			. AutoHeight()
	 *			[
	 *				SNew(STextBlock)
	 *				. Text(INVTEXT_"Footer after the list of data")
	 *			]
	 *		];
	 *	}
	 *	@endcode 
	 */
	template <
		CFunctionLike Transform,
		Mcro::Range::CRangeOfTuplesCompatibleWithFunction<Transform> Range,
		CFunctionLike OnEmpty = TUniqueFunction<TFunction_Return<Transform>()>,
		CSlotArguments = TFunction_Return<Transform>
	>
	requires (TFunction_ArgCount<Transform> >= 1)
	auto SlotsFromTuples(Range&& range, Transform&& transform, TOptional<OnEmpty>&& onEmpty = {})
	{
		using namespace Mcro::Range;
		using Tuple = TRangeElementType<Range>;
		
		return TSlots(FWD(range), [transform](Tuple const& tuple)
		{
			return InvokeWithTuple(transform, tuple);
		}, FWD(onEmpty));
	}

	/** @brief Convenience function for typing less when widget visibility depends on a boolean */
	MCRO_API EVisibility IsVisible(bool visible, EVisibility hiddenState = EVisibility::Collapsed);
}
