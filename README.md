# Tokebi Analytics Plugin for Unreal Engine

Track player behavior and game events with Tokebi's analytics platform directly from your Unreal Engine project using custom Blueprint nodes and C++ functions.

## Prerequisites

Before using this plugin, you need:

1. **Tokebi Account**: Sign up at [tokebimetrics.com](https://tokebimetrics.com)
2. **API Key**: Get your API key from the Tokebi dashboard after creating an account
3. **Unreal Engine**: Version 5.0 or later
4. **C++ Project**: Plugin requires C++ support (has `Source` folder)

## Features

- 🎯 **Custom Blueprint Nodes** - Native Tokebi analytics nodes in Blueprint editor
- 🔄 **Automatic Game Registration** - Seamless setup with Tokebi platform
- 📊 **Blueprint & C++ Support** - Works with Visual Scripting and code
- 🚀 **Smart Batching System** - Configurable event queuing with auto-flush timers
- 💾 **Persistent Player IDs** - Consistent player tracking across sessions
- 🌐 **Cross-Platform** - Works on all Unreal Engine supported platforms
- 🔧 **Manual Flush Control** - Force immediate event sending when needed
- 📱 **Offline Support** - Events saved locally and sent when connection resumes
- ⚡ **World Context Handling** - Works in Editor, PIE, and Shipping builds

## Installation

### 1. Download and Extract Plugin

Place the plugin files in your project's `Plugins` directory with this exact structure:

```
YourProject/
├── Plugins/
│   └── TokebiAnalytics/
│       ├── TokebiAnalytics.uplugin
│       └── Source/
│           └── TokebiAnalytics/
│               ├── TokebiAnalytics.Build.cs
│               ├── TokebiAnalytics.cpp
│               ├── TokebiAnalyticsFunctions.h
│               ├── TokebiAnalyticsFunctions.cpp
│               ├── TokebiAnalyticsSettings.h
│               └── TokebiAnalyticsSettings.cpp
```

**All files go directly in `Source/TokebiAnalytics/` - NO Public/Private subfolders**

### 2. Enable the Plugin

1. Open your project in Unreal Engine
2. Go to **Edit → Plugins**
3. Search for "Tokebi Analytics"
4. Check the **Enabled** checkbox
5. Restart the editor when prompted

### 3. Configure Your Project

After enabling the plugin and restarting:

1. **Get your credentials from Tokebi dashboard:**
   - Sign up at [tokebimetrics.com](https://tokebimetrics.com) if you haven't already
   - Copy your API Key from the dashboard

2. **Configure in Unreal:**
   - Go to **Edit → Project Settings**
   - Navigate to **Plugins → Tokebi Analytics**
   - Enter your settings:
     - **API Key**: Your Tokebi API key from the dashboard
     - **Game ID**: Your project's Game ID (or leave empty for auto-generation)
     - **API Endpoint**: `https://tokebi-api.vercel.app` (default)
     - **Environment**: `development` or `production`

### 4. Installation Best Practices

- **Clean Build:** Delete `Binaries` and `Intermediate` folders after adding the plugin
- **Regenerate Project Files:** Right-click your `.uproject` → "Generate Visual Studio project files"
- **Delay Strategy:** Wait 1-2 seconds after `BeginPlay` before calling analytics functions to ensure proper initialization

## Configuration Options

Configure these settings in **Edit → Project Settings → Plugins → Tokebi Analytics**:

| Setting | Required | Description | Default |
|---------|----------|-------------|---------|
| `API Key` | ✅ | Your Tokebi API key from dashboard | - |
| `Game ID` | ❌ | Unique identifier for your game (auto-generated if empty) | - |
| `API Endpoint` | ❌ | Tokebi API endpoint URL | `https://tokebi-api.vercel.app` |
| `Environment` | ❌ | Environment tag (development/production) | `development` |
| `Flush Interval` | ❌ | Auto-flush interval (seconds) | `30.0` |
| `Max Batch Size` | ❌ | Max events per batch | `50` |
| `Offline Retry Delay` | ❌ | Retry delay when offline (seconds) | `60.0` |

**Note:** The plugin will **refuse to send events** if API Key is not configured.

## Usage

### Blueprint Usage

The plugin provides custom Blueprint nodes in the **Tokebi Analytics** category:

**Basic Event Flow:**
```
Event BeginPlay → Delay (1.0) → Tokebi Start Session
     ↓
Event Tick → [Game Logic] → Tokebi Track → [Event Name] → [Event Data]
     ↓
Event EndPlay → Tokebi End Session
```

**Available Blueprint Nodes:**

#### **Session Management**
- **Tokebi Start Session** - Initialize analytics session when game starts
- **Tokebi End Session** - Clean up session when game ends (auto-flushes events)

#### **Event Tracking**  
- **Tokebi Track** - Track custom events with data map
  - **Event Name** (String): Name of the event (e.g., "button_clicked")
  - **Event Data** (String Map): Key-value pairs of event data
- **Tokebi Track Level Start** - Track when player starts a level
  - **Level Name** (String): Name/ID of the level
- **Tokebi Track Level Complete** - Track level completion with metrics
  - **Level Name** (String): Name/ID of the level
  - **Completion Time** (Float): Time taken to complete in seconds
  - **Score** (Integer): Player's score for the level
- **Tokebi Track Purchase** - Track in-game purchases
  - **Item ID** (String): Unique identifier for purchased item
  - **Currency** (String): Type of currency used
  - **Cost** (Integer): Amount spent

#### **System Functions**
- **Tokebi Flush Events** - Force immediate sending of queued events
- **Tokebi Register Game** - Manually register game (usually automatic)

**Example Blueprint Setup:**
```
BeginPlay → Delay (1 second) → Tokebi Start Session

[Player Clicks Button] → Tokebi Track
    ├─ Event Name: "button_clicked"
    └─ Event Data: {"button_name": "play", "screen": "main_menu"}

[Level Starts] → Tokebi Track Level Start
    └─ Level Name: "forest_level_1"

[Level Ends] → Tokebi Track Level Complete  
    ├─ Level Name: "forest_level_1"
    ├─ Completion Time: 67.5
    └─ Score: 1500

EndPlay → Tokebi End Session
```

### C++ Usage

```cpp
#include "TokebiAnalyticsFunctions.h"

// Session Management
void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Delay analytics initialization (recommended)
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this]()
    {
        UTokebiAnalyticsFunctions::TokebiStartSession();
    }, 1.0f, false);
}

void AYourGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // End session and flush all events
    UTokebiAnalyticsFunctions::TokebiEndSession();
    Super::EndPlay(EndPlayReason);
}

// Custom Event Tracking
void TrackButtonClick(const FString& ButtonName, const FString& Screen)
{
    TMap<FString, FString> EventData;
    EventData.Add(TEXT("button_name"), ButtonName);
    EventData.Add(TEXT("screen"), Screen);
    EventData.Add(TEXT("timestamp"), FString::FromInt(FDateTime::UtcNow().ToUnixTimestamp()));
    
    UTokebiAnalyticsFunctions::TokebiTrack(TEXT("button_clicked"), EventData);
}

// Level Events
void OnLevelStart(const FString& LevelName)
{
    UTokebiAnalyticsFunctions::TokebiTrackLevelStart(LevelName);
}

void OnLevelComplete(const FString& LevelName, float CompletionTime, int32 Score)
{
    UTokebiAnalyticsFunctions::TokebiTrackLevelComplete(LevelName, CompletionTime, Score);
}

// Purchase Tracking
void OnItemPurchased(const FString& ItemId, const FString& Currency, int32 Cost)
{
    UTokebiAnalyticsFunctions::TokebiTrackPurchase(ItemId, Currency, Cost);
}

// Manual Controls
void ForceFlushEvents()
{
    // Send all queued events immediately
    UTokebiAnalyticsFunctions::TokebiFlushEvents();
}

void EnsureGameRegistered()
{
    // Manually trigger game registration (usually automatic)
    UTokebiAnalyticsFunctions::TokebiRegisterGame();
}
```

### Function Reference

#### **UTokebiAnalyticsFunctions::TokebiStartSession()**
- **Purpose**: Initialize analytics session and start tracking
- **When to call**: At game start, after 1-2 second delay
- **Effect**: Generates session ID, registers game if needed, starts auto-flush timer

#### **UTokebiAnalyticsFunctions::TokebiEndSession()**  
- **Purpose**: Clean up session and ensure all events are sent
- **When to call**: At game end or level exit
- **Effect**: Sends session_end event, flushes all queued events, clears session

#### **UTokebiAnalyticsFunctions::TokebiTrack(EventName, EventData)**
- **Purpose**: Track custom events with arbitrary data
- **Parameters**:
  - `EventName`: String name for the event type
  - `EventData`: Map of string key-value pairs
- **Example**: Track UI interactions, game state changes, custom metrics

#### **UTokebiAnalyticsFunctions::TokebiTrackLevelStart(LevelName)**
- **Purpose**: Track when player begins a level
- **Parameters**: `LevelName` - identifier for the level
- **Auto-includes**: Timestamp, session data

#### **UTokebiAnalyticsFunctions::TokebiTrackLevelComplete(LevelName, CompletionTime, Score)**
- **Purpose**: Track successful level completion with performance metrics
- **Parameters**:
  - `LevelName`: Level identifier  
  - `CompletionTime`: Seconds taken to complete
  - `Score`: Player's score/points earned
- **Use for**: Level difficulty analysis, player progression tracking

#### **UTokebiAnalyticsFunctions::TokebiTrackPurchase(ItemId, Currency, Cost)**
- **Purpose**: Track in-game purchases and economy interactions
- **Parameters**:
  - `ItemId`: Unique identifier for purchased item
  - `Currency`: Type of currency (coins, gems, USD, etc.)
  - `Cost`: Amount spent
- **Use for**: Economy balancing, monetization analysis

#### **UTokebiAnalyticsFunctions::TokebiFlushEvents()**
- **Purpose**: Force immediate sending of all queued events
- **When to call**: Before critical game states, level transitions, app backgrounding
- **Effect**: Bypasses auto-flush timer, ensures events reach server immediately

#### **UTokebiAnalyticsFunctions::TokebiRegisterGame()**
- **Purpose**: Manually trigger game registration with Tokebi platform  
- **When to call**: Usually automatic, call manually if registration fails
- **Effect**: Sends game info to Tokebi, required before events can be tracked

## Event Batching & Flushing

### Automatic Batching
- Events are queued locally and sent in batches
- **Default flush interval:** Every 30 seconds (configurable)
- **Configurable batch size:** Up to 50 events per batch (configurable)
- **Smart timing:** Immediate flush on session end, errors, and critical events

### Manual Flushing
When you need immediate event delivery:

**Blueprint:** Use "Tokebi Flush Events" node
**C++:** Call `UTokebiAnalyticsFunctions::TokebiFlushEvents()`

### World Context Handling
The plugin intelligently finds valid world contexts for timers:
1. **Play World** (PIE mode) - first priority
2. **Editor World** - fallback for editor usage  
3. **Any Available World** - last resort
4. **Fallback Mode** - graceful degradation if no world available

### Game Registration
- **Automatic registration** with Tokebi platform on first use
- **Game ID generation** if not configured
- **Retry logic** for failed registrations
- **Offline queue** for events when registration is pending

## Common Events to Track

### Game Flow Events
- `game_started` - Player starts the game
- `level_start` - Player enters a level  
- `level_complete` - Player completes a level
- `game_over` - Player dies/fails

### Engagement Events
- `button_clicked` - UI interaction
- `tutorial_step` - Tutorial progress
- `achievement_unlocked` - Player achievements

### Economy Events
- `item_purchase` - In-game purchases
- `currency_earned` - Player earns currency
- `currency_spent` - Player spends currency

### Performance Events
- `loading_time` - Track loading performance
- `fps_drop` - Performance issues
- `crash` - Game crashes

## Player ID Management

The plugin automatically:
- Generates unique player IDs on first run
- Saves player IDs to `ProjectSaved/Analytics/TokebiPlayerID.txt`
- Reuses the same ID across game sessions
- Creates new IDs if the file is deleted
- Handles ID persistence across different world contexts

## Data Format

Events sent to Tokebi follow this format:

```json
{
  "eventType": "level_complete",
  "payload": {
    "level": "level_1",
    "score": 1500,
    "timestamp": 1642123456
  },
  "gameId": "your_game_id",
  "playerId": "player_1642123400_7834",
  "platform": "unreal",
  "environment": "development"
}
```

### Game Registration
First-time setup automatically registers your game:

```json
POST /api/games
{
  "gameTitle": "Your Game Name",
  "platform": "unreal",
  "apiKey": "your_api_key"
}
```

## Troubleshooting

### Plugin Not Loading
- Check that TokebiAnalytics plugin is enabled in Edit → Plugins
- Verify all 7 source files are in correct locations (no Public/Private folders)
- Restart the editor after enabling
- Ensure project is C++ enabled (has Source folder)

### Events Not Sending
- Check your API key is correct in Project Settings → Plugins → Tokebi Analytics
- Verify internet connection
- Look for errors in Output Log (search "Tokebi")
- Ensure API Key is configured in project settings
- Try manual flush: `UTokebiAnalyticsFunctions::TokebiFlushEvents()`

### Timer/Auto-Flush Issues
- Check logs for "Using [play/editor/context] world for timer"
- Verify world context is available when initializing
- Manual flush always works regardless of timer status

### Build Errors
- Make sure HTTP, Json, and Engine modules are available
- Check that your project supports plugins
- Try regenerating project files
- Ensure all files are in Source/TokebiAnalytics/ (not in Public/Private subfolders)

### Debug Logging

Enable verbose logging in **Edit → Project Settings → Engine → General Settings → Log Categories**:

Add new category:
- **Category Name**: `LogTokebiAnalytics`
- **Default Verbosity**: `Verbose`

Or add to your project's `DefaultEngine.ini`:
```ini
[Core.Log]
LogTokebiAnalytics=Verbose
```

## API Reference

The plugin provides these Blueprint-callable functions:

- `TokebiStartSession()` - Begin analytics session
- `TokebiEndSession()` - End analytics session  
- `TokebiTrack(EventName, EventData)` - Track custom events
- `TokebiTrackLevelStart(LevelName)` - Track level start
- `TokebiTrackLevelComplete(LevelName, Time, Score)` - Track level completion
- `TokebiTrackPurchase(ItemId, Currency, Cost)` - Track purchases
- `TokebiFlushEvents()` - Send queued events immediately
- `TokebiRegisterGame()` - Manual game registration (usually automatic)

## Support

- **Documentation**: [https://tokebimetrics.com/documentation-guide](https://www.tokebimetrics.com/documentation-guide)
- **Tokebi Dashboard**: Create account and manage your games at [tokebimetrics.com](https://tokebimetrics.com)

**🤝 Contributing**

1. Fork this repository
2. Create a feature branch: `git checkout -b my-feature`
3. Commit changes: `git commit -am 'Add feature'`
4. Push to branch: `git push origin my-feature`
5. Submit a Pull Request

**📄 License**

This project is licensed under the MIT License - see the LICENSE file for details.
