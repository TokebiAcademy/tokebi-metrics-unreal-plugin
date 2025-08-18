# Changelog

All notable changes to the Tokebi Analytics Unreal Engine Plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2025-08-18

### Added
- Initial release of Tokebi Analytics Provider for Unreal Engine
- Native integration with Unreal Engine's Analytics Provider system
- Automatic environment switching (Development vs Production)
- Support for all standard analytics events:
  - Session management (start/end)
  - Custom event tracking
  - Item purchases
  - Currency transactions
  - Progress tracking
  - Error reporting
- Blueprint and C++ support
- Automatic event batching and HTTP transmission
- Persistent player ID generation and storage
- Cross-platform compatibility (Windows, Mac, Linux, Console, Mobile)
- Configurable API endpoint and settings
- Comprehensive logging and error handling
- Thread-safe event queuing
- Automatic retry mechanism for failed requests

### Technical Features
- Implements `IAnalyticsProvider` interface
- Uses Unreal's HTTP module for network requests
- JSON serialization for event payloads
- Timer-based event flushing (30-second intervals)
- Critical section protection for thread safety
- Persistent storage for player identification
- Environment detection based on build configuration

### Documentation
- Complete installation and configuration guide
- Blueprint and C++ usage examples
- Troubleshooting documentation
- API reference documentation
- Example project setup instructions

### Configuration Options
- `TokebiApiKey` - Authentication key (required)
- `TokebiGameId` - Game identifier (required)
- `TokebiEndpoint` - API endpoint URL (optional)
- `TokebiEnvironment` - Environment tag (optional)

### Supported Unreal Engine Versions
- Unreal Engine 5.0+
- Unreal Engine 4.27+ (compatibility mode)

[Unreleased]: https://github.com/TokebiAcademy/tokebi-analytics-unreal-plugin/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/TokebiAcademy/tokebi-analytics-unreal-plugin/releases/tag/v1.0.0
