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

#include "Mcro/Error/SErrorDisplay.h"
#include "Mcro/Range.h"
#include "Mcro/Range/Conversion.h"
#include "Mcro/Range/Views.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

namespace Mcro::Error
{
	using namespace Mcro::Slate;
	using namespace Mcro::Range;

	void SErrorDisplay::Construct(const FArguments& inArgs)
	{
		namespace rv = ranges::views;

		auto error = inArgs._Error.ToSharedRef();
		auto allExtensions = IErrorDisplayExtension::GetAll();
		auto extensions = allExtensions
			| rv::filter([error](IErrorDisplayExtension* i) { return i->SupportsError(error); })
			| RenderAs<TArray>()
		;
		
		ChildSlot
		[
			SNew(SVerticalBox)
			+ Row()[ SeverityWidget(error) ]
			+ Row()[ inArgs._PostSeverity.Widget ]
			+ TSlots(
				extensions
					| rv::transform([error](IErrorDisplayExtension* i)
					{
						return i->PostSeverity(error);
					})
					| FilterValid()
				,
				[](TSharedPtr<SWidget> const& i)
				{
					return MoveTemp(Row()[ i.ToSharedRef() ]);
				}
			)
			
			+ Row()[ OptionalTextWidget(inArgs._Error->GetMessage()) ]
			+ Row()[ inArgs._PostMessage.Widget ]
			+ TSlots(
				extensions
					| rv::transform([error](IErrorDisplayExtension* i)
					{
						return i->PostMessage(error);
					})
					| FilterValid()
				,
				[](TSharedPtr<SWidget> const& i)
				{
					return MoveTemp(Row()[ i.ToSharedRef() ]);
				}
			)
			
			+ Row()[ ExpandableTextWidget(INVTEXT_"Further details", inArgs._Error->GetDetails()) ]
			+ Row()[ inArgs._PostDetails.Widget ]
			+ TSlots(
				extensions
					| rv::transform([error](IErrorDisplayExtension* i)
					{
						return i->PostDetails(error);
					})
					| FilterValid()
				,
				[](TSharedPtr<SWidget> const& i)
				{
					return MoveTemp(Row()[ i.ToSharedRef() ]);
				}
			)
			
			+ Row()[ ExpandableTextWidget(INVTEXT_"Code context", inArgs._Error->GetCodeContext()) ]
			+ Row()[ inArgs._PostCodeContext.Widget ]
			+ TSlots(
				extensions
					| rv::transform([error](IErrorDisplayExtension* i)
					{
						return i->PostCodeContext(error);
					})
					| FilterValid()
				,
				[](TSharedPtr<SWidget> const& i)
				{
					return MoveTemp(Row()[ i.ToSharedRef() ]);
				}
			)
			
			+ Row()[ ExpandableTextWidget(INVTEXT_"Error Propagation", inArgs._Error->GetErrorPropagationJoined()) ]
			+ Row()[ inArgs._PostErrorPropagation.Widget ]
			+ TSlots(
				extensions
					| rv::transform([error](IErrorDisplayExtension* i)
					{
						return i->PostErrorPropagation(error);
					})
					| FilterValid()
				,
				[](TSharedPtr<SWidget> const& i)
				{
					return MoveTemp(Row()[ i.ToSharedRef() ]);
				}
			)
			
			+ TSlots(error.Get(), [&](const FNamedError& inner)
			{
				return MoveTemp(Row()
				[
					SNew(SExpandableArea)
					. AreaTitle(FText::FromString(inner.Key))
					. InitiallyCollapsed(true)
					. Padding(FMargin(20, 0, 0, 0))
					. BodyContent()
					[
						inner.Value->CreateErrorWidget()
					]
				]);
			})
			+ Row()[ inArgs._PostInnerErrors.Widget ]
			+ TSlots(
				extensions
					| rv::transform([error](IErrorDisplayExtension* i)
					{
						return i->PostInnerErrors(error);
					})
					| FilterValid()
				,
				[](TSharedPtr<SWidget> const& i)
				{
					return MoveTemp(Row()[ i.ToSharedRef() ]);
				}
			)
		];
	}

	auto SErrorDisplay::Text(const FString& text) -> TAttributeBlock<SEditableTextBox>
	{
		return [&](SEditableTextBox::FArguments& args) -> auto&
		{
			return args
			. IsReadOnly(true)
			. Text(FText::FromString(text))
			. Font(FCoreStyle::GetDefaultFontStyle("Mono", 9));
		};
	}

	auto SErrorDisplay::Text(const FStringView& text) -> Slate::TAttributeBlock<SEditableTextBox>
	{
		return [&](SEditableTextBox::FArguments& args) -> auto&
		{
			return args
			. IsReadOnly(true)
			. Text(FText::FromStringView(text))
			. Font(FCoreStyle::GetDefaultFontStyle("Mono", 9));
		};
	}

	auto SErrorDisplay::OptionalText(const FString& text) -> TAttributeBlock<SEditableTextBox>
	{
		return [&](SEditableTextBox::FArguments& args) -> auto&
		{
			return args
				. Visibility(IsVisible(!text.IsEmpty()))
				/ Text(text);
		};
	}

	auto SErrorDisplay::OptionalTextWidget(const FString& text) -> TSharedRef<SEditableTextBox>
	{
		return SNew(SEditableTextBox) / OptionalText(text);
	}

	auto SErrorDisplay::ExpandableText(const FText& title, const FString& text) -> TAttributeBlock<SExpandableArea>
	{
		return [&](SExpandableArea::FArguments& args) -> auto&
		{
			return args
				. Visibility(IsVisible(!text.IsEmpty()))
				. AreaTitle(title)
				. InitiallyCollapsed(true)
				. BodyContent()
				[
					SNew(SEditableTextBox) / Text(text)
				];
		};
	}

	auto SErrorDisplay::ExpandableTextWidget(const FText& title, const FString& text) -> TSharedRef<SExpandableArea>
	{
		return SNew(SExpandableArea) / ExpandableText(title, text);
	}

	auto SErrorDisplay::Severity(const IErrorRef& error) -> TAttributeBlock<STextBlock>
	{
		return [&](STextBlock::FArguments& args) -> auto&
		{
			auto severity = error->GetSeverityString();
			
			return args
				. Visibility(IsVisible(error->GetSeverity() >= EErrorSeverity::Recoverable))
				. SimpleTextMode(true)
				. Text(FText::FromStringView(severity));
		};
	}
	
	auto SErrorDisplay::SeverityWidget(const IErrorRef& error) -> TSharedRef<STextBlock>
	{
		return SNew(STextBlock)
			. Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			/ Severity(error);
	}

	auto SErrorDisplay::Row() -> SVerticalBox::FSlot::FSlotArguments
	{
		return MoveTemp(SVerticalBox::Slot()
			. HAlign(HAlign_Fill)
			. AutoHeight()
		);
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
