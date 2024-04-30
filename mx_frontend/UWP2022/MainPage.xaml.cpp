//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#define SDLSW




#define CLI_BUILD
#define PAGE_FLIP_ANIMATION
#define INCLUDE_ITEM_DESCRIPTIONS
#define ENDIANESS_AWARE
#define EMIT_QUIT_OPTION
#define XRES_FRAMEBUFFER 320
#define YRES_FRAMEBUFFER 200


#include "Crawler.c"
#include "MainMenu.c"
#include "HelpScreen.c"
#include "CreditsScreen.c"
#include "GameMenu.c"
#include "Engine.c"
#include "LoadBitmap.c"
#include "Renderer.c"
#include "VisibilityStrategy.c"
#include "Globals.c"
#include "CTile3DProperties.c"
#include "Vec.c"
#include "Common.c"
#include "FixP.c"
#include "Mesh.c"
#include "MapWithCharKey.c"
#include "PackedFileReader.c"
#include "EDirection_Utils.c"
#include "Dungeon.c"
#include "Events.c"
#include "RendererRasterization.c"
#include "RendererTesselation.c"
#include "UI.c"
#include "HackingScreen.c"
#include "PCMAudio.c"
#include "HackingMinigameRules.c"
#include "Derelict.c"
#include "Core.c"
#include "Parser.c"

using namespace UWP2022;


using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace Microsoft::Graphics::Canvas::UI;
using namespace Platform;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::System::Threading;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Effects;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;


struct RGBEntry {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

#define CONTROLLER_COOLDOWN_TIME 150
#define CONTROLLER_TRIGGER_COOLDOWN_TIME 500

Platform::Array<uint8_t>^ data = nullptr;
CanvasSpriteBatch^ spriteBatch2 = nullptr;
Microsoft::Graphics::Canvas::CanvasBitmap^ bitmap = nullptr;
Windows::Gaming::Input::Gamepad^ mainGamepad = nullptr;
struct RGBEntry paletteRef[256];
uint8_t pixels[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * 4];
int32_t controllerCooldown = 0;
int32_t controllerTriggerCooldown = 0;
using namespace Microsoft::Graphics::Canvas::UI;
extern ESoundDriver soundDriver;

extern ECommand mBufferedCommand;
extern ECommand mTurnBuffer;
extern uint8_t isRunning;
extern long timeUntilNextState;
extern struct Bitmap* currentBackgroundBitmap;
extern int8_t cursorPosition;
extern enum EGameMenuState currentGameMenuState;
extern const char* mainText;

enum ESoundDriver soundDriver;

std::vector<Windows::Media::Playback::MediaPlayer^> playingSounds;

char keyState[256];



char* textBuffer;
extern char* messageLogBuffer;
extern enum EVisibility* visMap;
extern struct Vec2i* distances;
extern uint8_t* collisionMap;
extern struct Texture* textures;

struct Texture internalTexturesMem[TOTAL_TEXTURES];

void initHW(int argc, char** argv) {
    textBuffer = (char*)allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
    messageLogBuffer = (char*)allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t*)allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility*)allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i*)allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    textures = &internalTexturesMem[0];
    itemsInMap = (uint8_t*)allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t*), GENERAL_MEMORY, 1);
    map = (uint8_t*)allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t*), GENERAL_MEMORY, 1);

#ifndef CD32
    initFileReader("base.pfs");
#else
    initFileReader("base.pfs");
#endif
    initZMap();
    graphicsInit();
}

void shutdownHW(void) {
    graphicsShutdown();

    disposeMem(textBuffer);
    disposeMem(messageLogBuffer);
    disposeMem(collisionMap);
    disposeMem(visMap);
    disposeMem(distances);
    disposeMem(itemsInMap);
    disposeMem(map);
}


// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();


    // Tell Windows about our OnKeyDown and OnKeyUp event handlers.
    Window::Current->CoreWindow->KeyDown += ref new TypedEventHandler
        <CoreWindow^, KeyEventArgs^>(this, &MainPage::OnKeyDown);

    Window::Current->CoreWindow->KeyUp += ref new TypedEventHandler
        <CoreWindow^, KeyEventArgs^>(this, &MainPage::OnKeyUp);

    // Tell Windows about our OnCharReceived event handler.
    Window::Current->CoreWindow->CharacterReceived += ref new TypedEventHandler
        <CoreWindow^, CharacterReceivedEventArgs^>(this, &MainPage::OnCharReceived);

    // Tell Windows about our OnPointerPressed event handler.
    Window::Current->CoreWindow->PointerPressed += ref new TypedEventHandler
        <CoreWindow^, PointerEventArgs^>(this, &MainPage::OnPointerPressed);

    // Tell Windows about our OnPointerWheelChanged event handler.
    Window::Current->CoreWindow->PointerWheelChanged += ref new TypedEventHandler
        <CoreWindow^, PointerEventArgs^>(this, &MainPage::OnPointerWheelChanged);

    // Tell Windows about our OnPointerMoved event handler.
    Window::Current->CoreWindow->PointerMoved += ref new TypedEventHandler
        <CoreWindow^, PointerEventArgs^>(this, &MainPage::OnPointerMoved);

    // Tell Windows about or GamepadAdded event handler.
    Windows::Gaming::Input::Gamepad::GamepadAdded +=
        ref new Windows::Foundation::EventHandler
        <Windows::Gaming::Input::Gamepad^>(this, &MainPage::OnGamepadAdded);

    SystemNavigationManager::GetForCurrentView()->BackRequested += ref new EventHandler<BackRequestedEventArgs^>(this, &MainPage::Page_BackRequested);

}


uint8_t getPaletteEntry(const uint32_t origin) {
	uint8_t shade;

	if (!(origin & 0xFF000000)) {
		return TRANSPARENCY_COLOR;
	}

	shade = 0;
	shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
	shade += (((((origin & 0x00FF00) >>  8) << 3) >> 8)) << 3;
	shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

	return shade;
}

void graphicsInit(void) {
    int r, g, b;

    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                pixel = 0xFF000000 + (b << 16) + (g << 8) + (r);
                palette[paletteEntry] = pixel;
            }
        }
    }

    defaultFont = loadBitmap("font.img");
    enableSmoothMovement = TRUE;
}

void clearRenderer(void) {}



void MainPage::Page_BackRequested(Object^ sender, BackRequestedEventArgs^ args)
{
    args->Handled = true;
}

//-----------------------------------------------------------------------------
void MainPage::CreateResources(Xaml::CanvasAnimatedControl^ sender,
    CanvasCreateResourcesEventArgs^ args)
{
    TimeSpan period;
    period.Duration = 10000000 / 17; // Set Updates/Second
    sender->TargetElapsedTime = period;

    // Lambda function for timer
    ThreadPoolTimer^ periodicTimer =
        ThreadPoolTimer::CreatePeriodicTimer(
            ref new TimerElapsedHandler([&](ThreadPoolTimer^ source) {}),
            period);

    ApplicationView^ view = ApplicationView::GetForCurrentView();
    view->TryEnterFullScreenMode();

    srand(time(NULL));
    initHW(0, NULL);

    enterState(kMainMenu);
}

