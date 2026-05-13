#pragma once
#include "ControlMediator.h"
#include "MediatorConfig.h"
#include <Arduino.h>
#include <time.h>   // getLocalTime() — ESP32 Arduino framework

/**
 * TimerSource — two kinds of automatic actuator activation:
 *
 *   Periodic  : fire every N milliseconds (existing).
 *   Daily     : fire once per day at a specific HH:MM (new).
 *               Requires NTP sync; tick() silently skips if no time available.
 *               Uses getLocalTime() with a 0 ms timeout (non-blocking).
 *               Re-fires the next day automatically.
 *
 * Call timerSource.tick(mediator) from loop().
 */

// ── Periodic entries ──────────────────────────────────────────────────────────
struct TimerEntry {
    uint8_t  actuatorId;
    uint32_t periodMs;
    uint32_t durationMs;
    uint32_t lastFireMs;
    bool     enabled;
};

// ── Daily entries ─────────────────────────────────────────────────────────────
#define DAILY_TIMER_MAX 8

struct DailyTimerEntry {
    uint8_t  actuatorId;
    uint8_t  hour;         ///< 0–23
    uint8_t  minute;       ///< 0–59
    uint32_t durationMs;   ///< 0 = hold until next rule/manual command
    bool     firedThisMinute;  ///< prevents double-firing within same minute
    bool     enabled;
};

class TimerSource {
public:
    TimerSource() : _count(0), _dailyCount(0) {}

    // ── Periodic ──────────────────────────────────────────────────────────

    /**
     * Fire actuatorId ON every periodMs milliseconds for durationMs.
     * durationMs = 0 → keep ON until next command.
     */
    void addTimer(uint8_t actuatorId, uint32_t periodMs,
                  uint32_t durationMs = 0) {
        if (_count >= ACTUATOR_MAX) return;
        _entries[_count++] = {actuatorId, periodMs, durationMs, millis(), true};
    }

    void disable(uint8_t actuatorId) { _setEnabled(actuatorId, false); }
    void enable (uint8_t actuatorId) { _setEnabled(actuatorId, true);  }

    // ── Daily ─────────────────────────────────────────────────────────────

    /**
     * Fire actuatorId ON every day at hour:minute (local time).
     * Requires ESP32 NTP sync (configTime called before setup completes).
     * If no time is available, tick() skips daily checks silently.
     */
    void addDailyTimer(uint8_t actuatorId, uint8_t hour, uint8_t minute,
                       uint32_t durationMs = 0) {
        if (_dailyCount >= DAILY_TIMER_MAX) return;
        _daily[_dailyCount++] = {
            actuatorId, hour, minute, durationMs,
            false,  // firedThisMinute
            true
        };
    }

    void disableDaily(uint8_t actuatorId) { _setDailyEnabled(actuatorId, false); }
    void enableDaily (uint8_t actuatorId) { _setDailyEnabled(actuatorId, true);  }

    // ── Tick ─────────────────────────────────────────────────────────────
    void tick(ControlMediator& mediator) {
        uint32_t now = millis();

        // Periodic
        for (uint8_t i = 0; i < _count; i++) {
            TimerEntry& e = _entries[i];
            if (!e.enabled) continue;
            if (now - e.lastFireMs >= e.periodMs) {
                e.lastFireMs = now;
                mediator.onTimerTick(e.actuatorId, true, e.durationMs);
            }
        }

        // Daily (opportunistic — only when NTP time is available)
        struct tm t;
        bool hasTime = getLocalTime(&t, 0);  // 0 = non-blocking

        if (hasTime) {
            for (uint8_t i = 0; i < _dailyCount; i++) {
                DailyTimerEntry& e = _daily[i];
                if (!e.enabled) continue;

                bool inWindow = (t.tm_hour == e.hour && t.tm_min == e.minute);

                if (inWindow && !e.firedThisMinute) {
                    e.firedThisMinute = true;
                    mediator.onTimerTick(e.actuatorId, true, e.durationMs);
                } else if (!inWindow && e.firedThisMinute) {
                    e.firedThisMinute = false;  // reset for next occurrence
                }
            }
        }
    }

    uint8_t getCount()      const { return _count; }
    uint8_t getDailyCount() const { return _dailyCount; }

private:
    TimerEntry      _entries[ACTUATOR_MAX];
    uint8_t         _count;

    DailyTimerEntry _daily[DAILY_TIMER_MAX];
    uint8_t         _dailyCount;

    void _setEnabled(uint8_t id, bool v) {
        for (uint8_t i = 0; i < _count; i++)
            if (_entries[i].actuatorId == id) _entries[i].enabled = v;
    }
    void _setDailyEnabled(uint8_t id, bool v) {
        for (uint8_t i = 0; i < _dailyCount; i++)
            if (_daily[i].actuatorId == id) _daily[i].enabled = v;
    }
};
