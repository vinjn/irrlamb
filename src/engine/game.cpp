/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
*	Copyright (C) 2013  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include <engine/game.h>
#include <engine/input.h>
#include <engine/globals.h>
#include <engine/graphics.h>
#include <engine/interface.h>
#include <engine/audio.h>
#include <engine/state.h>
#include <engine/log.h>
#include <engine/fader.h>
#include <engine/random.h>
#include <engine/scripting.h>
#include <engine/physics.h>
#include <engine/objectmanager.h>
#include <engine/config.h>
#include <engine/save.h>
#include <engine/campaign.h>
#include <engine/constants.h>
#include <play.h>
#include <viewreplay.h>
#include <menu.h>
#include <null.h>
#include <IFileSystem.h>

using namespace irr;

_Game Game;

// Processes parameters and initializes the game
int _Game::Init(int Count, char **Arguments) {

	// Defaults
	SleepRate = SLEEP_RATE;
	TimeStep = PHYSICS_TIMESTEP;
	TimeStepAccumulator = 0.0f;
	TimeScale = 1.0f;
	LastFrameTime = 0.0f;
	TimeStamp = 0;
	WindowActive = true;
	MouseWasLocked = false;
	Done = false;
	_State *FirstState = &NullState;
	video::E_DRIVER_TYPE DriverType = video::EDT_NULL;
	bool AudioEnabled = true;
	PlayState.SetCampaign(-1);
	PlayState.SetCampaignLevel(-1);

	// Process arguments
	std::string Token;
	int TokensRemaining;
	for(int i = 1; i < Count; i++) {
		Token = std::string(Arguments[i]);
		TokensRemaining = Count - i - 1;
		if(Token == "-level" && TokensRemaining > 0) {
			PlayState.SetTestLevel(Arguments[++i]);
			FirstState = &PlayState;
		}
		else if(Token == "-replay" && TokensRemaining > 0) {
			ViewReplayState.SetCurrentReplay(Arguments[++i]);
			FirstState = &ViewReplayState;
		}
		else if(Token == "-noaudio") {
			AudioEnabled = false;
		}
	}

	// Set up the save system
	if(!Save.Init())
		return 0;

	// Initialize logging system
	Log.Init();
	Log.Write("irrlamb %s", GAME_VERSION);

	// Query joysticks with null device
	Input.InitializeJoysticks(true);

	// Set up config system
	if(!Config.Init())
		return 0;

	// Read the config file
	int HasConfigFile = Config.ReadConfig();

	// Set up the graphics
	DriverType = (video::E_DRIVER_TYPE)Config.DriverType;
	if(!Graphics.Init(Config.ScreenWidth, Config.ScreenHeight, Config.Fullscreen, DriverType, &Input))
		return 0;
	
	// Initialize the joysticks with the real device
	Input.InitializeJoysticks();
	
	// Save working path
	WorkingPath = std::string(irrFile->getWorkingDirectory().c_str()) + "/";

	// Write a config file if none exists
	if(!HasConfigFile)
		Config.WriteConfig();

	// Initialize level stats
	if(!Save.InitStatsDatabase())
		return 0;

	// Set random seed
	Random.SetSeed(irrTimer->getRealTime());

	// Set up the interface system
	if(!Interface.Init())
		return 0;

	// Set up audio
	if(AudioEnabled && Config.AudioEnabled)
		EnableAudio();

	// Set up the scripting system
	if(!Scripting.Init())
		return 0;

	// Set up the object manager
	if(!ObjectManager.Init())
		return 0;

	// Set up the object manager
	if(!Campaign.Init())
		return 0;

	// Set up physics world
	if(!Physics.Init())
		return 0;

	// Set up fader
	if(!Fader.Init())
		return 0;

	// Load stats file
	Save.LoadLevelStats();

	// Start the state timer
	ResetTimer();

	// Set the first state
	State = FirstState;
	NewState = NULL;
	ManagerState = STATE_INIT;
	Fader.Start(FADE_SPEED);

	return 1;
}

// Requests a state change
void _Game::ChangeState(_State *State) {
	TimeScale = 1.0f;
	Fader.Start(-FADE_SPEED);

	NewState = State;
	ManagerState = STATE_CLOSE;
}

// Updates the current state and runs the game engine
void _Game::Update() {

	// Run irrlicht engine
	if(!irrDevice->run())
		Done = true;

	// Get time difference from last frame
	LastFrameTime = (irrTimer->getTime() - TimeStamp) * 0.001f;
	TimeStamp = irrTimer->getTime();

	// Limit frame rate
	float ExtraTime = SleepRate - LastFrameTime;
	if(ExtraTime > 0.0f) {
		irrDevice->sleep((irr::u32)(ExtraTime * 1000));
	}

	// Check for window activity
	PreviousWindowActive = WindowActive;
	WindowActive = irrDevice->isWindowActive();

	// Check for window focus/blur events
	if(PreviousWindowActive != WindowActive) {
		Input.ResetInputState();

		if(!WindowActive) {
			MouseWasLocked = Input.GetMouseLocked();
			Input.SetMouseLocked(false);
		}
		else {
			Input.SetMouseLocked(MouseWasLocked);
		}
	}

	// Update fader
	Fader.Update(LastFrameTime * TimeScale);
	Graphics.BeginFrame();

	// Update the current state
	switch(ManagerState) {
		case STATE_INIT:
			ResetGraphics();
			Input.ResetInputState();
			if(!State->Init()) {
				Done = true;
				return;
			}

			Fader.Start(FADE_SPEED);
			ResetTimer();
			ManagerState = STATE_UPDATE;
		break;
		case STATE_UPDATE:
			TimeStepAccumulator += LastFrameTime * TimeScale;
			while(TimeStepAccumulator >= TimeStep) {
				State->Update(TimeStep);
				TimeStepAccumulator -= TimeStep;
			}
			
			State->UpdateRender(TimeStepAccumulator * TimeScale);
		break;
		case STATE_CLOSE:
			if(Fader.IsDoneFading()) {
				State->Close();
				State = NewState;
				ManagerState = STATE_INIT;
			}
		break;
	}

	Audio.Update();
	State->Draw();
	Graphics.EndFrame();
}

// Shuts down the system
void _Game::Close() {
	
	// Close the state
	State->Close();

	// Shut down the system
	Campaign.Close();
	Fader.Close();
	Physics.Close();
	ObjectManager.Close();
	Scripting.Close();
	Interface.Close();
	Audio.Close();
	Graphics.Close();
	Config.Close();
	Save.Close();
	Log.Close();
}

// Resets the game timer
void _Game::ResetTimer() {
	
	irrTimer->setTime(0);
	TimeStamp = irrTimer->getTime();
}

// Resets the graphics for a state
void _Game::ResetGraphics() {
	Graphics.SetClearColor(video::SColor(0, 0, 0, 0));
	Graphics.SetDrawScene(true);
	Interface.Clear();
}

// Initialize the audio system and load basic buffers
void _Game::EnableAudio() {
	Audio.Init(true);
	Audio.LoadBuffer("confirm.ogg");
	Audio.LoadBuffer("orb.ogg");
	Audio.LoadBuffer("player.ogg");
	Interface.LoadSounds();
}

// Disable audio system
void _Game::DisableAudio() {
	Audio.Close();
	Interface.UnloadSounds();
}