//-----------------------------------------------------------------------------
void MainPage::Update(Xaml::CanvasAnimatedControl^ sender,
    Xaml::CanvasAnimatedUpdateEventArgs^ args)
{


    if (mainGamepad != nullptr) {
        Windows::Gaming::Input::GamepadReading reading = mainGamepad->GetCurrentReading();

        if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::Y) == Windows::Gaming::Input::GamepadButtons::Y) {
            mBufferedCommand = kCommandFire3;
            if (controllerCooldown <= 0) {
                controllerCooldown = CONTROLLER_COOLDOWN_TIME;
            }
        }
        else
            if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::A) == Windows::Gaming::Input::GamepadButtons::A) {
                mBufferedCommand = kCommandFire1;
                if (controllerCooldown <= 0) {
                    controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                }
            }
            else
                if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::X) == Windows::Gaming::Input::GamepadButtons::X) {
                    mBufferedCommand = kCommandFire2;
                    if (controllerCooldown <= 0) {
                        controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                    }
                }
                else
                    if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::B) == Windows::Gaming::Input::GamepadButtons::B) {

                        if (currentGameMenuState == kMainMenu) {
                            graphicsShutdown();
                        }
                        else {
                            mBufferedCommand = kCommandBack;
                        }

                        if (controllerCooldown <= 0) {
                            controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                        }
                    }
                    else
                        if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::Menu) == Windows::Gaming::Input::GamepadButtons::Menu) {
                            mBufferedCommand = kCommandBack;
                            if (controllerCooldown <= 0) {
                                controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                            }
                        }
                        else

                            if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::DPadUp) == Windows::Gaming::Input::GamepadButtons::DPadUp) {
                                mBufferedCommand = kCommandUp;
                                if (controllerCooldown <= 0) {
                                    controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                }
                            }
                            else

                                if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::DPadDown) == Windows::Gaming::Input::GamepadButtons::DPadDown) {
                                    mBufferedCommand = kCommandDown;
                                    if (controllerCooldown <= 0) {
                                        controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                    }
                                }
                                else

                                    if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::DPadLeft) == Windows::Gaming::Input::GamepadButtons::DPadLeft) {
                                        mTurnBuffer = kCommandLeft;

                                        if (controllerCooldown <= 0) {
                                            controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                        }
                                    }
                                    else

                                        if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::DPadRight) == Windows::Gaming::Input::GamepadButtons::DPadRight) {
                                            mTurnBuffer = kCommandRight;

                                            if (controllerCooldown <= 0) {
                                                controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                            }
                                        }
                                        else

                                            if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::LeftShoulder) == Windows::Gaming::Input::GamepadButtons::LeftShoulder) {
                                                mBufferedCommand = kCommandStrafeLeft;
                                                if (controllerCooldown <= 0) {
                                                    controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                                }
                                            }
                                            else

                                                if ((reading.Buttons & Windows::Gaming::Input::GamepadButtons::RightShoulder) == Windows::Gaming::Input::GamepadButtons::RightShoulder) {
                                                    mBufferedCommand = kCommandStrafeRight;
                                                    if (controllerCooldown <= 0) {
                                                        controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                                    }
                                                }
                                                else {
                                                    controllerCooldown = 0;
                                                }

        if (mBufferedCommand != kCommandNone && controllerCooldown > 0 && controllerCooldown != CONTROLLER_COOLDOWN_TIME) {
            mBufferedCommand = kCommandNone;
        }

        if (controllerCooldown > 0) {
            controllerCooldown -= 30;
        }

        if (controllerTriggerCooldown > 0) {
            controllerTriggerCooldown -= 30;
        }
    }
    visibilityCached = false;
    menuTick(30);
}

//-----------------------------------------------------------------------------
void MainPage::Draw(Xaml::CanvasAnimatedControl^ sender,
    Xaml::CanvasAnimatedDrawEventArgs^ args)
{
    flipRenderer();
    spriteBatch2 = args->DrawingSession->CreateSpriteBatch();

    if (data == nullptr) {
        data = ref new Platform::Array<uint8_t>(&pixels[0], XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * 4);
    }
    else {
        for (int i = 0; i < YRES_FRAMEBUFFER * XRES_FRAMEBUFFER * 4; i++) {
            data[i] = pixels[i];
        }
    }

    if (bitmap == nullptr) {
        bitmap = Microsoft::Graphics::Canvas::CanvasBitmap::CreateFromBytes(sender, data, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized);
    }
    else {
        bitmap->SetPixelBytes(data);
    }

    float left = 0;
    float top = 0;
    float multiplier = 0;
    float width = 0;
    float height = 0;

    if (sender->Size.Width > sender->Size.Height) {
        multiplier = sender->Size.Height / 240.0f;
    }
    else {
        multiplier = sender->Size.Width / ((float)XRES_FRAMEBUFFER);
    }
    height = 240 * multiplier;
    width = XRES_FRAMEBUFFER * multiplier;

    left = (sender->Size.Width - width) / 2.0f;
    top = (sender->Size.Height - height) / 2.0f;

    Windows::Foundation::Rect rect(left, top, width, height);

    spriteBatch2->Draw(bitmap, rect);

    delete spriteBatch2;
}


