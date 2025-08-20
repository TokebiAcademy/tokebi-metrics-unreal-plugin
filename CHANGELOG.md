# Changelog
All notable changes to the Tokebi Analytics Unreal Engine Plugin will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2025-08-20

### Added
- Initial release of Tokebi Analytics Plugin for Unreal Engine
- **Custom Blueprint Function Library** with dedicated Tokebi Analytics nodes
- **Project Settings UI integration** for easy configuration (Edit → Project Settings → Plugins → Tokebi Analytics)
- **Automatic game registration** with Tokebi platform on first use
- **Smart batching system** with configurable intervals and batch sizes
- **Manual flush control** for immediate event sending

#### Blueprint Nodes
- `Tokebi Start Session` - Initialize analytics session
- `Tokebi End Session` - Clean up session and flush events
- `Tokebi Track` - Track custom events with data map
- `Tokebi Track Level Start` - Track level start events
- `Tokebi Track Level Complete` - Track level completion with metrics
- `Tokebi Track Purchase` - Track in-game purchases
- `Tokebi Flush Events` - Force immediate event sending
- `Tokebi Register Game` - Manual game registration

#### C++ Functions
- `UTokebiAnalyticsFunctions::TokebiStartSession()`
- `UTokebiAnalyticsFunctions::TokebiEndSession()`
- `UTokebiAnalyticsFunctions::TokebiTrack(EventName, EventData)`
- `UTokebiAnalyticsFunctions::TokebiTrackLevelStart(LevelName)`
- `UTokebiAnalyticsFunctions::TokebiTrackLevelComplete(LevelName, Time, Score)`
- `UTokebiAnalyticsFunctions::TokebiTrackPurchase(ItemId, Currency, Cost)`
- `UTokebiAnalyticsFunctions::TokebiFlushEvents()`
- `UTokebiAnalyticsFunctions::TokebiRegisterGame()`

#### Technical Features
- **World Context Intelligence** - Automatically finds valid world contexts for timers (Play World → Editor World → Any Available World)
- **Offline Support** - Events queued locally and sent when connection resumes
- **Thread-safe event queuing** with critical section protection
- **Persistent player ID management** stored in `ProjectSaved/Analytics/TokebiPlayerID.txt`
- **Automatic retry mechanism** for failed requests with configurable delays
- **HTTP-based communication** using Unreal's FHttpModule
- **JSON event serialization** with proper payload formatting
- **Platform detection** (automatically sends "unreal" as platform)

#### Configuration System
- **Project Settings UI** integration (no manual .ini file editing required)
- **UTokebiAnalyticsSettings** class for configuration management
- Configurable settings:
  - `API Key` - Tokebi authentication key (required)
  - `Game ID` - Unique game identifier (auto-generated if empty)
  - `API Endpoint` - Tokebi API URL (defaults to https://tokebi-api.vercel.app)
  - `Environment` - Development or production environment tag
  - `Flush Interval` - Auto-flush timer interval in seconds (default: 30)
  - `Max Batch Size` - Maximum events per batch (default: 50)
  - `Offline Retry Delay` - Retry delay when offline in seconds (default: 60)

#### Data & Event Management
- **Automatic batching** with configurable intervals and sizes
- **Session management** with unique session ID generation
- **Event queuing** for reliable delivery
- **Timestamp management** with Unix timestamp format
- **Game registration workflow** via `/api/games` endpoint
- **Event tracking** via `/api/track` endpoint

#### Cross-Platform Support
- Windows, Mac, Linux (all desktop platforms)
- Console platforms (PlayStation, Xbox, Nintendo Switch)
- Mobile platforms (iOS, Android)
- All Unreal Engine supported platforms

#### Developer Experience
- **No external dependencies** - self-contained plugin
- **Blueprint-first design** - works without C++ knowledge
- **Comprehensive logging** with LogTokebiAnalytics category
- **Error handling** with graceful degradation
- **Installation validation** - refuses to send events if not properly configured
- **Professional UI integration** matching Unreal Engine standards

### Documentation
- Complete installation guide with exact file structure
- Project Settings configuration walkthrough
- Blueprint node usage examples with visual flow diagrams
- C++ integration examples with practical code samples
- Function reference with detailed parameter explanations
- Troubleshooting guide for common issues
- Event tracking best practices and patterns
- Performance optimization guidelines

### Supported Unreal Engine Versions
- **Unreal Engine 5.0+** (primary target)
- **Unreal Engine 4.27+** (compatibility)

### Known Limitations
- Requires C++ project support (has `Source` folder)
- Requires internet connection for event transmission
- Events are batched by default (use manual flush for immediate sending)

[Unreleased]: https://github.com/TokebiAcademy/tokebi-analytics-unreal-plugin/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/TokebiAcademy/tokebi-analytics-unreal-plugin/releases/tag/v1.0.0
