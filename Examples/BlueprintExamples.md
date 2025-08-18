# Blueprint Examples - Tokebi Analytics

This guide shows how to use Tokebi Analytics in Blueprint visual scripting.

## Essential Analytics Nodes

All these nodes are found in the **Analytics** category in Blueprint:

- **Start Session** - Begin analytics tracking
- **End Session** - Stop analytics tracking  
- **Record Event** - Track custom events
- **Record Item Purchase** - Track in-game purchases
- **Record Progress** - Track player progression
- **Record Error** - Track errors and issues

## Example 1: Basic Session Management

### Level Blueprint Setup

```
[Event BeginPlay] 
       ↓
[Start Session]
├ Attributes (Array):
│  ├ [0] Name: "level_name", Value: [Get Current Level Name]
│  ├ [1] Name: "game_version", Value: "1.0.0"
│  └ [2] Name: "platform", Value: [Get Platform Name]
└ [Print String]: "Analytics session started"

[Event EndPlay]
       ↓
[End Session]
       ↓
[Print String]: "Analytics session ended"
```

## Example 2: Player Action Tracking

### Player Blueprint - Jump Tracking

```
[InputAction Jump]
       ↓
[Record Event]
├ Event Name: "player_jump"
├ Attributes (Array):
│  ├ [0] Name: "location_x", Value: [Get Actor Location X (String)]
│  ├ [1] Name: "location_y", Value: [Get Actor Location Y (String)]
│  └ [2] Name: "jump_count", Value: [Jump Counter (String)]
└ [Print String]: "Jump tracked"
```

### Player Blueprint - Level Complete

```
[Custom Event: OnLevelComplete]
       ↓
[Record Progress]
├ Progress Type: "level_complete"
├ Progress Name: [Format String: "level_{0}" with Level Number]
├ Attributes (Array):
│  ├ [0] Name: "completion_time", Value: [Completion Time (String)]
│  ├ [1] Name: "score", Value: [Final Score (String)]
│  ├ [2] Name: "deaths", Value: [Death Count (String)]
│  └ [3] Name: "collectibles", Value: [Items Collected (String)]
└ [Print String]: "Level completion tracked"
```

## Example 3: UI Interaction Tracking

### Main Menu Widget Blueprint

```
[Button: Play Game → OnClicked]
       ↓
[Record Event]
├ Event Name: "button_clicked"
├ Attributes (Array):
│  ├ [0] Name: "button_name", Value: "play_game"
│  ├ [1] Name: "menu_name", Value: "main_menu"
│  └ [2] Name: "session_time", Value: [Get Game Time in Seconds (String)]
└ [Print String]: "Play button clicked"

[Button: Settings → OnClicked]  
       ↓
[Record Event]
├ Event Name: "menu_navigation"
├ Attributes (Array):
│  ├ [0] Name: "from_menu", Value: "main_menu"
│  ├ [1] Name: "to_menu", Value: "settings"
│  └ [2] Name: "navigation_time", Value: [Get Game Time in Seconds (String)]
└ [Print String]: "Settings navigation tracked"
```

## Example 4: Purchase Tracking

### Store Widget Blueprint

```
[Button: Buy Item → OnClicked]
       ↓
[Record Item Purchase]
├ Item Id: [Selected Item ID (String)]
├ Currency: "gold"
├ Per Item Cost: [Item Cost (Integer)]
├ Item Quantity: 1
└ [Print String]: "Purchase tracked"

[Custom Event: OnRealMoneyPurchase]
       ↓
[Record Currency Purchase]  
├ Game Currency Type: "gems"
├ Game Currency Amount: [Gems Purchased (Integer)]
├ Real Currency Type: "USD"
├ Real Money Cost: [Dollar Amount (Float)]
├ Payment Provider: "app_store"
└ [Print String]: "Real money purchase tracked"
```

## Example 5: Error and Performance Tracking

### Game Instance Blueprint

```
[Custom Event: OnGameError]
       ↓
[Record Error]
├ Error: [Error Message (String)]
├ Attributes (Array):
│  ├ [0] Name: "error_context", Value: [Error Context (String)]
│  ├ [1] Name: "level_name", Value: [Current Level (String)]
│  └ [2] Name: "player_state", Value: [Player State Description (String)]
└ [Print String]: "Error tracked"

[Event Tick] (with custom timer logic)
       ↓
[Branch: Should Track FPS?] → True
       ↓
[Record Event]
├ Event Name: "performance_sample"
├ Attributes (Array):
│  ├ [0] Name: "fps", Value: [Get FPS (String)]
│  ├ [1] Name: "memory_usage", Value: [Get Memory Usage (String)]
│  └ [2] Name: "level_name", Value: [Current Level (String)]
└ [Print String]: "Performance tracked"
```