//-----------------------------------------------------------------------------
void MainPage::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
{
    // Exit application if the Esc key is pressed
    if (args->VirtualKey == VirtualKey::Escape)
    {
        mBufferedCommand = kCommandBack;
        //ApplicationView^ view = ApplicationView::GetForCurrentView();
        //    view->ExitFullScreenMode();
        //ApplicationView::GetForCurrentView()->ExitFullScreenMode();
    }

    // F11 toggles full screen mode
    if (args->VirtualKey == VirtualKey::F11)
    {
        ApplicationView^ view = ApplicationView::GetForCurrentView();
        if (view->IsFullScreenMode)
            view->ExitFullScreenMode();
        else
            view->TryEnterFullScreenMode();
    }
}

//-----------------------------------------------------------------------------
void MainPage::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
{
    if (args->VirtualKey == VirtualKey::Z || args->VirtualKey == VirtualKey::Enter)
    {
        mBufferedCommand = kCommandFire1;
        if (controllerCooldown <= 0) {
            controllerCooldown = CONTROLLER_COOLDOWN_TIME;
        }
    }
    else
        if (args->VirtualKey == VirtualKey::X || args->VirtualKey == VirtualKey::Escape)
        {
            mBufferedCommand = kCommandFire2;
            if (controllerCooldown <= 0) {
                controllerCooldown = CONTROLLER_COOLDOWN_TIME;
            }
        }
        else
            if (args->VirtualKey == VirtualKey::C || args->VirtualKey == VirtualKey::Insert)
            {
                mBufferedCommand = kCommandFire3;
                if (controllerCooldown <= 0) {
                    controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                }
            }
            else
                if (args->VirtualKey == VirtualKey::V)
                {
                    mBufferedCommand = kCommandFire4;
                    if (controllerCooldown <= 0) {
                        controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                    }
                }
                else
                    if (args->VirtualKey == VirtualKey::Escape)
                    {
                        mBufferedCommand = kCommandBack;
                        if (controllerCooldown <= 0) {
                            controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                        }
                    }
                    else
                        if (args->VirtualKey == VirtualKey::Up)
                        {
                            mBufferedCommand = kCommandUp;
                            if (controllerCooldown <= 0) {
                                controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                            }
                        }
                        else
                            if (args->VirtualKey == VirtualKey::Down)
                            {
                                mBufferedCommand = kCommandDown;
                                if (controllerCooldown <= 0) {
                                    controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                }
                            }
                            else
                                if (args->VirtualKey == VirtualKey::Left)
                                {
                                    mTurnBuffer = kCommandLeft;

                                    if (controllerCooldown <= 0) {
                                        controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                    }
                                }
                                else
                                    if (args->VirtualKey == VirtualKey::Right)
                                    {
                                        mTurnBuffer = kCommandRight;

                                        if (controllerCooldown <= 0) {
                                            controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                        }
                                    }
                                    else
                                        if (args->VirtualKey == VirtualKey::S)
                                        {
                                            mBufferedCommand = kCommandStrafeLeft;
                                            if (controllerCooldown <= 0) {
                                                controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                            }
                                        }
                                        else
                                            if (args->VirtualKey == VirtualKey::D)
                                            {
                                                mBufferedCommand = kCommandStrafeRight;
                                                if (controllerCooldown <= 0) {
                                                    controllerCooldown = CONTROLLER_COOLDOWN_TIME;
                                                }
                                            }
                                            else {
                                                controllerCooldown = 0;
                                            }
    /*
        if (mBufferedCommand != kCommandNone && controllerCooldown > 0 && controllerCooldown != CONTROLLER_COOLDOWN_TIME) {
            mBufferedCommand = kCommandNone;
        }

        if (controllerCooldown > 0) {
            controllerCooldown -= 30;
        }
        */
}

