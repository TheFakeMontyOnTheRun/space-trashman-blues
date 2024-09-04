//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include <ppl.h>
#include <ppltasks.h>
#include <wrl\wrappers\corewrappers.h>

using namespace concurrency;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

#include "MainPage.g.h"

namespace UWP2022
{
	using namespace Microsoft::Graphics::Canvas::UI;
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		void CreateResources(Xaml::CanvasAnimatedControl^ sender, CanvasCreateResourcesEventArgs^ args);

		void Update(Xaml::CanvasAnimatedControl^ sender, Xaml::CanvasAnimatedUpdateEventArgs^ args);

		void Draw(Xaml::CanvasAnimatedControl^ sender, Xaml::CanvasAnimatedDrawEventArgs^ args);


        //---------------------------------------------------------------------
        void OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args);


        void Page_BackRequested(Object^ sender, BackRequestedEventArgs^ args);

        //---------------------------------------------------------------------
        void OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args);

        //---------------------------------------------------------------------
        void OnCharReceived(CoreWindow^ sender, CharacterReceivedEventArgs^ args);

        //---------------------------------------------------------------------
        void OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args);

        //---------------------------------------------------------------------
        void OnPointerWheelChanged(CoreWindow^ sender, PointerEventArgs^ args);

        //---------------------------------------------------------------------
        void OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args);


        //---------------------------------------------------------------------


        //---------------------------------------------------------------------
        void OnGamepadAdded(Platform::Object^ sender,
            Windows::Gaming::Input::Gamepad^ args);
	};
}