## Example 6: Game-Specific Events

### Combat System Tracking

```
[Custom Event: OnEnemyKilled]
       ↓
[Record Event]
├ Event Name: "enemy_killed"
├ Attributes (Array):
│  ├ [0] Name: "enemy_type", Value: [Enemy Type (String)]
│  ├ [1] Name: "weapon_used", Value: [Weapon Name (String)]
│  ├ [2] Name: "damage_dealt", Value: [Total Damage (String)]
│  ├ [3] Name: "kill_time", Value: [Combat Duration (String)]
│  └ [4] Name: "player_level", Value: [Player Level (String)]
└ [Print String]: "Enemy kill tracked"
```

### Economy Tracking

```
[Custom Event: OnCurrencyEarned]
       ↓
[Record Event]
├ Event Name: "currency_earned"
├ Attributes (Array):
│  ├ [0] Name: "currency_type", Value: [Currency Type (String)]
│  ├ [1] Name: "amount", Value: [Amount Earned (String)]
│  ├ [2] Name: "source", Value: [Earning Source (String)] // "quest", "kill", "purchase"
│  └ [3] Name: "total_balance", Value: [New Balance (String)]
└ [Print String]: "Currency earning tracked"
```

## Blueprint Best Practices

### 1. Create Custom Events for Reusability

Instead of repeating analytics logic, create custom events:

```
[Custom Event: Track Button Click]
├ Input: Button Name (String)
├ Input: Menu Name (String)
       ↓
[Record Event]
├ Event Name: "button_clicked"
├ Attributes: Use input parameters
└ Output: Success (Boolean)
```

### 2. Use Blueprint Function Library

Create a Blueprint Function Library for common analytics functions:

```
Function: Track Level Event
├ Inputs: Event Type (String), Level Number (Integer), Additional Data (Map)
├ Logic: Format event name, convert data to attributes
└ Output: Call Record Event with formatted data
```

### 3. Handle Data Conversion

Always convert to strings for attributes:

```
[Integer] → [Convert to String] → [Attribute Value]
[Float] → [Convert to String] → [Attribute Value]  
[Vector] → [Break Vector] → [Convert each to String] → [Multiple Attributes]
```

### 4. Add Safety Checks

```
[Any Analytics Call]
       ↓
[Branch: Is Analytics Available?] → True → [Proceed with tracking]
                                  → False → [Print Warning: Analytics not available]
```

### 5. Group Related Events

Use consistent prefixes for related events:
- `ui_button_clicked`
- `ui_menu_opened`
- `ui_dialog_closed`
- `gameplay_level_start`
- `gameplay_level_complete`
- `gameplay_player_death`

## Common Blueprint Patterns

### Funnel Tracking Pattern

```
[Level Start] → Track: "level_funnel" with step: "start"
[Checkpoint] → Track: "level_funnel" with step: "checkpoint_1"
[Boss Fight] → Track: "level_funnel" with step: "boss_encounter"
[Level End] → Track: "level_funnel" with step: "complete"
```

### A/B Testing Pattern

```
[Game Start] → [Random Bool] → True: Set "variant_a"
                            → False: Set "variant_b"
                            
[Any Event] → Include "test_variant" attribute with stored variant
```

### Session Quality Pattern

```
[Session Start] → Record start time
[Session End] → Calculate session duration
              → Track "session_quality" with duration, events_sent, errors_encountered
```

## Debugging Blueprint Analytics

### Enable Debug Prints

Add Print String nodes after each analytics call to verify execution:

```
[Record Event] → [Print String]: "Tracked: {EventName}"
[Start Session] → [Print String]: "Session started with {AttributeCount} attributes"
```

### Check Output Log

Look for these messages in the Output Log window:
- `LogTokebiProvider: Recording event: your_event_name`
- `LogTokebiProvider: Queued event: your_event_name`
- `LogTokebiProvider: Sending X queued events to Tokebi`

### Test with Fake Data

Use placeholder values during development:
```
Event Name: "test_event"
Attributes: 
├ "test_key": "test_value"
└ "timestamp": [Current Time String]
```

This helps verify the analytics pipeline without affecting real metrics.