//-----------------------------------------------------------------------------
void MainPage::OnCharReceived(CoreWindow^ sender, CharacterReceivedEventArgs^ args)
{
}

//-----------------------------------------------------------------------------
void MainPage::OnPointerPressed(CoreWindow^ sender,
    PointerEventArgs^ args)
{
}

//-----------------------------------------------------------------------------
void MainPage::OnPointerWheelChanged(CoreWindow^ sender,
    PointerEventArgs^ args)
{
}

//-----------------------------------------------------------------------------
void MainPage::OnPointerMoved(CoreWindow^ sender,
    PointerEventArgs^ args)
{
}

//-----------------------------------------------------------------------------
//OnGamepadAdded(Windows::Gaming::Input::Gamepad^ sender, PointerEventArgs^ args)

void MainPage::OnGamepadAdded(
    Platform::Object^ sender,
    Windows::Gaming::Input::Gamepad^ args)
{
    mainGamepad = args;
}

void setupOPL2(int port) {}

void stopSounds() {}

void OnMediaEnded(Windows::Media::Playback::MediaPlayer^ mp, Object^ obj) {

}

void playSound(const int action) {



}

void soundTick() {}

void muteSound() {}

long frame = 0;

void graphicsShutdown(void) {
    Application::Current->Exit();
}

void putStr(int x, int y, const char* str, int fg, int bg) {

}

void drawTitleBox(void) {

}

void querySoundDriver(void) {

}

void handleSystemEvents(void) {
    if (mainGamepad != nullptr) {
        auto reading = mainGamepad->GetCurrentReading();
        if ((reading.LeftTrigger >= 0.5f || reading.RightTrigger >= 0.5f) && controllerTriggerCooldown <= 0) {
            mBufferedCommand = kCommandFire1;
            controllerTriggerCooldown = CONTROLLER_TRIGGER_COOLDOWN_TIME;
        }

        if (reading.LeftTrigger <= 0.5f && reading.RightTrigger <= 0.5f) {
            controllerTriggerCooldown = 0;
        }
    }
}

void flipRenderer(void) {
    int x, y;
    int i = 0;
    int		    index = 0;
    uint8_t stretchedBuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];
    OutputPixelFormat	    bigPixel;

    renderPageFlip(&stretchedBuffer[0], &framebuffer[0],
        &previousFrame[0], turnStep, turnTarget, FALSE);


    for (y = 0; y < YRES_FRAMEBUFFER; ++y) {
        for (x = 0; x < XRES_FRAMEBUFFER; ++x) {
            bigPixel = palette[stretchedBuffer[index]];

            pixels[((XRES_FRAMEBUFFER * 4) * y) + (x * 4) + 0] = (bigPixel & 0xFF) - 0x10;
            pixels[((XRES_FRAMEBUFFER * 4) * y) + (x * 4) + 1] = ((bigPixel >> 8) & 0xFF) - 0x18;
            pixels[((XRES_FRAMEBUFFER * 4) * y) + (x * 4) + 2] = ((bigPixel >> 16) & 0xFF) - 0x38;
            pixels[((XRES_FRAMEBUFFER * 4) * y) + (x * 4) + 3] = ((bigPixel >> 24) & 0xFF);


            ++index;
        }
    }
}

void clear() {}



