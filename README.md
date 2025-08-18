# Tokebi Analytics Plugin for Unreal Engine

Track player behavior and game events with Tokebi's analytics platform directly from your Unreal Engine project.

## Features

- 🎯 **Native Unreal Integration** - Uses Unreal's built-in Analytics Provider system
- 🔄 **Automatic Environment Switching** - Development vs Production builds
- 📊 **Blueprint & C++ Support** - Works with Visual Scripting and code
- 🚀 **Automatic Batching** - Efficient event queuing and sending
- 💾 **Persistent Player IDs** - Consistent player tracking across sessions
- 🌐 **Cross-Platform** - Works on all Unreal Engine supported platforms

## Installation

### 1. Download the Plugin

Place the plugin files in your project's `Plugins` directory:

```
YourProject/
├── Plugins/
│   └── TokebiAnalytics/
│       ├── TokebiAnalytics.uplugin
│       └── Source/
│           └── TokebiAnalytics/
│               ├── TokebiAnalytics.Build.cs
│               ├── Public/
│               │   ├── TokebiAnalyticsModule.h
│               │   └── TokebiAnalyticsProvider.h
│               └── Private/
│                   ├── TokebiAnalyticsModule.cpp
│                   └── TokebiAnalyticsProvider.cpp
```

### 2. Enable the Plugin

1. Open your project in Unreal Engine
2. Go to **Edit → Plugins**
3. Search for "Tokebi Analytics"
4. Check the **Enabled** checkbox
5. Restart the editor when prompted

### 3. Configure Your Project

Add configuration to your `Config/DefaultEngine.ini` file:

```ini
[Analytics]
ProviderModuleName=Tokebi
TokebiApiKey=your_production_api_key_here
TokebiGameId=your_game_id_here

[AnalyticsDevelopment]
ProviderModuleName=Tokebi
TokebiApiKey=your_development_api_key_here
TokebiGameId=your_game_id_here
```

## Configuration Options

| Setting | Required | Description | Default |
|---------|----------|-------------|---------|
| `TokebiApiKey` | ✅ | Your Tokebi API key | - |
| `TokebiGameId` | ✅ | Unique identifier for your game | - |
| `TokebiEndpoint` | ❌ | Tokebi API endpoint URL | `https://tokebi-api.vercel.app` |
| `TokebiEnvironment` | ❌ | Environment tag | `production` |

## Environment Switching

The plugin automatically switches between environments based on your build configuration:

- **Development/Debug builds** → Uses `[AnalyticsDevelopment]` settings
- **Shipping/Release builds** → Uses `[Analytics]` settings

This means you can use different API keys for testing vs production without any code changes!

## Usage

### Blueprint Usage

Use the standard Analytics Blueprint nodes:

1. **Start Session** - Call on game start
2. **Record Event** - Track custom events
3. **Record Item Purchase** - Track in-game purchases
4. **Record Progress** - Track level completion, achievements
5. **Record Error** - Track errors and crashes
6. **End Session** - Call on game end

### C++ Usage

```cpp
#include "Analytics.h"

// Start session
TArray<FAnalyticsEventAttribute> SessionAttribs;
SessionAttribs.Add(FAnalyticsEventAttribute(TEXT("version"), TEXT("1.0")));
FAnalytics::Get().GetDefaultProvider()->StartSession(SessionAttribs);

// Track custom event
TArray<FAnalyticsEventAttribute> Attributes;
Attributes.Add(FAnalyticsEventAttribute(TEXT("level"), TEXT("level_1")));
Attributes.Add(FAnalyticsEventAttribute(TEXT("score"), TEXT("1500")));
FAnalytics::Get().GetDefaultProvider()->RecordEvent(TEXT("level_complete"), Attributes);

// Track purchase
FAnalytics::Get().GetDefaultProvider()->RecordItemPurchase(
    TEXT("sword_legendary"), // Item ID
    TEXT("gold"),           // Currency
    100,                    // Cost per item
    1                       // Quantity
);

// End session
FAnalytics::Get().GetDefaultProvider()->EndSession();
```

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

## Event Batching

Events are automatically:
- Queued locally for efficient sending
- Sent in batches every 30 seconds
- Flushed immediately on session end
- Retried on network failures

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

## Troubleshooting

### Plugin Not Loading
- Check that Analytics plugin is enabled in Project Settings
- Verify TokebiAnalytics plugin is enabled
- Restart the editor after enabling

### Events Not Sending
- Check your API key is correct
- Verify internet connection
- Look for errors in Output Log (search "Tokebi")
- Ensure `TokebiGameId` is configured

### Build Errors
- Make sure HTTP, Json, and JsonUtilities modules are available
- Check that your project supports plugins
- Try regenerating project files

### Debug Logging

Enable verbose logging in `DefaultEngine.ini`:

```ini
[Core.Log]
LogTokebiProvider=Verbose
LogTokebiAnalytics=Verbose
```

## API Reference

The plugin implements Unreal's standard `IAnalyticsProvider` interface:

- `StartSession(Attributes)` - Begin analytics session
- `EndSession()` - End analytics session  
- `SetUserID(UserID)` - Set custom user identifier
- `RecordEvent(EventName, Attributes)` - Track custom events
- `RecordItemPurchase(...)` - Track in-game purchases
- `RecordCurrencyPurchase(...)` - Track real money transactions
- `RecordProgress(...)` - Track player progression
- `RecordError(...)` - Track errors and exceptions
- `FlushEvents()` - Send queued events immediately

## Support

- **Documentation**: [https://tokebimetrics.com/docs](https://www.tokebimetrics.com/documentation-guide)

## License

This plugin is provided under the MIT License. See LICENSE file for details.
